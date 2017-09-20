#pragma once

#include "ofMain.h"
#include "ofxImGui.h"
#include "IGuiBase.h"

#include "BicycleController.h"
#include "VideoController.h"
#include "RenderController.h"
#include "ImageCaptureController.h"
#include "ImageDisplayController.h"
#include "NetworkController.h"
#include "ViewController.h"
#include "PulseController.h"

class ofApp : public ofBaseApp, public IGuiBase {

public:

	void setup();
	void update();
	void draw();
	void exit();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);

	// IGuiBase methods
	void drawGUI();
	void changeMode();
	void setDefaults();
	bool loadParameters();
	bool saveParameters();

	typedef enum {
		APPLICATION_NONE = 0,
		APPLICATION_BIKEVIDEO1,
		APPLICATION_BIKEVIDEO2,
		APPLICATION_STATSLOCAL,
		APPLICATION_STATSREMOTE,
		APPLICATION_CAMERALOCAL,
		APPLICATION_CAMERAREMOTE,
		APPLICATION_DEBUG
	} ApplicationMode;

	vector<string> applicationModes = {
		"APPLICATION_NONE",
		"APPLICATION_BIKEVIDEO1",
		"APPLICATION_BIKEVIDEO2",
		"APPLICATION_STATSLOCAL",
		"APPLICATION_STATSREMOTE",
		"APPLICATION_CAMERALOCAL",
		"APPLICATION_CAMERAREMOTE",
		"APPLICATION_DEBUG"
	};

	ApplicationMode nextApplicationMode;
	ApplicationMode currentApplicationMode;

	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version) {
		ar & BOOST_SERIALIZATION_NVP(nextApplicationMode);
	}

	bool bShowDebug;
	bool bShowFullScreen;
	bool bShowCursor;

	ofxImGui gui;

	BicycleController * bicycleController;
	VideoController * videoController;
	ImageCaptureController * imageCaptureController;
	ImageDisplayController * imageDisplayController;
	RenderController * renderController;
	NetworkController * networkController;
	ViewController * viewController;
	PulseController * pulseController;

	ofTrueTypeFont font;

};
