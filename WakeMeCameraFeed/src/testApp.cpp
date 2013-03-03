#include "testApp.h"

void testApp::setup() {
	
	ofEnableAlphaBlending();
	ofSetVerticalSync(true);
	ofSetLogLevel(OF_LOG_VERBOSE);
	camera1.setup(0);
	front.setup(0,true);
	back.setContext(front.getContext());
	back.setup(1);
	ghostFrames = 0;
	
	showRGBD = false;
	
	
	timeBetweenTriggers = 6;
	timeOfNextTrigger = 0;
	
	requiredThresholdTime = 7;

	gui.setup("tests");
    gui.add(xshift.setup("xshift", ofxParameter<float>(), -.15, .15));
    gui.add(yshift.setup("yshift", ofxParameter<float>(), -.15, .15));
    gui.add(xsimplify.setup("xsimplify", ofxParameter<float>(), 1, 8));
    gui.add(ysimplify.setup("ysimplify", ofxParameter<float>(), 1, 8));
	gui.add(threshold.setup("threshold", ofxParameter<float>(), 15, 255));
	gui.add(minSize.setup("minSize", ofxParameter<float>(), 1, 640*480));
	gui.add(minTriggerDepth.setup("minTriggerDepth", ofxParameter<float>(), 50, 255));
	gui.add(maxTriggerDepth.setup("maxTriggerDepth", ofxParameter<float>(), 50, 255));
	

    gui.loadFromFile("defaultSettings.xml");
//	ofSetLogLevel(ofxOpenNIContext::LOG_NAME, OF_LOG_VERBOSE);
//  numDevices = openNIContext->getNumDevices();
	
	renderer1.setup("_calibration");
	renderer2.setDepthOnly();
	//renderer1.setup("_calibration");
	photoPreview.allocate(696, 464, OF_IMAGE_COLOR);
	renderer1.setRGBTexture(photoPreview);
	
	//renderer2.setDepthOnly();
	
	renderer1.setSimplification(ofVec2f(4,4));
	renderer2.setSimplification(ofVec2f(6,6));

	renderer1.farClip = 1500;
	renderer2.farClip = 1500;
	
	renderer1.cacheValidVertices = true;
	renderer2.cacheValidVertices = true;
	
	renderer1.setDepthImage(front.getRawDepth());
	renderer2.setDepthImage(back.getRawDepth());

	sender.setup("localhost", 12345);
	
	cam.setup();
	cam.autosavePosition = true;
	cam.loadCameraPosition();
	
	contourFinder.setMinAreaRadius(1);
//	contourFinder.setMaxAreaRadius(100);

	inThreshold = true;
	shouldFireCamera = false;
}

void testApp::update() {
	
//	for (int deviceID = 0; deviceID < numDevices; deviceID++){
//		if(openNIDevices[deviceID]->update()){
//			cout << "UPDATED " << deviceID << endl;
//		}
//	}
	
	front.update();
	if(front.isFrameNew()){
		contourFinder.setThreshold(threshold);
		contourFinder.setMinArea(minSize);
		
		compressor.convertTo8BitImage(front.getRawDepth(), depthImage1);
		
		contourFinder.findContours(depthImage1);
		if(contourFinder.size() > 0){
			int maxAreaIndex = 0;
			float maxArea = 0;

			for(int i = 0; i < contourFinder.size(); i++) {
				if(maxArea < contourFinder.getBoundingRect(i).area()){
					maxArea	 = contourFinder.getBoundingRect(i).area();
					maxAreaIndex = i;
				}
			}
			ofPoint center = ofxCv::toOf(contourFinder.getCenter(maxAreaIndex) );
			float areaDepth =  depthImage1.getColor( center.x, center.y ).getBrightness();
			//cout << "depth? " << areaDepth << endl;
			if(areaDepth > minTriggerDepth && areaDepth < maxTriggerDepth){
				ghostFrames = 0;
				if(!inThreshold){
					thresholdEnteredTime = ofGetElapsedTimef();
					inThreshold = true;
				}
				float timeInThreshold = ofGetElapsedTimef() - thresholdEnteredTime;
				cout << "time in threshold " << timeInThreshold << endl;
				if(timeInThreshold > requiredThresholdTime){
					shouldFireCamera = true;
					inThreshold = false;
					cout << "FIRING CAMERA!" << endl;
				}
			}
			else{
				if(ghostFrames++ > 5){
					inThreshold = false;
				}
			}
		}
		//cout << "found " << contourFinder.size() << endl;
		/*
		for(int y = 0; y < 640; y+=2){
			for(int x = 0; x < 480; x++){
				
			}
		}
		 */
	}
	
	back.update();
	if(back.isFrameNew()){
		compressor.convertTo8BitImage(back.getRawDepth(), depthImage2);
	}
	
	camera1.update();
	if(camera1.isFrameNew()) {
		// process the live view with camera.getLivePixels()
		pix.setFromPixels(camera1.getLivePixels());
	}
	
	if(renderer1.shift.x != xshift || renderer1.shift.y != yshift){
		renderer1.shift = ofVec2f(xshift,yshift);
		renderer1.update();
	}
	
	if(camera1.isPhotoNew()) {
		// process the photo with camera.getPhotoPixels()
		// or just save the photo to disk (jpg only):
		cout << "saved camera 1 photo" << endl;
		if(renderer1.hasTriangles && renderer2.hasTriangles){
			//camera1.savePhoto(ofToString(ofGetFrameNum()) + ".jpg");
			camera1.getPhotoPixels().resize(696, 464);
			//camera1.savePhoto(currentSaveDirectory + "/photo.jpg");
			ofSaveImage(camera1.getPhotoPixels(), currentSaveDirectory + "/photo.jpg");
			
			ofSleepMillis(500);
			
			ofxOscMessage m;
			m.setAddress("/mesh");
			m.addStringArg("file://"+currentSaveDirectory);
			sender.sendMessage(m);
		}
		if(showRGBD){
			photoPreview.setFromPixels(camera1.getPhotoPixels());
			renderer1.update();
		}
		inThreshold = false;
	}
	
	    
/*
	camera2.update();
	if(camera2.isFrameNew()) {
		// process the live view with camera.getLivePixels()
		pix.setFromPixels(camera2.getLivePixels());
	}
	if(camera2.isPhotoNew()) {
		// process the photo with camera.getPhotoPixels()
		// or just save the photo to disk (jpg only):
		cout << "saved camera 2 photo" << endl;
		camera2.savePhoto(ofToString(ofGetFrameNum()) + ".jpg");
	}
*/
	if(shouldFireCamera){
		fireCamera();
	}
}

