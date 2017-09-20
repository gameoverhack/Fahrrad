#include "ImageCaptureController.h"

/*
                     brightness (int)    : min=-64 max=64 step=1 default=-8193 value=0
                       contrast (int)    : min=0 max=64 step=1 default=57343 value=32
                     saturation (int)    : min=0 max=128 step=1 default=57343 value=60
                            hue (int)    : min=-40 max=40 step=1 default=-8193 value=0
 white_balance_temperature_auto (bool)   : default=1 value=0
                          gamma (int)    : min=72 max=500 step=1 default=57343 value=100
                           gain (int)    : min=0 max=100 step=1 default=57343 value=0
           power_line_frequency (menu)   : min=0 max=2 default=1 value=1
				0: Disabled
				1: 50 Hz
				2: 60 Hz
      white_balance_temperature (int)    : min=2800 max=6500 step=1 default=57343 value=2800
                      sharpness (int)    : min=0 max=6 step=1 default=57343 value=2
         backlight_compensation (int)    : min=0 max=2 step=1 default=57343 value=1
                  exposure_auto (menu)   : min=0 max=3 default=0 value=3
				1: Manual Mode
				3: Aperture Priority Mode
              exposure_absolute (int)    : min=1 max=5000 step=1 default=157 value=157 flags=inactive
         exposure_auto_priority (bool)   : default=0 value=1
*/


//--------------------------------------------------------------
ImageCaptureController::ImageCaptureController() {
	className = "ImageCaptureController";
	ofLogNotice() << className << ": constructor";
}

//--------------------------------------------------------------
ImageCaptureController::~ImageCaptureController() {

	ofLogNotice() << className << ": destructor";

	lock();
	ofRemoveListener(ofxFlickr::APIEvent::events, this, &ImageCaptureController::onFlickrEvent);
	flickr->stop();
	unlock();

	// kill the thread
	waitForThread();
	delete flickr;

	this->IGuiBase::~IGuiBase(); // call base destructor
}

//--------------------------------------------------------------
void ImageCaptureController::setup() {

	ofLogNotice() << className << ": setup";

	camSettingNames = { "brightness",
		"contrast",
		"saturation",
		"hue",
		"white_balance_temperature_auto",
		"gamma",
		"gain",
		"power_line_frequency",
		"white_balance_temperature",
		"sharpness",
		"backlight_compensation",
		"exposure_auto",
		"exposure_absolute",
		"exposure_auto_priority" };

	camDefault["brightness"] = 0;
	camDefault["contrast"] = 32;
	camDefault["saturation"] = 60;
	camDefault["hue"] = 0;
	camDefault["white_balance_temperature_auto"] = 0;
	camDefault["gamma"] = 100;
	camDefault["gain"] = 0;
	camDefault["power_line_frequency"] = 1;
	camDefault["white_balance_temperature"] = 2800;
	camDefault["sharpness"] = 2;
	camDefault["backlight_compensation"] = 1;
	camDefault["exposure_auto"] = 3;
	camDefault["exposure_absolute"] = 157;
	camDefault["exposure_auto_priority"] = 1;

	camMins["brightness"] = -64;
	camMaxs["brightness"] = +64;
	camMins["contrast"] = 0;
	camMaxs["contrast"] = 64;
	camMins["saturation"] = 0;
	camMaxs["saturation"] = 128;
	camMins["hue"] = -40;
	camMaxs["hue"] = +40;
	camMins["white_balance_temperature_auto"] = 0;
	camMaxs["white_balance_temperature_auto"] = 1;
	camMins["gamma"] = 72;
	camMaxs["gamma"] = 500;
	camMins["gain"] = 0;
	camMaxs["gain"] = 100;
	camMins["power_line_frequency"] = 0;
	camMaxs["power_line_frequency"] = 2;
	camMins["white_balance_temperature"] = 2800;
	camMaxs["white_balance_temperature"] = 6500;
	camMins["sharpness"] = 0;
	camMaxs["sharpness"] = 6;
	camMins["backlight_compensation"] = 0;
	camMaxs["backlight_compensation"] = 2;
	camMins["exposure_auto"] = 0;
	camMaxs["exposure_auto"] = 3;
	camMins["exposure_absolute"] = 1;
	camMaxs["exposure_absolute"] = 5000;
	camMins["exposure_auto_priority"] = 0;
	camMaxs["exposure_auto_priority"] = 1;

	// call base clase setup for now
	IGuiBase::setup();

	// send commands to cam on nix
	for (int i = 0; i < camSettingNames.size(); i++) {
		string setting = camSettingNames[i];
		ostringstream os;
		os << "v4l2-ctl --set-ctrl " << setting << "=" << camSettings[setting];
		ofLogNotice() << "Command system: " << os.str();
		system(os.str().c_str());
	}

	//system("v4l2-ctl --set-ctrl contrast=56");

	bLEDBlinkOn = false;
	lastLedBlinkTime = ofGetElapsedTimeMillis();

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

	lastFlickrAuthenticateTime = ofGetElapsedTimeMillis() - flickrAuthenticateTimeout;

	flickr = new ofxFlickr::API;

	ofAddListener(ofxFlickr::APIEvent::events, this, &ImageCaptureController::onFlickrEvent);

	flickr->start();

	startThread();
}

