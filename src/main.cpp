#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main( ){

	// setup the GL context

#ifdef TARGET_WIN32
	ofGLWindowSettings settings;
	settings.setGLVersion(2, 1); // in shader this is #version 120
	//settings.setGLVersion(4, 0); // in shader this is #version 150
#else
	ofGLESWindowSettings settings;
	settings.glesVersion = 2;
#endif

	settings.width = 1920;
	settings.height = 1080;
	ofCreateWindow(settings);

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp(new ofApp());

}
