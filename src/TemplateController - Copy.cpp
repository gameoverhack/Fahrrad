#include "BicycleController.h"

//--------------------------------------------------------------
BicycleController::BicycleController() {
	className = "BicycleController";
	ofLogNotice() << className << ": constructor";
}

//--------------------------------------------------------------
BicycleController::~BicycleController() {
	ofLogNotice() << className << ": destructor";
	IGuiBase::~IGuiBase(); // call base destructor
}

//--------------------------------------------------------------
void BicycleController::setup() {
	ofLogNotice() << className << ": setup";

	// call base clase setup for now
	IGuiBase::setup();
}

//--------------------------------------------------------------
void BicycleController::update() {
	if (!bUse) return;
}

//--------------------------------------------------------------
void BicycleController::drawGUI() {
	if (ImGui::CollapsingHeader(className.c_str())) {
		beginGUI();
		{
			ImGui::SliderInt("Time Out", &timeOut, 1, 2000);
		}
		endGUI();
	}
}

//--------------------------------------------------------------
void BicycleController::setDefaults() {
	ofLogNotice() << className << ": setDefaults";
	timeOut = 1000;
}

//--------------------------------------------------------------
bool BicycleController::loadParameters() {
	return Serializer.loadClass(ofToDataPath("configs\\" + className + ".conf"), (*this), ARCHIVE_BINARY);
}

//--------------------------------------------------------------
bool BicycleController::saveParameters() {
	return Serializer.saveClass(ofToDataPath("configs\\" + className + ".conf"), (*this), ARCHIVE_BINARY);
}