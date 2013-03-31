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

	showPointcloud = false;
	showRGBD = false;
	
	cam1Calib.resize(4);
	cam2Calib.resize(4);

	timeBetweenTriggers = 6;
	timeOfNextTrigger = 0;
	
	requiredThresholdTime = 7;

	gui.setup("tests");
    gui.add(xshift.setup("xshift", ofParameter<float>(), -.15, .15));
    gui.add(yshift.setup("yshift", ofParameter<float>(), -.15, .15));
	gui.add(threshold.setup("threshold", ofParameter<float>(), 15, 255));
	gui.add(minSize.setup("minSize", ofParameter<float>(), 1, 640*480));
	gui.add(minTriggerDepth.setup("minTriggerDepth", ofParameter<float>(), 50, 255));
	gui.add(maxTriggerDepth.setup("maxTriggerDepth", ofParameter<float>(), 50, 255));
	gui.add(autoFire.setup("auto fire", ofParameter<bool>()));
	
	
	gui.add(xTranslate.setup("xTranslate", ofParameter<float>(), -100, 100));
	gui.add(yTranslate.setup("yTranslate", ofParameter<float>(), -100, 100));
	gui.add(zTranslate.setup("zTransalte", ofParameter<float>(), 1000, 3000));
	
	gui.add(xRotate.setup("xRotate", ofParameter<float>(), 0, 360));
	gui.add(yRotate.setup("yRotate", ofParameter<float>(), 0, 360));
	gui.add(zRotate.setup("zRotate", ofParameter<float>(), 0, 360));
	gui.add(alwaysUpdate.setup("auto update mesh", ofParameter<bool>()));
	
	gui.setPosition(ofPoint(640*2, 0));
	
	rotation = ofQuaternion(0.001134582, 0.9988741, 0.03121687, 0.0357037);
	translation = ofVec3f(-70.8, -71.8, 2680.2);

    gui.loadFromFile("defaultSettings.xml");