void testApp::draw() {
	ofSetColor(255,255);
//	camera1.draw(0, 0);
	
	ofRectangle rgbdRect(0, 0, 1280, 720);

	cam.applyRotation = cam.applyTranslation = showRGBD && rgbdRect.inside(mouseX, mouseY);
	if(showRGBD){
		ofSetColor(0);
		ofRect(rgbdRect);
		ofSetColor(255);
		cam.begin(rgbdRect);
		renderer1.drawMesh();
		cam.end();
	}
	else{
//		front.getColorImage().draw(0,0);
		//back.getRawIRImage().draw(640, 480);
		if(depthImage1.isAllocated()) depthImage1.draw(0, 0);
		if(depthImage2.isAllocated()) depthImage2.draw(640, 0);
		if(inThreshold){
			ofSetColor(255,200,10);
		}
		else{
			ofSetColor(255);
		}
		contourFinder.draw();
		ofSetColor(255);
	}
		
	gui.draw();
	
//	camera2.draw(0, 0);

	// camera.drawPhoto(0, 0, 432, 288);
	/*
	if(camera.isLiveReady()) {
		stringstream status; 
			status << camera.getWidth() << "x" << camera.getHeight() << " @ " <<
			(int) ofGetFrameRate() << " app-fps " << " / " <<
			(int) camera.getFrameRate() << " cam-fps";
		ofDrawBitmapString(status.str(), 10, 20);
	}
	*/
	
	if(pix.isAllocated()){
//		pix.draw(0,0);
	}
}

void testApp::keyPressed(int key) {
	if(key == ' ') {
		
		fireCamera();
	}
	
	if(key == '1'){
		showRGBD = !showRGBD;
		if(showRGBD){
			photoPreview.setFromPixels(camera1.getPhotoPixels());
			photoPreview.resize(696, 464);
			renderer1.update();
		}
	}
	
//	if(key == '2'){
//		camera2.takePhoto();
//	}
}

void testApp::fireCamera(){
	cout << "Firing camrea now" << endl;
	shouldFireCamera = false;
	camera1.takePhoto();
	
	ofDirectory dir;
	char pathc[1024];
	sprintf(pathc, "%02d_%02d_%02d_%02d", ofGetDay(), ofGetHours(), ofGetMinutes(), ofGetSeconds());
	string path(pathc);
	dir.createDirectory(path);
	
	renderer1.update();
	renderer2.update();
	
	ofFile f(path);
	cout << " absolute path is " << f.getAbsolutePath() << endl;
	
	ofMesh mesh1 = renderer1.getReducedMesh(true, ofVec3f(1,1,1), false, true);
	ofMesh mesh2 = renderer2.getReducedMesh(true, ofVec3f(1,1,1), false, true);
	ofxObjLoader::save(f.getAbsolutePath() + "/mesh1.obj", mesh1);
	ofxObjLoader::save(f.getAbsolutePath() + "/mesh2.obj", mesh2);
	front.getColorImage().saveImage(f.getAbsolutePath() + "/backTexture.png");
	currentSaveDirectory = f.getAbsolutePath();
	
	
	
	//		ofxOscMessage m;
	//		m.setAddress("/mesh");
	//		m.addStringArg("file://"+f.getAbsolutePath());
	//		sender.sendMessage(m);
}

//--------------------------------------------------------------
void testApp::exit(){
    gui.saveToFile("defaultSettings.xml");
}
