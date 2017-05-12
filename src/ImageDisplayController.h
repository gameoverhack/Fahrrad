#pragma once

#include "ofMain.h"
#include "IGuiBase.h"
#include "ofxFlickr.h"

class ImageDisplayController : public IGuiBase, public ofThread {
public:

	ImageDisplayController();
	~ImageDisplayController();

	void setup();
	void update();
	void drawGUI();

	const ofTexture& getDisplayFBO();

protected:

	ofFbo fbo;
	vector<ofPixels> imagePixels;
	vector<ofTexture> imageTextures;

	int loadImagesTimeout;
	int lastLoadImagesTimeout;

	bool bRenderImages;

	ofxFlickr::API * flickr;

	int flickrAuthenticateTimeout;
	int lastFlickrAuthenticateTime;

	int flickrSearchTimeout;
	int lastFlickrSearchTime;

	bool bIsSearching;
	bool bIsDownloading;
	bool bIsDirectoryListed;

	vector<ofxFlickr::Media> downloadQueue;

	bool bSetImageDownloadPath;
	string imageDownloadPath;

	ofDirectory imageDownloadDir;
	int flickrSearchPage;

	void threadedFunction();

	void onFlickrEvent(ofxFlickr::APIEvent & evt);

	void setDefaults();
	bool loadParameters();
	bool saveParameters();

	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version) {
		ar & BOOST_SERIALIZATION_NVP(imageDownloadPath);
		ar & BOOST_SERIALIZATION_NVP(flickrAuthenticateTimeout);
		ar & BOOST_SERIALIZATION_NVP(flickrSearchTimeout);
		ar & BOOST_SERIALIZATION_NVP(loadImagesTimeout);
	}
};