//--------------------------------------------------------------
void ImageCaptureController::setDefaults() {

	ofLogNotice() << className << ": setDefaults";

#ifdef TARGET_WIN32
	imageStorePath = "C:/Users/gameover8/Desktop/img/capture";
#else
	imageStorePath = "/home/pi/Desktop/img/capture";
#endif

	brightness = 0.15;
	contrast = 1.0;
	saturation = 1.0;

	simulateTimeout = 2000;

	flickrAuthenticateTimeout = 10000;

	ledBlinkSpeed = 70;

	camSettings = camDefault;

}

//--------------------------------------------------------------
void ImageCaptureController::update() {

	if (!bUse) return;

	if (bSetImageStorePath) {
#ifndef TARGET_WIN32
		ofSetWindowShape(10, 10);
#endif
		ofFileDialogResult result = ofSystemLoadDialog("Select Capture Folder", true);
		if (result.bSuccess) {
			lock();
			imageStorePath = result.getPath();
			unlock();
		}
#ifndef TARGET_WIN32
		ofSetWindowShape(1920, 1080);
#endif
	}

	// check thread safe GUI changes
	lock();

	// check what mode we're in
	if (nextSensorMode != currentSensorMode) {
		changeMode();
	}

	unlock();

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
				//shader.setUniform1f("brightness", brightness);
				//shader.setUniform1f("contrast", contrast);
				//shader.setUniform1f("saturation", saturation);

				mesh.draw();

			}
			shader.end();
			//cam.draw(0, 0); // thanks openGL we need to fix the shader
		}
		fbo.end();
	}


	lock();

	// grab the current photo state
	int thisCurrentPhotoState = currentPhotoState;

	unlock();

	if (thisCurrentPhotoState == PHOTO_TAKEIMAGE) {
		bLEDBlinkOn = true;
		takeImage();
	}

}

//--------------------------------------------------------------
void ImageCaptureController::changeMode() {

	// shutdown the current mode

	ofLogNotice() << "Shutting down: " << sensorModes[currentSensorMode];

	switch (currentSensorMode) {
	case SENSOR_TEENSY:
	{

	}
	break;
	case SENSOR_GPIO:
	{
#ifndef TARGET_WIN32
		gpio22.setval_gpio("0");
		gpio27.unexport_gpio();
#endif
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
#ifndef TARGET_WIN32
		gpio27.setup("27"); // button input
		gpio27.export_gpio();
		gpio27.setdir_gpio("in");
		gpio22.setup("22"); // led output
		gpio22.export_gpio();
		gpio22.setdir_gpio("out");
		lastGPIOMsg = "0";
		gpio22.setval_gpio("1");
#endif
	}
	break;
	}

	// reset timeouts
	lastSensorTimeout = ofGetElapsedTimeMillis();

	// set current to next mode
	currentSensorMode = nextSensorMode;

}

