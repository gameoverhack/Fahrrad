#pragma once

#include "ofMain.h"
#include "IGuiBase.h"
#include "ofxNetwork.h"

class NetworkController : public IGuiBase, public ofThread {
public:

	NetworkController();
	~NetworkController();

	typedef enum {
		NETWORK_NONE = 0,
		NETWORK_SEND,
		NETWORK_RECV
	} NetworkMode;

	void setup();
	void setMode(NetworkMode mode);
	void update();
	void drawGUI();

protected:

	vector<string> networkModes = {
		"NETWORK_NONE",
		"NETWORK_SEND",
		"NETWORK_RECV"
	};

	NetworkMode nextNetworkMode;
	NetworkMode currentNetworkMode;

	double lastNetworkTimeout;
	int networkTimeout;

	void changeMode();

	void threadedFunction();

	void setDefaults();
	bool loadParameters();
	bool saveParameters();

	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version) {
		ar & BOOST_SERIALIZATION_NVP(nextNetworkMode);
		ar & BOOST_SERIALIZATION_NVP(networkTimeout);
	}
};
