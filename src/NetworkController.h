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

	void setup(string path) {
		configPath = path;
		setup();
	}

	void setup();
	void setMode(NetworkMode mode);
	void update();
	void drawGUI();

	void setRiderSummary(const RiderSummaryUnion& rsu);
	const RiderSummaryUnion& getRiderSummary();

protected:

	bool connectSender();
	bool connectReceiver();

	int ipAddress[4];
	int ipPort;
	bool bIsConnected;
	ofxUDPManager udp;

	bool bNetworkNeedsUpdate;
	RiderSummaryUnion riderSummary;

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
		ar & BOOST_SERIALIZATION_NVP(networkTimeout);
		ar & BOOST_SERIALIZATION_NVP(ipAddress);
		ar & BOOST_SERIALIZATION_NVP(ipPort);
	}
};
