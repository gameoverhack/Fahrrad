#include "ImageDisplayController.h"

//--------------------------------------------------------------
ImageDisplayController::ImageDisplayController() {

	className = "ImageDisplayController";
	ofLogNotice() << className << ": constructor";

}

//--------------------------------------------------------------
ImageDisplayController::~ImageDisplayController() {

	ofLogNotice() << className << ": destructor";

	lock();
	ofRemoveListener(ofxFlickr::APIEvent::events, this, &ImageDisplayController::onFlickrEvent);
	flickr->stop();
	unlock();

	// kill the thread
	waitForThread();
	delete flickr;

	this->IGuiBase::~IGuiBase(); // call base destructor

}

//--------------------------------------------------------------
void ImageDisplayController::setup() {

	ofLogNotice() << className << ": setup";

	// call base clase setup for now
	IGuiBase::setup();

	bSetImageDownloadPath = false;

	fbo.allocate(ofGetWidth(), ofGetHeight(), GL_RGB);

	imagePixels.resize(6);
	imageTextures.resize(6);

	lastLoadImagesTimeout = ofGetElapsedTimeMillis() - loadImagesTimeout;

	bRenderImages = false;

	bIsSearching = false;
	bIsDownloading = false;
	flickrSearchPage = 0;

	lastFlickrAuthenticateTime = ofGetElapsedTimeMillis() - flickrAuthenticateTimeout;
	lastFlickrSearchTime = ofGetElapsedTimeMillis() - flickrSearchTimeout;

	flickr = new ofxFlickr::API;

	ofAddListener(ofxFlickr::APIEvent::events, this, &ImageDisplayController::onFlickrEvent);

	flickr->start();
	startThread();
}

//--------------------------------------------------------------
void ImageDisplayController::setDefaults() {

	ofLogNotice() << className << ": setDefaults";

#ifdef TARGET_WIN32
	imageDownloadPath = "C:/Users/gameover8/Desktop/img/download";
#else
	imageDownloadPath = "/home/pi/Desktop/img/download";
#endif

	flickrAuthenticateTimeout = 10000;
	flickrSearchTimeout = 10000;

	loadImagesTimeout = 10000;
}

//--------------------------------------------------------------
void ImageDisplayController::update() {

	if (!bUse) return;

	// check and set the folder we want to download to
	if (bSetImageDownloadPath) {
#ifndef TARGET_WIN32
		ofSetWindowShape(10, 10);
#endif
		ofFileDialogResult result = ofSystemLoadDialog("Select Download Folder", true);
		if (result.bSuccess) {
			lock();
			imageDownloadPath = result.getPath();
			unlock();
		}
#ifndef TARGET_WIN32
		ofSetWindowShape(1920, 1080);
#endif
	}

	lock();
	if (bRenderImages) {
		fbo.begin();
		{
			ofClear(0);
			//ofDrawRectangle(400, 400, 400, 400);
			int xCounter = 0, yCounter = 0;
			for (int i = 0; i < imageTextures.size(); i++) {
				if (imageTextures[i].getWidth() != imagePixels[i].getWidth() ||
					imageTextures[i].getHeight() != imagePixels[i].getHeight()) {
					imageTextures[i].allocate(imagePixels[i]);
				}
				imageTextures[i].loadData(imagePixels[i]);

				if (xCounter == imageTextures.size() / 2) xCounter = 0;
				if (i >= imageTextures.size() / 2) yCounter = 1;

				imageTextures[i].draw((ofGetWidth() / 3)*xCounter, (ofGetHeight() / 2) * yCounter, ofGetWidth() / 3, ofGetHeight() / 2);
				xCounter++;

			}
		}
		fbo.end();
		bRenderImages = false;
	}
	unlock();

}

