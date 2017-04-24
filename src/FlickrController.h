#pragma once

#include "ofMain.h"
#include "IGuiBase.h"
#include "ofxFlickr.h"

class FlickrController : public IGuiBase, public ofThread {
public:

	FlickrController();
	~FlickrController();

	typedef enum {
		FLICKR_NONE = 0,
		FLICKR_UPLOAD,
		FLICKR_DOWNLOAD
	} FlickrMode;

	void setup();
	void update();
	void drawGUI();

protected:

	vector<string> flickrModes = {
		"FLICKR_NONE",
		"FLICKR_UPLOAD",
		"FLICKR_DOWNLOAD"
	};

	ofxFlickr::API flickr;

	int flickrAuthenticateTimeout;
	int lastFlickrAuthenticateTime;

	int flickrDownloadTimeout;
	int lastFlickrDownloadTime;

	bool bIsSearching;
	bool bIsDownloading;
	bool bIsDirectoryListed;

	vector<ofxFlickr::Media> downloadQueue;

	string API_KEY = "8eae23d79b1fa5153426d0c5b966ac2b";
	string API_SECRET = "db2d65d186157c1e";

	bool bSetFlickrDownloadPath;
	string flickrDownloadPath;

	FlickrMode nextFlickrMode;
	FlickrMode currentFlickrMode;

	ofDirectory downloadDir;
	int flickrSearchPage;

	void threadedFunction();

	void onFlickrEvent(ofxFlickr::APIEvent & evt);

	void changeMode();

	void setDefaults();
	bool loadParameters();
	bool saveParameters();

	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version) {
		ar & BOOST_SERIALIZATION_NVP(nextFlickrMode);
		ar & BOOST_SERIALIZATION_NVP(flickrDownloadPath);
		ar & BOOST_SERIALIZATION_NVP(flickrAuthenticateTimeout);
		ar & BOOST_SERIALIZATION_NVP(flickrDownloadTimeout);
	}
};
