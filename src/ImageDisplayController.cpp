#include "ImageDisplayController.h"

//--------------------------------------------------------------
ImageDisplayController::ImageDisplayController() {

	className = "ImageDisplayController";
	ofLogNotice() << className << ": constructor";

}

//--------------------------------------------------------------
ImageDisplayController::~ImageDisplayController() {

	ofLogNotice() << className << ": destructor";
	waitForThread();
	this->IGuiBase::~IGuiBase(); // call base destructor

}

//--------------------------------------------------------------
void ImageDisplayController::setup() {

	ofLogNotice() << className << ": setup";

	// call base clase setup for now
	IGuiBase::setup();

	bSetImageDownloadPath = false;

	bIsSearching = false;
	bIsDownloading = false;
	flickrSearchPage = 0;

	lastFlickrAuthenticateTime = ofGetElapsedTimeMillis() - flickrAuthenticateTimeout;
	lastFlickrSearchTime = ofGetElapsedTimeMillis() - flickrSearchTimeout;

	ofAddListener(ofxFlickr::APIEvent::events, this, &ImageDisplayController::onFlickrEvent);

	flickr.start();
	startThread();
}

//--------------------------------------------------------------
void ImageDisplayController::setDefaults() {

	ofLogNotice() << className << ": setDefaults";

#ifdef TARGET_WIN32
	imageDownloadPath = "C:/Users/gameover8/Desktop/flickr2";
#else
	imageDownloadPath = "/home/pi/Desktop/flickr";
#endif

	flickrAuthenticateTimeout = 10000;
	flickrSearchTimeout = 10000;

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
		if (result.getPath() != "") {
			imageDownloadPath = result.getPath();
		}
#ifndef TARGET_WIN32
		ofSetWindowShape(1920, 1080);
#endif
	}

	// check if we are authenticated app with flickr
	if (!flickr.getIsAuthenticated()) {

		// try to authorize everz XX millis if we're not authenticated (assume our credentials are correct)
		if (ofGetElapsedTimeMillis() - lastFlickrAuthenticateTime >= flickrAuthenticateTimeout) {
			ofLogNotice() << "Authenticating Flickr application";
			flickr.authenticate(API_KEY, API_SECRET, ofxFlickr::FLICKR_WRITE, true);
			lastFlickrAuthenticateTime = ofGetElapsedTimeMillis();
		}
		//return;

	}
	else {

		if (ofGetElapsedTimeMillis() - lastFlickrSearchTime >= flickrSearchTimeout) {
			if (!bIsSearching && !bIsDownloading) {
				// need to have strategy for getting the latest images from today???!!! etc random how do we store etc???
				flickr.searchThreaded("", "149397704@N05", flickrSearchPage);
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

}

//--------------------------------------------------------------
void ImageDisplayController::threadedFunction() {

	while (isThreadRunning()) {

		if (bUse) {

			lock();
			if (bIsDownloading) {
				if (downloadQueue.size() > 0) {

					if (!bIsDirectoryListed) {
						unlock();
						ofDirectory dir;
						dir.allowExt("jpg");
						dir.listDir(imageDownloadPath);
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
	os << flickr.getCallTypeAsString(evt.callType)
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

		}
		endGUI();
	}
}

//--------------------------------------------------------------
bool ImageDisplayController::loadParameters() {
	return Serializer.loadClass(ofToDataPath("configs/" + className + CONFIG_TYPE), (*this), ARCHIVE_BINARY);
}

//--------------------------------------------------------------
bool ImageDisplayController::saveParameters() {
	return Serializer.saveClass(ofToDataPath("configs/" + className + CONFIG_TYPE), (*this), ARCHIVE_BINARY);
}
