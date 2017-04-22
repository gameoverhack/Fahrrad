#include "CamController.h"

//--------------------------------------------------------------
CamController::CamController() {
	className = "CamController";
	ofLogNotice() << className << ": constructor";
}

//--------------------------------------------------------------
CamController::~CamController() {
	ofLogNotice() << className << ": destructor";

	// kill the thread
	waitForThread();

	this->IGuiBase::~IGuiBase(); // call base destructor
}

//--------------------------------------------------------------
void CamController::setup() {
	ofLogNotice() << className << ": setup";

	// call base clase setup for now
	IGuiBase::setup();

	bSetImageStorePath = false;

	currentPhotoState = PHOTO_NONE;

	currentSensorMode = SENSOR_NONE; // so we force sensor mode change in update
	timeSinceLastSensor = 0;

	//setup Camera
	float w = 1280.0;
	float h = 720.0;

	cam.setDeviceID(0);
	cam.setDesiredFrameRate(60);
	cam.setPixelFormat(OF_PIXELS_NATIVE);
	cam.initGrabber(w, h);

	//setup Audio
	soundPlayerCountdown.load(ofToDataPath("audio/countdown.wav"));
	soundPlayerShutter.load(ofToDataPath("audio/shutter.wav"));



	// setup shader
#ifdef TARGET_WIN32
	bool didLoadShader = shader.load(ofToDataPath("shader/passWIN.vert"), ofToDataPath("shader/colorWIN.frag"), "");
#else
	bool didLoadShader = shader.load(ofToDataPath("shader/passGLES.vert"), ofToDataPath("shader/colorGLES.frag"), "");
#endif

	if (!didLoadShader)
	{
		ofLogError() << "Load Shader FAIL";
	}

	fbo.allocate(w, h, GL_RGB);

	pixels.allocate(w, h, OF_IMAGE_COLOR);

	// set up mesh to draw to
	mesh.addVertex(ofVec3f(0, 0, 0));
	mesh.addVertex(ofVec3f(w, 0, 0));
	mesh.addVertex(ofVec3f(0, h, 0));
	mesh.addVertex(ofVec3f(w, h, 0));

#ifdef TARGET_WIN32
	mesh.addTexCoord(ofVec2f(0, 0));
	mesh.addTexCoord(ofVec2f(w, 0));
	mesh.addTexCoord(ofVec2f(0, h));
	mesh.addTexCoord(ofVec2f(w, h));
#else
	mesh.addTexCoord(ofVec2f(0, 0));
	mesh.addTexCoord(ofVec2f(1, 0));
	mesh.addTexCoord(ofVec2f(0, 1));
	mesh.addTexCoord(ofVec2f(1, 1));
#endif

	mesh.addIndex(0);
	mesh.addIndex(1);
	mesh.addIndex(3);
	mesh.addIndex(0);
	mesh.addIndex(3);
	mesh.addIndex(2);

	startThread();
}

//--------------------------------------------------------------
void CamController::setDefaults() {
	ofLogNotice() << className << ": setDefaults";

#ifdef TARGET_WIN32
	imgStorePath = "C:/Users/JB/Desktop/img";
#else
	imgStorePath = "/home/pi/Desktop/img";
#endif

	brightness = 0.15;
	contrast = 1.0;
	saturation = 1.0;
}

//--------------------------------------------------------------
void CamController::update() {

	if (!bUse) return;

	if (bSetImageStorePath) {
#ifndef TARGET_WIN32
		ofSetWindowShape(10, 10);
#endif
		ofFileDialogResult result = ofSystemLoadDialog("Select Save Folder", true);
		if (result.getPath() != "") {
			imgStorePath = result.getPath();
		}
#ifndef TARGET_WIN32
		ofSetWindowShape(1920, 1080);
#endif
	}

	// check thread safe GUI changes
	lock();

	// check what mode we're in
	if (nextSensorMode != currentSensorMode) {

		// change sensor mode
		changeMode();

		// set current to next mode
		currentSensorMode = nextSensorMode;
	}

	unlock();

	// doing all velocity updates in threadedUpdate for now

	// lock the model
	// copy to the appModel
	// unlock the model

	cam.update();

	if (cam.isFrameNew()) {

		fbo.begin();
		{
			ofClear(0, 0, 0, 0);
			shader.begin();
			{
#ifdef TARGET_WIN32
				shader.setUniformTexture("tex", cam.getTexture(), 0);
#else
				shader.setUniformTexture("Ytex", cam.getTexturePlanes()[0], 0);
				shader.setUniformTexture("Utex", cam.getTexturePlanes()[1], 1);
				shader.setUniformTexture("Vtex", cam.getTexturePlanes()[2], 2);
				shader.setUniform2f("tex_scaleY", 1.0, 1.0);
				shader.setUniform2f("tex_scaleU", 1.0, 1.0);
				shader.setUniform2f("tex_scaleV", 1.0, 1.0);
#endif
				shader.setUniform1f("brightness", brightness);
				shader.setUniform1f("contrast", contrast);
				shader.setUniform1f("saturation", saturation);

				mesh.draw();

			}
			shader.end();
			//cam.draw(0, 0); // thanks openGL we need to fix the shader
		}
		fbo.end();
	}
}

//--------------------------------------------------------------
void CamController::changeMode() {

	// shutdown the current mode

	ofLogNotice() << "Shutting down: " << sensorModes[currentSensorMode];

	switch (currentSensorMode) {
	case SENSOR_TEENSY:
	{

	}
	break;
	case SENSOR_GPIO:
	{

	}
	break;
	}

	// setup for the next mode

	ofLogNotice() << "Setting up: " << sensorModes[nextSensorMode];

	switch (nextSensorMode) {
	case SENSOR_TEENSY:
	{

	}
	break;
	case SENSOR_GPIO:
	{

	}
	break;
	}

	lastSensorTimeout = ofGetElapsedTimeMillis();

}

