#pragma once

#include "IGuiBase.h"

class BicycleController : public IGuiBase {

public:

	BicycleController();
	~BicycleController();

	void setup();
	void update();
	void drawGUI();

protected:

	int timeOut;

	void setDefaults();
	bool loadParameters();
	bool saveParameters();

	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version) {
		ar & BOOST_SERIALIZATION_NVP(timeOut);
	}

};