//--------------------------------------------------------------
void ImageCaptureController::threadedFunction() {

	while (isThreadRunning()) {

		if (bUse) {

			// check if we are authenticated app with flickr
			if (!flickr->getIsAuthenticated()) {

				// try to authorize everz XX millis if we're not authenticated (assume our credentials are correct)
				if (ofGetElapsedTimeMillis() - lastFlickrAuthenticateTime >= flickrAuthenticateTimeout) {
					ofLogNotice() << "Authenticating Flickr application";
					if (flickr->authenticate(API_KEY, API_SECRET, ofxFlickr::FLICKR_WRITE, true)) {
						ofLogNotice() << "Requeue any un-uploaded files";
						lock();
						Serializer.loadClass(ofToDataPath("configs/UploadQueue" + string(CONFIG_TYPE)), (uploadQueue), ARCHIVE_BINARY);
						for (int i = 0; i < uploadQueue.size(); i++) {
							ofLogNotice() << "Requeue: " << uploadQueue[i];
							flickr->uploadThreaded(uploadQueue[i]);
						}
						unlock();
					}
					lastFlickrAuthenticateTime = ofGetElapsedTimeMillis();
				}
				//return;

			}

			lock();

			// grab the current mode
			int thisCurrentMode = currentSensorMode;



			// calculate the amount of time since the last sensor measurement
			timeSinceLastSensor = ofGetElapsedTimeMillis() - lastSensorTimeout;

			unlock();


			switch (thisCurrentMode) {
			case SENSOR_SIMULATE:
			{
				if (timeSinceLastSensor >= simulateTimeout) {
					triggerSensor(SENSOR_SIMULATE);
					lastSensorTimeout = ofGetElapsedTimeMillis();
				}
			}
			break;
			case SENSOR_TEENSY:
			{

			}
			break;
			case SENSOR_GPIO:
			{
#ifndef TARGET_WIN32
				// read gpio value
				gpio27.getval_gpio(gio27_state);

				// if some gpio value, then triggerSensor(SENSOR_GPIO)
				if (gio27_state == "1" && lastGPIOMsg == "0") {
					triggerSensor(SENSOR_GPIO);
				}
				lastGPIOMsg = gio27_state;
#endif
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
				bLEDBlinkOn = false;
			}
			break;
			case PHOTO_REQUESTED:
			{
				playCountdownSound();
				bLEDBlinkOn = true;
				lastLedBlinkTime = ofGetElapsedTimeMillis();
			}
			break;
			case PHOTO_COUNTDOWN:
			{
				if (!soundPlayerCountdown.isPlaying()) {
					lock();
					if (currentPhotoState == PHOTO_COUNTDOWN) {
						currentPhotoState = PHOTO_TAKEIMAGE;
					}
					unlock();
				}
				else {
					//cout << soundPlayerCountdown.getPosition() << endl;
					if (ofGetElapsedTimeMillis() - lastLedBlinkTime > ledBlinkSpeed / soundPlayerCountdown.getPosition()) {
						lastLedBlinkTime = ofGetElapsedTimeMillis();
						bLEDBlinkOn = !bLEDBlinkOn;
					}
				}
			}
			break;
			case PHOTO_TAKEIMAGE:
			{
				// happens in update as fbo.readPixels can't happen in a thread
			}
			break;
			case PHOTO_SAVEIMAGE:
			{
				saveImage();
			}
			break;
			case PHOTO_FINISHING:
			{
				bLEDBlinkOn = true;
				playShutterSound();
			}
			break;
			case PHOTO_FINISHED:
			{
				if (!soundPlayerShutter.isPlaying()) {
					lock();
					if (currentPhotoState == PHOTO_FINISHED) {
						currentPhotoState = PHOTO_NONE;
						bLEDBlinkOn = false;
					}
					unlock();
				}
				
			}
			break;
			}

#ifndef TARGET_WIN32
				gpio22.setval_gpio(bLEDBlinkOn ? "0" : "1"); // blink that LED
#endif

		}

		ofSleepMillis(1);

	}

}

//--------------------------------------------------------------
void ImageCaptureController::triggerSensor(SensorMode sensorMode) {
	lock();

	if (sensorMode == currentSensorMode) {
		if (currentPhotoState == PHOTO_NONE) {
			currentPhotoState = PHOTO_REQUESTED;
		}
	}

	unlock();

}

//--------------------------------------------------------------
void ImageCaptureController::playCountdownSound() {
	soundPlayerCountdown.play();
	lock();
	if (currentPhotoState == PHOTO_REQUESTED) {
		currentPhotoState = PHOTO_COUNTDOWN;
	}
	unlock();
}

//--------------------------------------------------------------
void ImageCaptureController::playShutterSound() {
	soundPlayerShutter.play();
	lock();
	if (currentPhotoState == PHOTO_FINISHING) {
		currentPhotoState = PHOTO_FINISHED;
	}
	unlock();
}

//--------------------------------------------------------------
void ImageCaptureController::takeImage() {
	//clear pixels
	pixels.clear();
	
	//get img name from timestamp - care: do we need to lock on this?
	// putting it here so later we can use it to timestamp in the image during testing
	currentImageFilePath = imageStorePath + "/" + ofGetTimestampString() + ".jpg";

	//get frame buffer pixels
	fbo.readToPixels(pixels);
	lock();
	
	if (currentPhotoState == PHOTO_TAKEIMAGE) {
		currentPhotoState = PHOTO_SAVEIMAGE;
	}
	unlock();
}

//--------------------------------------------------------------
void ImageCaptureController::saveImage() {
	bool ok = ofSaveImage(pixels, currentImageFilePath, OF_IMAGE_QUALITY_BEST);
	
	lock();
	if (ok) {
		ofLogNotice() << "Image: " << currentImageFilePath << " save SUCCESS";
		uploadQueue.push_back(currentImageFilePath);
		Serializer.saveClass(ofToDataPath("configs/UploadQueue" + string(CONFIG_TYPE)), (uploadQueue), ARCHIVE_BINARY);
		flickr->uploadThreaded(currentImageFilePath);
	}
	else {
		ofLogError() << "Image: " << currentImageFilePath << " save FAILED";
	}
	if (currentPhotoState == PHOTO_SAVEIMAGE) {
		currentPhotoState = PHOTO_FINISHING;
	}
	unlock();
}

