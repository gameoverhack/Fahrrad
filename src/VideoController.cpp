#include "VideoController.h"

//--------------------------------------------------------------
VideoController::VideoController() {
	className = "VideoController";
	ofLogNotice() << className << ": constructor";
}

//--------------------------------------------------------------
VideoController::~VideoController() {
	ofLogNotice() << className << ": destructor";
	vid.close();
	this->IGuiBase::~IGuiBase(); // call base destructor
}

//--------------------------------------------------------------
void VideoController::setup() {
	ofLogNotice() << className << ": setup";

	// call base clase setup for now
	IGuiBase::setup();

	bRewindPending = false;
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
    	videoPath = "/home/pi/Desktop/video";
#endif
	currentVideoIndex = 0;
	speedUpdateTimeout = 1000;
	videoFadeThreshold = 10.0f;
	pauseFrame = 200;
	startLoopFrame = 400;
	endLoopFrame = 600;
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

		if (nextVideoIndex >= videoFilePaths.size()) nextVideoIndex = 0;

		if (nextVideoIndex != 0 && videoFilePaths[nextVideoIndex] != "") {
			ofLogNotice() << "Loading video: " << videoFilePaths[nextVideoIndex];

#ifdef TARGET_WIN32
            		vid.stop();
			vid.load(videoFilePaths[nextVideoIndex]);
			vid.setLoopState(OF_LOOP_NONE);
			vid.setFrame(pauseFrame);
			vid.play();
			vid.setPaused(true);
#else
            		ofxOMXPlayerSettings settings;
            		settings.videoPath = videoFilePaths[nextVideoIndex];
            		settings.useHDMIForAudio = false;	//default true
            		settings.enableLooping = false;		//default true
            		settings.enableTexture = true;		//default true
			vid.setup(settings);
			vid.seekToTimeInSeconds(pauseFrame * 25);
			vid.setPaused(true);
#endif

		}

		currentVideoIndex = nextVideoIndex;
	}

#ifdef TARGET_WIN32
	vid.update();
#endif

	if (vid.getCurrentFrame() > endLoopFrame) {
#ifdef TARGET_WIN32
		vid.setFrame(startLoopFrame);
#else
		vid.seekToTimeInSeconds(startLoopFrame / 25.0);
#endif
	}

}

//--------------------------------------------------------------
void VideoController::listDirectory() {

	dir.allowExt("mov");
	dir.allowExt("mp4");
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
			ImGui::SliderFloat("Video Fade Threshold (km/h)", &videoFadeThreshold, 0.0f, 40.0f);
			float frame = vid.getCurrentFrame();
			ImGui::SliderFloat("Current video frame", &frame, 0.0f, vid.getTotalNumFrames());
			ImGui::SliderInt("Pause frame", &pauseFrame, 0, 400);
			ImGui::SliderInt("Start loop frame", &startLoopFrame, 0, 400);
			ImGui::SliderInt("End loop frame", &endLoopFrame, vid.getTotalNumFrames() - 400, vid.getTotalNumFrames());
		}
		endGUI();
	}
}

//--------------------------------------------------------------
void VideoController::setSpeed(float speed) {

	if (ofGetElapsedTimeMillis() - lastSpeedUpdateTime >= speedUpdateTimeout) {

		if(bRewindPending && vid.getCurrentFrame() >= pauseFrame + 5) {
			vid.setPaused(true);
		}

		if (speed == 0.0f) {
			//vid.setPaused(true);
		}
		else {
			if (vid.isPaused()) {
				vid.setPaused(false);
			}
			bRewindPending = false;
			vid.setSpeed(speed);
		}
		

		lastSpeedUpdateTime = ofGetElapsedTimeMillis();
	}

}

//--------------------------------------------------------------
const float & VideoController::getVideoFadeThreshold() {
	return videoFadeThreshold;
}

//--------------------------------------------------------------
const ofTexture & VideoController::getVideoTexture() {
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
void VideoController::fastforward() {
#ifdef TARGET_WIN32
	if (vid.isLoaded()) {
		vid.setFrame(endLoopFrame - 200);
#else
	if (vid.getIsOpen()) {
		vid.seekToTimeInSeconds((endLoopFrame - 200) / 25.0);
#endif
	}
}

//--------------------------------------------------------------
void VideoController::rewind() {
#ifdef TARGET_WIN32
	if (vid.getCurrentFrame() != pauseFrame && !bRewindPending && vid.isLoaded()) {
		vid.setFrame(pauseFrame);
#else
	if (vid.getCurrentFrame() != pauseFrame && !bRewindPending && vid.getIsOpen()) {
		vid.seekToTimeInSeconds(pauseFrame / 25.0);
#endif
		//vid.setPaused(true);
		bRewindPending = true;
	}
}

//--------------------------------------------------------------
int VideoController::isLooping() {
	return vid.getCurrentFrame() >= startLoopFrame;
}

//--------------------------------------------------------------
bool VideoController::loadParameters() {
	return Serializer.loadClass(ofToDataPath("configs/" + configPath + "/" + className + CONFIG_TYPE), (*this), ARCHIVE_BINARY);
}

//--------------------------------------------------------------
bool VideoController::saveParameters() {
	return Serializer.saveClass(ofToDataPath("configs/" + configPath + "/" + className + CONFIG_TYPE), (*this), ARCHIVE_BINARY);
}
