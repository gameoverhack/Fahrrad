#include "VideoController.h"

//--------------------------------------------------------------
VideoController::VideoController() {
	className = "VideoController";
	ofLogNotice() << className << ": constructor";
}

//--------------------------------------------------------------
VideoController::~VideoController() {
	ofLogNotice() << className << ": destructor";
	this->IGuiBase::~IGuiBase(); // call base destructor
}

//--------------------------------------------------------------
void VideoController::setup() {
	ofLogNotice() << className << ": setup";

	// call base clase setup for now
	IGuiBase::setup();

	bSetVideoPath = false;
	lastSpeedUpdateTime = ofGetElapsedTimeMillis();
#ifndef TARGET_WIN32
    defaultTexture.allocate(1,1,GL_RGB);
#endif

	listDirectory();

}

//--------------------------------------------------------------
void VideoController::setDefaults() {
	ofLogNotice() << className << ": setDefaults";
#ifdef TARGET_WIN32
	videoPath = "C:/Users/gameover8/Desktop/video";
#else
    videoPath = "/home/pi/openFrameworks/addons/ofxOMXPlayer/video";
#endif
	currentVideoIndex = 0;
	speedUpdateTimeout = 1000;

}

//--------------------------------------------------------------
void VideoController::update() {
	if (!bUse) return;

	if (bSetVideoPath) {
#ifndef TARGET_WIN32
    ofSetWindowShape(10, 10);
#endif
		ofFileDialogResult result = ofSystemLoadDialog("Select Video Folder", true);
		if (result.bSuccess){
            if (result.getPath() != "") {
                videoPath = result.getPath();
                listDirectory();
            }
        }
#ifndef TARGET_WIN32
    ofSetWindowShape(1920, 1080);
#endif
	}

	if (nextVideoIndex != currentVideoIndex) {

		if (nextVideoIndex != 0 && videoFilePaths[nextVideoIndex] != "") {
			ofLogVerbose() << "Loading video: " << videoFilePaths[nextVideoIndex];

#ifdef TARGET_WIN32
            vid.stop();
			vid.load(videoFilePaths[nextVideoIndex]);
            vid.play();
#else
            ofxOMXPlayerSettings settings;
            settings.videoPath = videoFilePaths[nextVideoIndex];
            settings.useHDMIForAudio = false;	//default true
            settings.enableLooping = true;		//default true
            settings.enableTexture = true;		//default true
            vid.setup(settings);
#endif

		}

		currentVideoIndex = nextVideoIndex;
	}

#ifdef TARGET_WIN32
	vid.update();
#endif

}

//--------------------------------------------------------------
void VideoController::listDirectory() {

	dir.allowExt("mov");
	dir.listDir(videoPath);

	videoFilePaths.clear();
	videoFilePaths.push_back("NONE");

	for (int i = 0; i < dir.size(); i++) {
		videoFilePaths.push_back(dir.getPath(i));
	}

	nextVideoIndex = currentVideoIndex;
	currentVideoIndex = 0;

}

//--------------------------------------------------------------
void VideoController::drawGUI() {
	if (ImGui::CollapsingHeader(className.c_str())) {
		beginGUI();
		{
			bSetVideoPath = ImGui::Button("Set Video Path");
			ImGui::Combo("Video File", &nextVideoIndex, videoFilePaths);
			ImGui::SliderInt("Speed Update Time (millis)", &speedUpdateTimeout, 0, 2000);
		}
		endGUI();
	}
}

//--------------------------------------------------------------
void VideoController::setSpeed(float speed) {

	if (ofGetElapsedTimeMillis() - lastSpeedUpdateTime >= speedUpdateTimeout) {

		if (speed == 0.0f) {
			vid.setPaused(true);
		} else {
			if (vid.isPaused()) {
				vid.setPaused(false);
			}
			vid.setSpeed(speed);
		}

		lastSpeedUpdateTime = ofGetElapsedTimeMillis();
	}

}

//--------------------------------------------------------------
ofTexture & VideoController::getVideoTexture() {
#ifdef TARGET_WIN32
    return vid.getTexture();
#else
    if(vid.isTextureEnabled()){
        return vid.getTextureReference();
    }else{
        return defaultTexture;
    }

#endif
}

//--------------------------------------------------------------
bool VideoController::loadParameters() {
	return Serializer.loadClass(fixPath("configs/" + className + CONFIG_TYPE), (*this), ARCHIVE_BINARY);
}

//--------------------------------------------------------------
bool VideoController::saveParameters() {
	return Serializer.saveClass(fixPath("configs/" + className + CONFIG_TYPE), (*this), ARCHIVE_BINARY);
}
