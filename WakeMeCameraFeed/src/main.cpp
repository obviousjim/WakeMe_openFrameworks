#include "testApp.h"
#include "ofAppGlutWindow.h"

int main() {
	ofAppGlutWindow window;
	ofSetupOpenGL(&window, 1480, 840, OF_WINDOW);
	ofRunApp(new testApp());
}
