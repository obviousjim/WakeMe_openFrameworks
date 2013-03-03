#pragma once

#include "ofMain.h"
#include "ofxEdsdk.h"
#include "ofxDepthImageProviderOpenNI.h"
#include "ofxDepthImageCompressor.h"
#include "ofxRGBDCPURenderer.h"
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
	ofxPanel gui;
	ofxFloatSlider xshift;
    ofxFloatSlider yshift;
    ofxFloatSlider xsimplify;
    ofxFloatSlider ysimplify;
	ofxFloatSlider threshold;
	ofxFloatSlider minSize;
	ofxFloatSlider minTriggerDepth;
	ofxFloatSlider maxTriggerDepth;
	
	ofxRGBDCPURenderer renderer1;
	ofxRGBDCPURenderer renderer2;

	ofImage photoPreview;
	ofxOscSender sender;
	ofxGameCamera cam;
	bool showRGBD;
	
	void exit();
	string currentSaveDirectory;
	
	ofxCv::ContourFinder contourFinder;
	void fireCamera();
	
	bool inThreshold;
	bool shouldFireCamera;
	
	float timeBetweenTriggers;
	float timeOfNextTrigger;
	float thresholdEnteredTime;
	float requiredThresholdTime;
	int ghostFrames;
};


