#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
	
	ofBackground(0);
	ofSetVerticalSync(true);
	
	sender.setup("localhost", 12345);
	loadFolder("snapscans");

}

//--------------------------------------------------------------
void testApp::update(){
	
	if(sendOrder.size() == 0){
		return;
	}
	
	if( (ofGetElapsedTimef() - lastFiredTime) > timeBetween ){
		sendNext();
	}
}

//--------------------------------------------------------------
void testApp::sendNext(){
	ofxOscMessage m;
	m.setAddress( "/mesh" );
	string fileName = "file://"+ ofFilePath::getAbsolutePath( files.getPath( sendOrder[ currentOneToSend ]) );
	m.addStringArg( fileName );
	sender.sendMessage(m);
	
	cout << "firing " << fileName << endl;
	
	currentOneToSend = (currentOneToSend + 1) % sendOrder.size();
	timeBetween = baseTimeBetween + ofRandom(-10,10);
	
	lastFiredTime = ofGetElapsedTimef();
	
}

//--------------------------------------------------------------
void testApp::draw(){
	ofDrawBitmapString( "Press 's' to force send next mesh.\nLoaded " + ofToString(sendOrder.size()) + " meshes. Next fire: " + ofToString(timeBetween - (ofGetElapsedTimef() - lastFiredTime), 2), 10,10 );
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
	if(key == 's'){
		sendNext();		ofxOscMessage m;
		m.setAddress( "/mesh" );
		string fileName = "file://"+ ofFilePath::getAbsolutePath( files.getPath( sendOrder[ currentOneToSend ]) );
		m.addStringArg( fileName );
		sender.sendMessage(m);
		
		cout << "firing " << fileName << endl;
		
		currentOneToSend = (currentOneToSend + 1) % sendOrder.size();
		timeBetween = baseTimeBetween + ofRandom(-10,10);
		
		lastFiredTime = ofGetElapsedTimef();
	}
}

//--------------------------------------------------------------
void testApp::loadFolder(string path){
	
	files = ofDirectory(path);
	
	if(files.exists()){
		files.listDir();
		
		for(int i = 0; i < files.numFiles(); i++){
			sendOrder.push_back(i);
		}
		
		ofRandomize(sendOrder);
		
		currentOneToSend = 0;
		
		timeBetween = 30;
		baseTimeBetween = 30;
		
		lastFiredTime = ofGetElapsedTimef();
		
	}
	
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){
	loadFolder(dragInfo.files[0]);
}