//--------------------------------------------------------------
void ImageCaptureController::onFlickrEvent(ofxFlickr::APIEvent & evt) {

	ostringstream os;
	os << flickr->getCallTypeAsString(evt.callType)
		<< " event: " << string(evt.success ? "OK" : "ERROR")
		<< " with " << evt.results.size() << " and "
		<< evt.resultString;

	switch (evt.callType) {
	case ofxFlickr::FLICKR_UPLOAD:
	{
		ofLogNotice() << os.str() << endl;
		if (evt.success) {
			lock();
			int uploadQueueIndex = -1;
			for (int i = 0; i < uploadQueue.size(); i++) {
				if (uploadQueue[i] == evt.resultString) {
					uploadQueueIndex = i;
					break;
				}
			}

			if (uploadQueueIndex == -1) {
				ofLogError() << "We have a problem Huston! The uploaded file wasn't in the uploadQueue...oh oh!";
			}
			else {
				ofLogNotice() << "Removing from uploadQueue: " << uploadQueue[uploadQueueIndex];
				uploadQueue.erase(uploadQueue.begin() + uploadQueueIndex);
				Serializer.saveClass(ofToDataPath("configs/UploadQueue" + string(CONFIG_TYPE)), (uploadQueue), ARCHIVE_BINARY);
			}

			unlock();
		}
		else {
			ofLogWarning() << "Upload failed! Re-upload: " << evt.resultString;
			flickr->uploadThreaded(evt.resultString);
		}
	}
	break;
	case ofxFlickr::FLICKR_SEARCH:
	{
		// ignore
	}
	break;
	}
}

//--------------------------------------------------------------
void ImageCaptureController::drawGUI() {

	if (ImGui::CollapsingHeader(className.c_str())) {
		beginGUI();
		{
			bSetImageStorePath = ImGui::Button("Set Image Save Path");

			ImGui::SliderInt("Flickr Authorize Timeout (millis)", &flickrAuthenticateTimeout, 1000, 20000);
			ImGui::SliderInt("LED Blink Speed (millis)", &ledBlinkSpeed, 10, 1000);

			ImGui::Combo("Sensor Mode", (int*)&nextSensorMode, sensorModes);

			switch (nextSensorMode) {
			case SENSOR_SIMULATE:
			{
				ImGui::SliderInt("Simulate Timeout", &simulateTimeout, 1, 600000);
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

			if (ImGui::CollapsingHeader("Camera Settings"))
			{
				
				map<string, int> camTemp = camSettings;
				vector<string> commands;
				for (int i = 0; i < camSettingNames.size(); i++) {
					string setting = camSettingNames[i];
					if (ImGui::Button("reset")) camTemp[setting] = camDefault[setting]; ImGui::SameLine();
					ImGui::SliderInt(setting.c_str(), &camTemp[setting], camMins[setting], camMaxs[setting]); 
					ImGui::Spacing();

					if (camTemp[setting] != camSettings[setting]) {
						camSettings[setting] = camTemp[setting];
						ostringstream os;
						os << "v4l2-ctl --set-ctrl " << setting << "=" << camSettings[setting];
						commands.push_back(os.str());
					}
				}

				for (int i = 0; i < commands.size(); i++) {
					ofLogNotice() << "Command system: " << commands[i];
					system(commands[i].c_str());
				}

			}

			lock();

			ImGui::NewLine();
			ImGui::Text("Time since last sensor reading : %.0f millis", timeSinceLastSensor);
			ImGui::NewLine();

			ImGui::Checkbox("LED Blink", &bLEDBlinkOn);

			unlock();

		}
		endGUI();
	}
}

//--------------------------------------------------------------
ofTexture & ImageCaptureController::getCameraTexture() {
	return fbo.getTextureReference(0);
}

//--------------------------------------------------------------
bool ImageCaptureController::loadParameters() {
	cout << "load: " << configPath << endl;
	return Serializer.loadClass(ofToDataPath("configs/" + configPath + "/" + className + CONFIG_TYPE), (*this), ARCHIVE_BINARY);
}

//--------------------------------------------------------------
bool ImageCaptureController::saveParameters() {
	cout << "save: " << configPath << endl;
	return Serializer.saveClass(ofToDataPath("configs/" + configPath + "/" + className + CONFIG_TYPE), (*this), ARCHIVE_BINARY);
}
