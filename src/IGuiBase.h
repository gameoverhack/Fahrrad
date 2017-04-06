#pragma once

#include "ofMain.h"
#include "Utils.h"

class IGuiBase {

public:

	//IGuiBase() {}
	virtual ~IGuiBase() {
	
	}

	virtual void setup() {

		if (className == "UNKNOWN") {
			ofLogError() << "Name not defined for class!";
		}

		if (!loadParameters()) {
			setDefaults();
		}

	}

	virtual void update() = 0;

	virtual void drawGUI() = 0;

	virtual void setDefaults() = 0;

	virtual bool loadParameters() = 0;
	virtual bool saveParameters() = 0;

protected:

	string className = "UNKNOWN";

	bool bUse = true;
	bool bSaveSettings = false;
	bool bLoadSettings = false;
	bool bResetDefaults = false;

	inline void beginGUI() {

		ImGui::PushID(className.c_str());

		ImGui::Checkbox("Use ", &bUse); ImGui::SameLine();
		ImGui::Checkbox("Reset Defaults", &bResetDefaults); ImGui::SameLine();
		ImGui::Checkbox("Save Settings", &bSaveSettings); ImGui::SameLine();
		ImGui::Checkbox("Load Settings", &bLoadSettings);

		if (bResetDefaults) {
			setDefaults();
			bResetDefaults = false;
		}

		if (bSaveSettings) {
			saveParameters();
			bSaveSettings = false;
		}

		if (bLoadSettings) {
			loadParameters();
			bLoadSettings = false;
		}

	}

	inline void endGUI() {
		ImGui::PopID();
	}

};

