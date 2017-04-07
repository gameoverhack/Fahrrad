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
	listDirectory();
}

//--------------------------------------------------------------
void VideoController::setDefaults() {
	ofLogNotice() << className << ": setDefaults";

	videoPath = "C:/Users/gameover8/Desktop/video";
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
}

//--------------------------------------------------------------
void VideoController::listDirectory() {
	dir.allowExt("mov");
	dir.listDir(videoPath);
}

//--------------------------------------------------------------
void VideoController::drawGUI() {
	if (ImGui::CollapsingHeader(className.c_str())) {
		beginGUI();
		{
			bSetVideoPath = ImGui::Button("Set Video Path");
			//ImGui::SliderInt("Time Out", &timeOut, 1, 2000);
		}
		endGUI();
	}
}

//--------------------------------------------------------------
bool VideoController::loadParameters() {
	return Serializer.loadClass(fixPath("configs/" + className + CONFIG_TYPE), (*this), ARCHIVE_BINARY);
}

//--------------------------------------------------------------
bool VideoController::saveParameters() {
	return Serializer.saveClass(fixPath("configs/" + className + CONFIG_TYPE), (*this), ARCHIVE_BINARY);
}