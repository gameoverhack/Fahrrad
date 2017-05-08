#include "ImageCaptureController.h"

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


	lock();

	// grab the current photo state
	int thisCurrentPhotoState = currentPhotoState;

	unlock();

	if (thisCurrentPhotoState == PHOTO_TAKEIMAGE) {
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
						currentPhotoState = PHOTO_TAKEIMAGE;
					}
					unlock();
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

			if (ImGui::CollapsingHeader("Shader Settings"))
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

			lock();

			ImGui::NewLine();
			ImGui::Text("Time since last sensor reading : %.0f millis", timeSinceLastSensor);

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
	return Serializer.loadClass(ofToDataPath("configs/" + className + CONFIG_TYPE), (*this), ARCHIVE_BINARY);
}

//--------------------------------------------------------------
bool ImageCaptureController::saveParameters() {
	return Serializer.saveClass(ofToDataPath("configs/" + className + CONFIG_TYPE), (*this), ARCHIVE_BINARY);
}
