#include "FlickrController.h"

//--------------------------------------------------------------
FlickrController::FlickrController() {

	className = "FlickrController";
	ofLogNotice() << className << ": constructor";

}

//--------------------------------------------------------------
FlickrController::~FlickrController() {

	ofLogNotice() << className << ": destructor";
	waitForThread();
	this->IGuiBase::~IGuiBase(); // call base destructor

}

//--------------------------------------------------------------
void FlickrController::setup() {

	ofLogNotice() << className << ": setup";

	// call base clase setup for now
	IGuiBase::setup();

	currentFlickrMode = FLICKR_NONE;
	bSetFlickrDownloadPath = false;

	bIsSearching = false;
	bIsDownloading = false;
	flickrSearchPage = 0;

	lastFlickrAuthenticateTime = ofGetElapsedTimeMillis() - flickrAuthenticateTimeout;
	lastFlickrDownloadTime = ofGetElapsedTimeMillis() - flickrDownloadTimeout;

	ofAddListener(ofxFlickr::APIEvent::events, this, &FlickrController::onFlickrEvent);

	flickr.start();
	startThread();
}

//--------------------------------------------------------------
void FlickrController::setDefaults() {

	ofLogNotice() << className << ": setDefaults";

#ifdef TARGET_WIN32
	flickrDownloadPath = "C:/Users/gameover8/Desktop/flickr2";
#else
	flickrDownloadPath = "/home/pi/Desktop/flickr";
#endif

	flickrAuthenticateTimeout = 10000;
	flickrDownloadTimeout = 10000;

}

//--------------------------------------------------------------
void FlickrController::update() {

	if (!bUse) return;

	lock();

	// check what mode we're in
	if (nextFlickrMode != currentFlickrMode) {
		changeMode();
	}

	unlock();

	// check and set the folder we want to download to
	if (bSetFlickrDownloadPath) {
#ifndef TARGET_WIN32
		ofSetWindowShape(10, 10);
#endif
		ofFileDialogResult result = ofSystemLoadDialog("Select Download Folder", true);
		if (result.getPath() != "") {
			flickrDownloadPath = result.getPath();
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
		return;
	}
	else {

		switch (currentFlickrMode) {
		case FLICKR_UPLOAD:
		{
			
		}
		break;
		case FLICKR_DOWNLOAD:
		{
			if (ofGetElapsedTimeMillis() - lastFlickrDownloadTime >= flickrDownloadTimeout) {
				if (!bIsSearching && !bIsDownloading) {
					// need to have strategy for getting the latest images from today???!!! etc random how do we store etc???
					flickr.searchThreaded("", "149397704@N05", flickrSearchPage);
					lock();
					bIsSearching = true;
					unlock();
				}else{
					ofLogNotice() << "Trying to search when we already have a search/download in progress";
					// handle this!
				}
				lastFlickrDownloadTime = ofGetElapsedTimeMillis();
			}
		}
		break;
		}

	}



}

//--------------------------------------------------------------
void FlickrController::threadedFunction() {

	while (isThreadRunning()) {

		if (bUse) {

			lock();
			int thisCurrentMode = currentFlickrMode;
			unlock();

			switch (thisCurrentMode) {
			case FLICKR_UPLOAD:
			{

			}
			break;
			case FLICKR_DOWNLOAD:
			{
				lock();
				if (bIsDownloading) {
					if (downloadQueue.size() > 0) {
						
						ofxFlickr::Media media = downloadQueue.front();
						string title = media.title + ".jpg";
						bool bDoDownload = true;
						for (int i = 0; i < downloadDir.size(); i++) {

							if (title == downloadDir.getName(i)) {
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
								image.save(flickrDownloadPath + "/" + title);
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
			break;
			}

		}

		ofSleepMillis(1);

	}

}

//--------------------------------------------------------------
void FlickrController::onFlickrEvent(ofxFlickr::APIEvent & evt) {

	ostringstream os;
	os << flickr.getCallTypeAsString(evt.callType)
		<< " event: " << string(evt.success ? "OK" : "ERROR")
		<< " with " << evt.results.size() << " and "
		<< evt.resultString;

	ofLogNotice() << os.str() << endl;

	switch (evt.callType) {
	case ofxFlickr::FLICKR_UPLOAD:
	{
		if (evt.success) {

		}
		else {
		
		}
	}
	break;
	case ofxFlickr::FLICKR_SEARCH:
	{
		if (evt.success) {
			ofLogNotice() << "Search succeeded";
			lock();
			bIsSearching = false;
			bIsDownloading = true;
			downloadQueue = evt.results;
			listDirectory();
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
void FlickrController::changeMode() {

	// shutdown the current mode

	ofLogNotice() << "Shutting down: " << flickrModes[currentFlickrMode];

	switch (currentFlickrMode) {
	case FLICKR_UPLOAD:
	{
		
	}
	break;
	case FLICKR_DOWNLOAD:
	{
		
	}
	break;
	}

	// setup for the next mode

	ofLogNotice() << "Setting up: " << flickrModes[nextFlickrMode];

	switch (nextFlickrMode) {
	case FLICKR_UPLOAD:
	{
		
	}
	break;
	case FLICKR_DOWNLOAD:
	{
		listDirectory();
	}
	break;
	}

	// set current to next mode
	currentFlickrMode = nextFlickrMode;

}

//--------------------------------------------------------------
void FlickrController::listDirectory() {

	downloadDir.reset();
	downloadDir.allowExt("jpg");
	downloadDir.listDir(flickrDownloadPath);

}

//--------------------------------------------------------------
void FlickrController::drawGUI() {

	if (ImGui::CollapsingHeader(className.c_str())) {
		beginGUI();
		{
			bSetFlickrDownloadPath = ImGui::Button("Set Flickr Download Path");
			
			ImGui::SliderInt("Flickr Authorize Timeout (millis)", &flickrAuthenticateTimeout, 1000, 20000);
			
			ImGui::Combo("Flickr Mode", (int*)&nextFlickrMode, flickrModes);
			
			switch (nextFlickrMode) {
			case FLICKR_UPLOAD:
			{
				//ImGui::SliderInt("Flickr Download Timeout (millis)", &flickrUploadTimeout, 1000, 60000);
			}
			break;
			case FLICKR_DOWNLOAD:
			{
				ImGui::SliderInt("Flickr Download Timeout (millis)", &flickrDownloadTimeout, 1000, 60000);
			}
			break;
			}

		}
		endGUI();
	}
}

//--------------------------------------------------------------
bool FlickrController::loadParameters() {
	return Serializer.loadClass(ofToDataPath("configs/" + className + CONFIG_TYPE), (*this), ARCHIVE_BINARY);
}

//--------------------------------------------------------------
bool FlickrController::saveParameters() {
	return Serializer.saveClass(ofToDataPath("configs/" + className + CONFIG_TYPE), (*this), ARCHIVE_BINARY);
}