//--------------------------------------------------------------
void CamController::threadedFunction() {

	while (isThreadRunning()) {

		if (bUse) {

			lock();

			// grab the current mode
			int thisCurrentMode = currentSensorMode;



			// calculate the amount of time since the last sensor measurement
			timeSinceLastSensor = ofGetElapsedTimeMillis() - lastSensorTimeout;

			unlock();


			switch (thisCurrentMode) {
			case SENSOR_SIMULATE:
			{
				double simulateTimeout = 10000.0; // mm / mm/s * 1000.0 = milliseconds
				if (timeSinceLastSensor >= simulateTimeout) {
					triggerSensor(SENSOR_SIMULATE);
				}
			}
			break;
			case SENSOR_TEENSY:
			{

			}
			break;
			case SENSOR_GPIO:
			{

			}
			break;
			}


			lock();

			// grab the current photo state
			int thisCurrentPhotoState = currentPhotoState;

			unlock();


			// handle the current photo state and lock and unlock only when changing state

			switch (thisCurrentPhotoState) {
			case PHOTO_NONE:
			{

			}
			break;
			case PHOTO_REQUESTED:
			{
				playCountdownSound();
			}
			break;
			case PHOTO_COUNTDOWN:
			{
				if (!soundPlayerCountdown.isPlaying()) {
					lock();
					if (currentPhotoState == PHOTO_COUNTDOWN) {
						currentPhotoState = PHOTO_SAVEIMAGE;
					}
					unlock();
				}
			}
			break;
			case PHOTO_SAVEIMAGE:
			{
				saveIMG();
			}
			break;
			case PHOTO_FINISHING:
			{
				playShutterSound();
			}
			break;
			case PHOTO_FINISHED:
			{
				if (!soundPlayerShutter.isPlaying()) {
					lock();
					if (currentPhotoState == PHOTO_FINISHED) {
						currentPhotoState = PHOTO_NONE;
					}
					unlock();
				}
			}
			break;
			}


		}

		ofSleepMillis(1);

	}

}

//--------------------------------------------------------------
void CamController::triggerSensor(SensorMode sensorMode) {
	lock();

	if (sensorMode == currentSensorMode) {
		if (currentPhotoState == PHOTO_NONE) {
			currentPhotoState = PHOTO_REQUESTED;
		}
	}

	unlock();

}

//--------------------------------------------------------------
void CamController::playCountdownSound() {
	lock();
	if (currentPhotoState == PHOTO_REQUESTED) {
		currentPhotoState = PHOTO_COUNTDOWN;
	}
	unlock();

	soundPlayerCountdown.play();
}

//--------------------------------------------------------------
void CamController::playShutterSound() {
	lock();
	if (currentPhotoState == PHOTO_FINISHING) {
		currentPhotoState = PHOTO_FINISHED;
	}
	unlock();

	soundPlayerShutter.play();
}

//--------------------------------------------------------------
void CamController::saveIMG() {

	if (currentPhotoState == PHOTO_SAVEIMAGE) {
		currentPhotoState = PHOTO_FINISHING;
	}

	//get img name from timestamp
	string savename = ofGetTimestampString();

	lock();
	pixels.clear();

	//get frame buffer pixels
	fbo.readToPixels(pixels);

	//save
	//toDO still no image saved 
	ofSaveImage(pixels, imgStorePath + "/" + savename + ".jpg", OF_IMAGE_QUALITY_BEST);

	//toDo check if Image was saved

	unlock();

}

//--------------------------------------------------------------
void CamController::drawGUI() {

	if (ImGui::CollapsingHeader(className.c_str())) {
		beginGUI();
		{
			bSetImageStorePath = ImGui::Button("Set Image Save Path");

			if (ImGui::CollapsingHeader("Render Settings"))
			{

				ImGui::SliderFloat("Image brightness", &brightness, 0.0, 1.0);
				if (ImGui::Button("reset brightness")) brightness = 0.15;
				ImGui::Spacing();

				ImGui::SliderFloat("Image contrast", &contrast, 0.0, 10.0);
				if (ImGui::Button("reset contrast")) contrast = 1.0;
				ImGui::Spacing();

				ImGui::SliderFloat("Image saturation", &saturation, 0.0, 10.0);
				if (ImGui::Button("reset saturation")) saturation = 1.0;
				ImGui::Spacing();

			}

			ImGui::Combo("Sensor Mode", (int*)&nextSensorMode, sensorModes);

			switch (nextSensorMode) {
			case SENSOR_SIMULATE:
			{

			}
			break;
			case SENSOR_TEENSY:
			{

			}
			break;
			case SENSOR_GPIO:
			{

			}
			break;
			}

			lock();

			ImGui::NewLine();
			ImGui::Text("Time since last sensor reading : %.0f millis", timeSinceLastSensor);

			unlock();

		}
		endGUI();
	}
}

//--------------------------------------------------------------
ofTexture & CamController::getCameraTexture() {
	return fbo.getTextureReference(0);
}

//--------------------------------------------------------------
bool CamController::loadParameters() {
	return Serializer.loadClass(fixPath("configs/" + className + CONFIG_TYPE), (*this), ARCHIVE_BINARY);
}

//--------------------------------------------------------------
bool CamController::saveParameters() {
	return Serializer.saveClass(fixPath("configs/" + className + CONFIG_TYPE), (*this), ARCHIVE_BINARY);
}