//--------------------------------------------------------------
void ImageDisplayController::threadedFunction() {

	while (isThreadRunning()) {

		if (bUse) {

			lock();
			bool bCanLoadNewImages = !bRenderImages; // we shouldn't load images if we're rendering them
			unlock();

			if (ofGetElapsedTimeMillis() - lastLoadImagesTimeout >= loadImagesTimeout && bCanLoadNewImages) {

				ofDirectory dir;
				dir.allowExt("jpg");
				dir.listDir(imageDownloadPath);
				
				vector<int> randomDirectoryIndexes;
				vector<string> availablePaths;

				if (dir.size() > 0) {

					// check if we have deleted anything on Flickr
					for (int j = 0; j < dir.size(); j++) {
						bool bBlackListed = true;
						for (int i = 0; i < allFlickrMedia.size(); i++) {
							if (allFlickrMedia[i] == dir.getName(j)) {
								bBlackListed = false;
							}
						}
						if (!bBlackListed) availablePaths.push_back(dir.getPath(j));
					}

					if(availablePaths.size() > 0) uniqueRandomIndex(randomDirectoryIndexes, 0, availablePaths.size(), MIN(availablePaths.size(), imagePixels.size())); // see Utils.h
					
				}

				for (int i = 0; i < imagePixels.size(); i++) {
					string filePath = "";
					if (i < availablePaths.size()) {
						filePath = availablePaths[randomDirectoryIndexes[i]];
					}else{
						filePath = ofToDataPath("images/tmpImage.jpg");
					}
					bool ok = ofLoadImage(imagePixels[i], filePath);
					if (ok) {
						ofLogNotice() << "Loading random image: " << filePath;
						float aspect = (ofGetWidth() / 3.0) / (ofGetHeight() / 2.0);
						float tWidth = imagePixels[i].getHeight() * aspect;
						float tOffset = (imagePixels[i].getWidth() - tWidth) / 2.0f;
						cout << imagePixels[i].getWidth() << " x " << imagePixels[i].getHeight() << endl;
						imagePixels[i].crop(tOffset, 0, tWidth, imagePixels[i].getHeight());
						cout << imagePixels[i].getWidth() << " x " << imagePixels[i].getHeight() << endl;
					}else{
						ofLogError() << "Error loading random image: " << filePath;
					}
				}

				lock();
				bRenderImages = true;
				unlock();

				lastLoadImagesTimeout = ofGetElapsedTimeMillis();

			}
			
			// check if we are authenticated app with flickr
			if (!flickr->getIsAuthenticated()) {

				// try to authorize everz XX millis if we're not authenticated (assume our credentials are correct)
				if (ofGetElapsedTimeMillis() - lastFlickrAuthenticateTime >= flickrAuthenticateTimeout) {
					ofLogNotice() << "Authenticating Flickr application";
					flickr->authenticate(API_KEY, API_SECRET, ofxFlickr::FLICKR_WRITE, true);
					lastFlickrAuthenticateTime = ofGetElapsedTimeMillis();
				}
				//return;

			}
			else {

				if (ofGetElapsedTimeMillis() - lastFlickrSearchTime >= flickrSearchTimeout) {
					if (!bIsSearching && !bIsDownloading) {
						// need to have strategy for getting the latest images from today???!!! etc random how do we store etc???
						flickr->searchThreaded("", "149397704@N05", flickrSearchPage);
						lock();
						bIsSearching = true;
						unlock();
					}
					else {
						ofLogNotice() << "Trying to search when we already have a search/download in progress";
						// handle this!
					}
					lastFlickrSearchTime = ofGetElapsedTimeMillis();
				}

			}

			lock();
			if (bIsDownloading) {
				if (downloadQueue.size() > 0) {

					if (!bIsDirectoryListed) {
						unlock();
						ofDirectory dir;
						dir.allowExt("jpg");
						dir.listDir(imageDownloadPath);

						bool bHaveListed = false;
						for (int j = 0; j < downloadQueue.size(); j++) {
							string title = downloadQueue[j].title + ".jpg";
							for (int i = 0; i < allFlickrMedia.size(); i++) {
								if (allFlickrMedia[i] == title) {
									bHaveListed = true;
									break;
								}
							}
							if (!bHaveListed) allFlickrMedia.push_back(title);
						}
						

						lock();
						imageDownloadDir = dir;
						bIsDirectoryListed = true;
					}

					ofxFlickr::Media media = downloadQueue.front();
					string title = media.title + ".jpg";

					bool bDoDownload = true;
					for (int i = 0; i < imageDownloadDir.size(); i++) {

						if (title == imageDownloadDir.getName(i)) {
							ofLogVerbose() << "Not downloading...already have: " + title;
							bDoDownload = false;
							break;
						}

					}

					unlock();

					if (bDoDownload) {

						ofImage image;
						ofLogNotice() << "Loading image url: " << media.getURL();

						bool bGotImage = image.load(media.getURL());

						if (bGotImage) {
							ofLogNotice() << "Saving image locally: " << title;
							image.save(imageDownloadPath + "/" + title);
						}

					}


					lock();

					downloadQueue.erase(downloadQueue.begin());

				}
				else {
					bIsDownloading = false;
				}

			}
			unlock();

		}

		ofSleepMillis(1);

	}

}

//--------------------------------------------------------------
void ImageDisplayController::onFlickrEvent(ofxFlickr::APIEvent & evt) {

	ostringstream os;
	os << flickr->getCallTypeAsString(evt.callType)
		<< " event: " << string(evt.success ? "OK" : "ERROR")
		<< " with " << evt.results.size() << " and "
		<< evt.resultString;

	switch (evt.callType) {
	case ofxFlickr::FLICKR_UPLOAD:
	{
		// ignore
	}
	break;
	case ofxFlickr::FLICKR_SEARCH:
	{
		ofLogNotice() << os.str() << endl;
		if (evt.success) {
			ofLogNotice() << "Search succeeded";
			lock();
			bIsSearching = false;
			bIsDownloading = true;
			bIsDirectoryListed = false;
			downloadQueue = evt.results;
			if (evt.results.size() == 100) {
				flickrSearchPage++;
			}else{
				flickrSearchPage = 0;
				allFlickrMedia.clear();
			}
			unlock();
		}
		else {
			ofLogError() << "Search failed";
			lock();
			bIsSearching = false;
			unlock();
		}
		
	}
	break;
	}
}

//--------------------------------------------------------------
void ImageDisplayController::drawGUI() {

	if (ImGui::CollapsingHeader(className.c_str())) {
		beginGUI();
		{
			bSetImageDownloadPath = ImGui::Button("Set Image Download Path");
			
			ImGui::SliderInt("Flickr Authorize Timeout (millis)", &flickrAuthenticateTimeout, 1000, 20000);
			ImGui::SliderInt("Flickr Download Timeout (millis)", &flickrSearchTimeout, 1000, 60000);
			ImGui::SliderInt("Load Image Timeout (millis)", &loadImagesTimeout, 1000, 240000);
			
		}
		endGUI();
	}
}

//--------------------------------------------------------------
const ofTexture & ImageDisplayController::getDisplayFBO(){
	ofScopedLock lock(mutex);
	return fbo.getTexture();
}

//--------------------------------------------------------------
bool ImageDisplayController::loadParameters() {
	cout << "load: " << configPath << endl;
	return Serializer.loadClass(ofToDataPath("configs/" + configPath + "/" + className + CONFIG_TYPE), (*this), ARCHIVE_BINARY);
}

//--------------------------------------------------------------
bool ImageDisplayController::saveParameters() {
	cout << "save: " << configPath << endl;
	return Serializer.saveClass(ofToDataPath("configs/" + configPath + "/" + className + CONFIG_TYPE), (*this), ARCHIVE_BINARY);
}
