#include "ImageLoadController.h"

//--------------------------------------------------------------
ImageLoadController::ImageLoadController() {
	className = "ImageLoadController";
	ofLogNotice() << className << ": constructor";
}

//--------------------------------------------------------------
ImageLoadController::~ImageLoadController() {
	ofLogNotice() << className << ": destructor";

	// kill the thread
	waitForThread();

	this->IGuiBase::~IGuiBase(); // call base destructor
}

//--------------------------------------------------------------
void ImageLoadController::setup() {
	ofLogNotice() << className << ": setup";

	// call base clase setup for now
	IGuiBase::setup();

	bSetImageLoadPath = false;

	startThread();

	//setup timer
	startTime = ofGetElapsedTimeMillis();

	//setup image loading
	bSetupImages = true;
	bLoadImages = false;
	bImagesLoaded = false;
	bImagesReady = false;
	tmpImage = ofToDataPath("images/tmpImage.jpg");

	currentFboIndex = 0;

	images.resize(6);

	fbos.resize(2);
	fbos[0].allocate(ofGetWidth(), ofGetHeight(), GL_RGB);
	fbos[1].allocate(ofGetWidth(), ofGetHeight(), GL_RGB);

	loadImages();
}

//--------------------------------------------------------------
void ImageLoadController::setDefaults() {
	ofLogNotice() << className << ": setDefaults";

#ifdef TARGET_WIN32
	imgLoadPath = "C:/Users/JB/Desktop/img";
#else
	imgLoadPath = "C:/Users/JB/Desktop/img";
#endif

	transitionTime = 0.5;

}

//--------------------------------------------------------------
void ImageLoadController::update() {

	if (!bUse) return;

	if (bSetImageLoadPath) {
#ifndef TARGET_WIN32
		ofSetWindowShape(10, 10);
#endif
		ofFileDialogResult result = ofSystemLoadDialog("Select Load Folder", true);
		if (result.getPath() != "") {
			imgLoadPath = result.getPath();
		}
#ifndef TARGET_WIN32
		ofSetWindowShape(ofGetWidth(), ofGetHeight());
#endif
	}

	//Timer for image change
	float timer = ofGetElapsedTimeMillis() - startTime;

	if (timer >= transitionTime * 60000) {
		bLoadImages = true;
		startTime = ofGetElapsedTimeMillis();
	}

	if (bImagesReady) {

		for (int i = 0; i < images.size(); i++) {
			images[i].update();
		}

		int nextFboIndex = currentFboIndex + 1;
		if (nextFboIndex > 1) nextFboIndex = 0;

		fbos[nextFboIndex].begin();
		{
			int xCounter = 0, yCounter = 0;
			for (int i = 0; i < images.size(); i++) {
				if (xCounter == images.size() / 2) xCounter = 0;
				if (i >= images.size() / 2) yCounter = 1;

				images[i].draw((ofGetWidth() / 3)*xCounter, (ofGetHeight() / 2) * yCounter, ofGetWidth() / 3, ofGetHeight() / 2);
				xCounter++;
			}
		}
		fbos[nextFboIndex].end();

		currentFboIndex = nextFboIndex;

		bImagesReady = false;
	}
}

//--------------------------------------------------------------
void ImageLoadController::draw() {
	fbos[currentFboIndex].draw(0, 0, ofGetWidth(), ofGetHeight());
}

//--------------------------------------------------------------
void ImageLoadController::threadedFunction() {

	while (isThreadRunning()) {
		if (bUse) {
			if (bLoadImages) {
				lock();
				bLoadImages = false;
				bImagesReady = false;
				unlock();

				loadImages();
			}

			lock();
			if (bImagesLoaded && !bImagesReady) {
				bImagesReady = true;
				bImagesLoaded = false;
			}
			unlock();
		}
	}
}

//--------------------------------------------------------------
void ImageLoadController::loadImages() {
	//List Images in Dir
	dir.allowExt("jpg");
	dir.listDir(imgLoadPath);

	imgFilePaths.clear();
	if (dir.size() == 0) {
		imgFilePaths.push_back(tmpImage);
	}
	else {
		for (int i = 0; i < dir.size(); i++) {
			imgFilePaths.push_back(dir.getPath(i));
		}
	}

	//Load IMAGES
	//setup for first view
	if(bSetupImages){
		int imageCounter = 0;

		for (int i = 0; i < images.size(); i++) {
			int randomImage = int(ofRandom(0, imgFilePaths.size()));

			ofLogVerbose() << "Loading image: " << imgFilePaths[randomImage];
			if (images[i].load(imgFilePaths[randomImage])) imageCounter++;
		}

		if (imageCounter == images.size()) {
			bImagesLoaded = true;
			bSetupImages = false;
		}
	}
	//get random image @ random position
	else {
		int randomImage = int(ofRandom(0, imgFilePaths.size()));
		int randomPos = int(ofRandom(0, images.size()));

		ofLogVerbose() << "Loading image: " << imgFilePaths[randomImage];
		if (images[randomPos].load(imgFilePaths[randomImage])) bImagesLoaded = true;
	}
}

//--------------------------------------------------------------
void ImageLoadController::drawGUI() {

	if (ImGui::CollapsingHeader(className.c_str())) {
		beginGUI();
		{
			bSetImageLoadPath = ImGui::Button("Set Image Load Path");
			ImGui::Spacing();
			ImGui::SliderFloat("Image transition time", &transitionTime, 0.1, 10.0);
			ImGui::Spacing();
		}
		endGUI();
	}
}

//--------------------------------------------------------------
bool ImageLoadController::loadParameters() {
	return Serializer.loadClass(fixPath("configs/" + className + CONFIG_TYPE), (*this), ARCHIVE_BINARY);
}

//--------------------------------------------------------------
bool ImageLoadController::saveParameters() {
	return Serializer.saveClass(fixPath("configs/" + className + CONFIG_TYPE), (*this), ARCHIVE_BINARY);
}