#pragma once

#include "ofMain.h"
#include "ofxEdsdk.h"
#include "ofxDepthImageProviderOpenNI.h"
#include "ofxDepthImageCompressor.h"
#include "ofxRGBDCPURenderer.h"
#include "ofxRGBDepthCalibration.h"
#include "ofxObjLoader.h"
#include "ofxOsc.h"
#include "ofxGameCamera.h"
#include "ofxGui.h"

class testApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	void keyPressed(int key);
	
	void mousePressed(int x, int y, int button);
	
	ofImage pix;
	ofxEdsdk::Camera camera1;
//	ofxEdsdk::Camera camera2;
    int numDevices;
//	ofxOpenNI* openNIDevices[MAX_DEVICES];
	
	ofxDepthImageProviderOpenNI front;
	ofxDepthImageProviderOpenNI back;
	ofxDepthImageCompressor compressor;
	ofImage depthImage1;
	ofImage depthImage2;
	ofRectangle depthImage1Rect;
	ofRectangle depthImage2Rect;
	
	ofxPanel gui;
	ofxFloatSlider xshift;
    ofxFloatSlider yshift;
    ofxFloatSlider xsimplify;
    ofxFloatSlider ysimplify;
	ofxFloatSlider threshold;
	ofxFloatSlider minSize;
	ofxFloatSlider minTriggerDepth;
	ofxFloatSlider maxTriggerDepth;
	
	ofxToggle alwaysUpdate;
	
	ofxFloatSlider xTranslate;
	ofxFloatSlider yTranslate;
	ofxFloatSlider zTranslate;
	
	ofxFloatSlider xRotate;
	ofxFloatSlider yRotate;
	ofxFloatSlider zRotate;
	
	ofxToggle autoFire;

	ofxRGBDCPURenderer renderer1;
	ofxRGBDCPURenderer renderer2;

	ofImage photoPreview;
	ofxOscSender sender;
	ofxGameCamera cam;
	
	ofMatrix4x4 getMatrix();
	
	bool showRGBD;
	bool showPointcloud;
	
	void saveTestObjs();
	void exit();
	string currentSaveDirectory;
	ofQuaternion rotation;
	ofVec3f translation;

	ofxCv::ContourFinder contourFinder;
	ofxCv::ContourFinder backContourFinder;
	
	void fireCamera();
	float areaDepth;
	bool inThreshold;
	bool shouldFireCamera;
	
	float timeBetweenTriggers;
	float timeOfNextTrigger;
	float thresholdEnteredTime;
	float requiredThresholdTime;
	int ghostFrames;
	
	vector<ofVec2f> cam1Calib;
	vector<ofVec2f> cam2Calib;
	
	ofColor markerColors[4];
	void calibrate();
	void saveCalibrationPoints();
	cv::Mat rvec;
	cv::Mat tvec;
	ofMatrix4x4 calibrationMatrix;
	void saveCombinedMesh(string path);
	
};