//	ofSetLogLevel(ofxOpenNIContext::LOG_NAME, OF_LOG_VERBOSE);
//  numDevices = openNIContext->getNumDevices();
	
	renderer1.setup("_calibration");
	renderer2.setDepthOnly();
	//renderer1.setDepthOnly("depthCalib1Refined.yml");
	//renderer2.setDepthOnly("depthCalib2Refined.yml");
	
	//renderer1.setup("_calibration");
	photoPreview.allocate(696, 464, OF_IMAGE_COLOR);
	renderer1.setRGBTexture(photoPreview);
	renderer1.setTextureScaleForImage(photoPreview);
	//renderer2.setDepthOnly();
	
	renderer1.setSimplification(ofVec2f(5,5));
	renderer2.setSimplification(ofVec2f(5,5));

	renderer1.farClip = 1900;
	renderer2.farClip = 1900;
	
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

	markerColors[0] = ofColor(232,   0, 122);
	markerColors[1] = ofColor(  0, 180, 255);
	markerColors[2] = ofColor(212, 255,   0);
	markerColors[3] = ofColor( 73, 240, 159);

	inThreshold = true;
	shouldFireCamera = false;
	
	ofxXmlSettings settings;
	settings.loadFile("calibrationPoints.xml");
	for(int i = 0; i < 4; i++){
		cam1Calib[i].x = settings.getValue("cam1:x" + ofToString(i), 0);
		cam1Calib[i].y = settings.getValue("cam1:y" + ofToString(i), 0);
		cam2Calib[i].x = settings.getValue("cam2:x" + ofToString(i), 0);
		cam2Calib[i].y = settings.getValue("cam2:y" + ofToString(i), 0);
	}

	ofxCv::loadMat(rvec, "rotation.yml");
	ofxCv::loadMat(tvec, "translation.yml");
	

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
			ofPoint center = ofxCv::toOf( contourFinder.getCenter(maxAreaIndex) );
			areaDepth =  depthImage1.getColor( center.x, center.y ).getBrightness();
			//cout << "depth? " << areaDepth << endl;
			if(areaDepth > minTriggerDepth && areaDepth < maxTriggerDepth){
				ghostFrames = 0;
				if(!inThreshold){
					thresholdEnteredTime = ofGetElapsedTimef();
					inThreshold = true;
				}
				float timeInThreshold = ofGetElapsedTimef() - thresholdEnteredTime;
//				cout << "time in threshold " << timeInThreshold << endl;
				if(timeInThreshold > requiredThresholdTime){
					shouldFireCamera = true;
					inThreshold = false;
					cout << "FIRING CAMERA!" << endl;
				}
			}
			else {
				if(ghostFrames++ > 5){
					inThreshold = false;
				}
			}
		}
		else {
			if(inThreshold && ghostFrames++ > 5){
				inThreshold = false;
			}
		}
		
		if(showRGBD && alwaysUpdate){
			renderer1.update();
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
		
		backContourFinder.setThreshold(threshold);
		backContourFinder.setMinArea(minSize);
		
		backContourFinder.findContours(depthImage2);

		if(showRGBD && alwaysUpdate){
			renderer2.update();
		}
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
//		if(showRGBD){
			photoPreview.setFromPixels(camera1.getPhotoPixels());
			renderer1.update();
//		}
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
	
	if(autoFire && shouldFireCamera){
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
		
		glEnable(GL_DEPTH);
		renderer1.drawWireFrame();
		
		ofPushMatrix();
		
//		ofMultMatrix(calibrationMatrix);
//		ofVec3f axis;
//		float angle;
//		rotation.getRotate(angle, axis);
//		ofRotate(-angle, axis.x,axis.y,axis.z);
//		ofTranslate(-translation);
		
		ofMultMatrix(getMatrix());
		renderer2.drawWireFrame();
		
		ofPopMatrix();
		
		glDisable(GL_DEPTH);

		cam.end();
	}
	else{
//		front.getColorImage().draw(0,0);
		//back.getRawIRImage().draw(640, 480);
		depthImage1Rect = ofRectangle(0,0,640,480);
		depthImage2Rect = ofRectangle(640,0,640,480);

		if(depthImage1.isAllocated()) depthImage1.draw(0, 0);
		if(depthImage2.isAllocated()) depthImage2.draw(640, 0);
		
		if(inThreshold){
			ofSetColor(255,200,10);
		}
		else{
			ofSetColor(255);
		}
		contourFinder.draw();
		
		ofPushMatrix();
		ofTranslate(640, 0);
		backContourFinder.draw();
		ofPopMatrix();
		
		ofSetColor(255);
		
		ofPushStyle();
//		for(int i = 0; i < 4; i++){
//			ofSetColor(	markerColors[i] );
//			ofCircle(depthImage1Rect.getTopLeft() + cam1Calib[i], 5);
//			ofCircle(depthImage2Rect.getTopLeft() + cam2Calib[i], 5);
//
//		}
		//draw cross hires
		ofSetLineWidth(3);
		ofSetColor(255, 100, 100, 150);
		ofLine(320, 0, 320, 480);
		ofLine(640+320, 0, 640+320, 480);
		ofLine(0, 240, 640*2, 240);
		ofPopStyle();
		
		ofDrawBitmapString("area depth: " + ofToString(areaDepth,1) + "\n"+
						   "time in thresdhold: " + ofToString(ofGetElapsedTimef() - thresholdEnteredTime,1),
						   10, 500);

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


ofMatrix4x4 testApp::getMatrix(){
	ofMatrix4x4 mat;
	ofQuaternion quat;
	quat.makeRotate(xRotate, ofVec3f(1,0,0),
					yRotate, ofVec3f(0,1,0),
					zRotate, ofVec3f(0,0,1));
	
	mat.makeRotationMatrix(quat);
	mat.translate(xTranslate, yTranslate, zTranslate);
	return mat;
}

void testApp::keyPressed(int key) {
	if(key == ' ') {
		fireCamera();
	}
	
	if(key == '-'){
		showRGBD = !showRGBD;
		if(showRGBD){
			if(camera1.isLiveReady()){
				photoPreview.setFromPixels(camera1.getPhotoPixels());
				photoPreview.resize(696, 464);
			}
			renderer1.update();
		}
	}
	
	if(key == 'p'){
		showPointcloud = !showPointcloud;
	}
	if(key == '='){
		calibrate();
	}
	if(key == 't'){
		saveTestObjs();
	}
	if(key == 'y'){
		saveCombinedMesh("TestCombined.obj");
	}
	
//	if(key == '2'){
//		camera2.takePhoto();
//	}
}

void testApp::mousePressed(int x, int y,int button ){
//	cout << "mouse pressed!" << endl;
	
	ofVec2f camvec(x,y);
	ofVec2f offset = depthImage1Rect.getTopLeft();
	if(ofGetKeyPressed('1') && depthImage1Rect.inside(camvec)){
		cam1Calib[0] = camvec - offset;
	}
	if(ofGetKeyPressed('2') && depthImage1Rect.inside(camvec)){
		cam1Calib[1] = camvec - offset;
	}
	if(ofGetKeyPressed('3') && depthImage1Rect.inside(camvec)){
		cam1Calib[2] = camvec - offset;
	}
	if(ofGetKeyPressed('4') && depthImage1Rect.inside(camvec)){
		cam1Calib[3] = camvec - offset;
	}
	
	offset = depthImage2Rect.getTopLeft();
	if(ofGetKeyPressed('5') && depthImage2Rect.inside(camvec)){
		cam2Calib[0] = camvec - offset;
	}
	if(ofGetKeyPressed('6') && depthImage2Rect.inside(camvec)){
		cam2Calib[1] = camvec - offset;
	}
	if(ofGetKeyPressed('7') && depthImage2Rect.inside(camvec)){
		cam2Calib[2] = camvec - offset;
	}
	if(ofGetKeyPressed('8') && depthImage2Rect.inside(camvec)){
		cam2Calib[3] = camvec - offset;
	}

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
	
	saveCombinedMesh( f.getAbsolutePath() + "/mesh.obj" );
	//stitch together mesh
//	ofxObjLoader::save(f.getAbsolutePath() + "/mesh1.obj", mesh1);
//	ofxObjLoader::save(f.getAbsolutePath() + "/mesh2.obj", mesh2);
//	ofQuaternion rotation(0.001134582, 0.9988741, 0.03121687, 0.0357037);
//	ofVec3f translation(-70.8, -71.8, 2680.2);
	
	//front.getColorImage().saveImage(f.getAbsolutePath() + "/backTexture.png");
	currentSaveDirectory = f.getAbsolutePath();
	
	//		ofxOscMessage m;
	//		m.setAddress("/mesh");
	//		m.addStringArg("file://"+f.getAbsolutePath());
	//		sender.sendMessage(m);
}

//--------------------------------------------------------------
void testApp::saveCombinedMesh(string path){
	ofMesh mesh;
	renderer1.getReducedMesh(mesh, true, false, true);
	cout << "renderer 1 " << mesh.getNumVertices() << " verts and " << mesh.getNumTexCoords() << " tex coords " << endl;
	renderer2.getReducedMesh(mesh, true, false, true, getMatrix());
	
	while(mesh.getNumTexCoords() < mesh.getNumVertices()){
		mesh.addTexCoord(ofVec2f(0,0));
	}
	cout << "mesh has " << mesh.getNumVertices() << " verts and " << mesh.getNumTexCoords() << " tex coords " << endl;
	
//	int originalIndeces = mesh1.getNumVertices();
//	ofQuaternion fixedrot;
//	ofVec3f axis;
//	float angle;
//	rotation.getRotate(angle, axis);
//	fixedrot.makeRotate(170, 0,1,0);
//	
//	for(int i = 0; i < mesh2.getNumVertices(); i++){
//		mesh1.addVertex( (rotation * mesh2.getVertices()[i]) + translation );
//		mesh1.addTexCoord(ofVec2f(0,0)); //TODO back texture
//	}
//	for(int i = 0; i < mesh2.getNumIndices(); i++){
//		mesh1.addIndex(originalIndeces + mesh2.getIndexPointer()[i]);
//	}
//	
	//stitch!
	ofIndexType curIndex = 0;
	for(int y = 0; y < depthImage1.getHeight() / renderer1.getSimplification().y; y++){
		//find the corresponding index based on the pixel
		bool inside = false;

		for(int x = 0; x < depthImage2.getHeight() / renderer1.getSimplification().y; x++){
			//find the edge jumps that are inside
			if(renderer1.isIndexValid(curIndex) && !inside){
				inside = true;
			}
		}
	}
	
	ofxObjLoader::save(path, mesh);
}

//--------------------------------------------------------------
void testApp::calibrate(){
	
	ofxCv::Calibration cam1CalibBase, cam2CalibBase, cam1CalibRefined, cam2CalibRefined;
	cam1CalibBase.load("depthCalibBase.yml");
	cam2CalibBase.load("depthCalibBase.yml");
	ofxRGBDepthCalibration calibrator;
//	calibrator.refineDepthCalibration(cam1CalibBase, cam1CalibRefined, &front);
//	calibrator.refineDepthCalibration(cam2CalibBase, cam2CalibRefined, &back);
	
	vector<cv::Point2f> imagePoints1,imagePoints2;
	
	for(int i = 0; i < 4; i++){
		imagePoints1.push_back(ofxCv::toCv(cam1Calib[i]));
		imagePoints2.push_back(ofxCv::toCv(cam2Calib[i]));
	}
	cout << "START CAM CALIB IS " << cam1Calib[0] << endl;
	
//	vector<cv::Point2f> ; = ofxCv::toCv(cam2Calib);
	vector<cv::Point3f> objectPoints1;
	vector<cv::Point3f> objectPoints2;
	cout << "object points " << endl;
	for(int i = 0; i < 4; i++){
		objectPoints1.push_back( ofxCv::toCv( front.getWorldCoordinateAt(imagePoints1[i].x, imagePoints1[i].y) ));
		objectPoints2.push_back( ofxCv::toCv(  back.getWorldCoordinateAt(imagePoints2[i].x, imagePoints2[i].y) ));
		
		cout << "	" << objectPoints1[i] << " vs " << objectPoints2[i] << endl;
	}
	
	cv::Mat camMatrix1,camMatrix2;
	cv::Mat distCoeff1,distCoeff2;
	cam1CalibRefined.getDistortedIntrinsics().getCameraMatrix().copyTo( camMatrix1 );
	cam2CalibRefined.getDistortedIntrinsics().getCameraMatrix().copyTo( camMatrix2 );
	cam1CalibRefined.getDistCoeffs().copyTo(distCoeff1);
	cam2CalibRefined.getDistCoeffs().copyTo(distCoeff2);
	
	//cv::Mat affine3d;
	//cv::Mat inliers;
	cv::Mat outputMat(3,4,CV_64F);
	vector<uchar> inliers;
	cv::estimateAffine3D(objectPoints1, objectPoints2, outputMat, inliers);
	
	double* af = outputMat.ptr<double>(0);
	calibrationMatrix =  ofMatrix4x4(af[0], af[4], af[8], 0.0f,
									 af[1], af[5], af[9], 0.0f,
									 af[2], af[6], af[10], 0.0f,
									 af[3], af[7], af[11], 1.0f);
	
//	calibrationMatrix =  ofMatrix4x4(af[0], af[1], af[2], 0.0f,
//									 af[4], af[5], af[6], 0.0f,
//									 af[8], af[9], af[10], 0.0f,
//									 af[3], af[7], af[11], 1.0f);
	
//	calibrationMatrix =  ofMatrix4x4(1, 0, 0, 0.0f,
//									 0, 1, 0, 0.0f,
//									 0, 0, 1, 0.0f,
//									 af[3], af[7], af[11], 1.0f);
		
	cout << "affine: " << endl << outputMat << endl;
//	cout << "matrix: " << endl << inliers << endl;

//	cam1CalibRefined.save("depthCalib1Refined.yml");
//	cam2CalibRefined.save("depthCalib2Refined.yml");
//	renderer1.setDepthOnly("depthCalib1Refined.yml");
//	renderer2.setDepthOnly("depthCalib2Refined.yml");
	
	saveCalibrationPoints();

//	cv::Mat E,F;
//	cv::stereoCalibrate(objectPoints, imagePoints1, imagePoints2,
//						camMatrix1, distCoeff1,
//						camMatrix2, distCoeff2,
//						cv::Size(640,480), rvec, tvec, E, F);
//	cv::solvePnPRansac(objectPoints, imagePoints2, camMatrix2, distCoeff2, rvec, tvec);
	
//	ofxCv::saveMat(rvec, "rotation.yml");
//	ofxCv::saveMat(tvec, "translation.yml");
	
//	cv::Mat m3x3;
//	cv::Rodrigues(rvec, m3x3);
//	calibrationMatrix.set(1, 0, 0, tvec.at<double>(1,0),
//						  0, 1, m3x3.at<double>(2,1), tvec.at<double>(2,0),
//						  0, 0, m3x3.at<double>(2,2), tvec.at<double>(3,0),
//						  0, 0, 0, 1);
////	calibrationMatrix.set(m3x3.at<double>(0,0), m3x3.at<double>(1,0), m3x3.at<double>(2,0), tvec.at<double>(1,0),
////						  m3x3.at<double>(0,1), m3x3.at<double>(1,1), m3x3.at<double>(2,1), tvec.at<double>(2,0),
////						  m3x3.at<double>(0,2), m3x3.at<double>(1,2), m3x3.at<double>(2,2), tvec.at<double>(3,0),
////						  0, 0, 0, 1);
	
//	calibrationMatrix.getTransposedOf(calibrationMatrix);
//    calibrationMatrix = ofxCv::makeMatrix(rvec, tvec);
	
	cout << "calibrated!" << endl;
	cout << "rotation: " << endl << rvec << endl;
	cout << "translation: " << endl << tvec << endl;
	
	cout << "START CAM CALIB IS " << cam1Calib[0] << endl;
//	saveCalibrationPoints();
}

//--------------------------------------------------------------
void testApp::exit(){
    gui.saveToFile("defaultSettings.xml");
}

void testApp::saveCalibrationPoints(){
	ofxXmlSettings settings;
	settings.addTag("cam1");
	settings.addTag("cam2");
	for(int i = 0; i < 4; i++){
		settings.setValue("cam1:x" + ofToString(i), cam1Calib[i].x);
		settings.setValue("cam1:y" + ofToString(i), cam1Calib[i].y);
		settings.setValue("cam2:x" + ofToString(i), cam2Calib[i].x);
		settings.setValue("cam2:y" + ofToString(i), cam2Calib[i].y);
	}
	settings.saveFile("calibrationPoints.xml");	
}

void testApp::saveTestObjs(){
	ofMesh mesh1 = renderer1.getReducedMesh(true, false, true);
	ofMesh mesh2 = renderer2.getReducedMesh(true, false, true);
	ofxObjLoader::save("Test1.obj", mesh1);
	ofxObjLoader::save("Test2.obj", mesh2);
	
}


