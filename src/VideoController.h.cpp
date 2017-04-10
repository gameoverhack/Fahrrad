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

	listDirectory();

}

//--------------------------------------------------------------
void VideoController::setDefaults() {
	ofLogNotice() << className << ": setDefaults";

	videoPath = "C:/Users/gameover8/Desktop/video";
	currentVideoIndex = 0;
	speedUpdateTimeout = 1000;

}

//--------------------------------------------------------------
void VideoController::update() {
	if (!bUse) return;

	if (bSetVideoPath) {
		ofFileDialogResult result = ofSystemLoadDialog("Select Video Folder", true);
		if (result.getPath() != "") {
			videoPath = result.getPath();
			listDirectory();
		}
	}

	if (nextVideoIndex != currentVideoIndex) {

		vid.stop();

		if (nextVideoIndex != 0) {
			ofLogVerbose() << "Loading video: " << videoFilePaths[nextVideoIndex];

#ifdef TARGET_WIN32
			vid.load(videoFilePaths[nextVideoIndex]);
			vid.play();
#else
			vid.load(videoFilePaths[nextVideoIndex]);
			vid.play();
#endif
			
		}

		currentVideoIndex = nextVideoIndex;
	}

	vid.update();

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
	return vid.getTexture();
}

//--------------------------------------------------------------
bool VideoController::loadParameters() {
	return Serializer.loadClass(fixPath("configs/" + className + CONFIG_TYPE), (*this), ARCHIVE_BINARY);
}

//--------------------------------------------------------------
bool VideoController::saveParameters() {
	return Serializer.saveClass(fixPath("configs/" + className + CONFIG_TYPE), (*this), ARCHIVE_BINARY);
}