#pragma once

#include "SerializationUtils.h"
#include "ofxImGui.h"

const static string API_KEY = "8eae23d79b1fa5153426d0c5b966ac2b";
const static string API_SECRET = "db2d65d186157c1e";

#ifdef TARGET_WIN32
#define CONFIG_TYPE ".winconf"
#else
#define CONFIG_TYPE ".lnxconf"
#endif

static int days_between( struct tm &tsa,  struct tm &tsb) {
	time_t a = mktime(&tsa);
	time_t b = mktime(&tsb);
	//cout << difftime(b, a) / (60 * 60 * 24) << endl;
	return difftime(b, a) / (60 * 60 * 24);
}
static int day_of_week(const int& d1, const int& m1, const int& y1) {
	struct tm tsa = { 0 };
	tsa.tm_hour = 0;
	tsa.tm_sec = 0;
	tsa.tm_mday = d1;
	tsa.tm_mon = m1;
	tsa.tm_year = y1 - 1900;
	time_t a = mktime(&tsa);
	return tsa.tm_wday;
}

static int days_between(const int& d1, const int& m1, const int& y1, const int& d2, const int& m2, const int& y2) {
	
	struct tm tsa = { 0 };
	struct tm tsb = { 0 };

	//cout << "d: " << d1 << " " << m1 << " " << y1 << " " << d2 << " " << m2 << " " << y2 << " " << endl;

	tsa.tm_hour = 0;
	tsa.tm_sec = 0;
	tsa.tm_mday = d1;
	tsa.tm_mon = m1;
	tsa.tm_year = y1 - 1900;

	tsb.tm_hour = 0;
	tsb.tm_sec = 0;
	tsb.tm_mday = d2;
	tsb.tm_mon = m2;
	tsb.tm_year = y2 - 1900;

	return days_between(tsa, tsb);
}

typedef union {
	float*	data = nullptr;
	char*	chars;
} RiderSummaryUnion;

typedef enum {
	RS_DATA_SIZE = 0,
	RS_NUM_DAYS,
	RS_IS_ACTIVE,
	//RS_RANK_DAILY,
	//RS_RANK_ALL,
	RS_SPEED_CURRENT,
	//RS_SPEED_TOP,
	//RS_SPEED_NORMAL,
	//RS_SPEED_ANIMAL,
	//RS_KW_CURRENT,
	//RS_KW_TOP,
	//RS_KW_NORMAL,
	//RS_KW_DEVICE,
	//RS_DISTANCE_CURRENT,
	RS_DISTANCE_DAY,
	RS_DISTANCE_TOTAL,
	//RS_TIME_CURRENT,
	//RS_TIME_DAY,
	RS_TIME_TOTAL,
	RS_RIDERS_TOTAL,
	RS_DATA_START

};

static string getString(float n, int precision = 0, int fill = 0) {
	ostringstream os;
	os << std::setfill('0') << std::setw(fill) << std::setprecision(precision) << std::fixed << n;
	return os.str();
}

typedef struct {

	bool isActive = false;

	int dayranking = INFINITY;
	int allranking = INFINITY;

	float normalisedSpeed = 0.0f;
	float currentSpeed = 0.0f;
	float topSpeed = 0.0f;
	float currentKiloWatts = 0.0f;
	float topKiloWatts = 0.0f;
	float distanceTravelled = 0.0f;

	int day = 0;
	int month = 0;
	int year = 0;
	
	int time = 0; // millis

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version) {
		ar & BOOST_SERIALIZATION_NVP(dayranking);
		ar & BOOST_SERIALIZATION_NVP(allranking);
		ar & BOOST_SERIALIZATION_NVP(topSpeed);
		ar & BOOST_SERIALIZATION_NVP(topKiloWatts);
		ar & BOOST_SERIALIZATION_NVP(distanceTravelled);
		ar & BOOST_SERIALIZATION_NVP(day);
		ar & BOOST_SERIALIZATION_NVP(month);
		ar & BOOST_SERIALIZATION_NVP(year);
		ar & BOOST_SERIALIZATION_NVP(time);
	}

} RiderInfo;

inline void uniqueRandomIndex(vector<int> & vec, int start, int end, int size) {

	int r = (int)ofRandom(start, end);
	bool bInVec = false;

	for (int i = 0; i < vec.size(); i++) {
		if (vec[i] == r) {
			bInVec = true;
			break;
		}
	}

	if (!bInVec) vec.push_back(r);

	if (vec.size() != size && end - start >= size) uniqueRandomIndex(vec, start, end, size);

}

//from: https://eliasdaler.github.io/using-imgui-with-sfml-pt2

namespace ImGui
{
	static auto vector_getter = [](void* vec, int idx, const char** out_text)
	{
		auto& vector = *static_cast<std::vector<std::string>*>(vec);
		if (idx < 0 || idx >= static_cast<int>(vector.size())) { return false; }
		*out_text = vector.at(idx).c_str();
		return true;
	};

	inline bool Combo(const char* label, int* currIndex, std::vector<std::string>& values)
	{
		if (values.empty()) { return false; }
		return Combo(label, currIndex, vector_getter,
			static_cast<void*>(&values), values.size());
	}

	inline bool ListBox(const char* label, int* currIndex, std::vector<std::string>& values)
	{
		if (values.empty()) { return false; }
		return ListBox(label, currIndex, vector_getter,
			static_cast<void*>(&values), values.size());
	}

}

namespace boost {
	namespace serialization {

		template<class Archive>
		void serialize(Archive & ar, ImVec4 & i, const unsigned int version) {
			ar & BOOST_SERIALIZATION_NVP(i.x);
			ar & BOOST_SERIALIZATION_NVP(i.y);
			ar & BOOST_SERIALIZATION_NVP(i.z);
			ar & BOOST_SERIALIZATION_NVP(i.w);
		};

		template<class Archive>
		void serialize(Archive & ar, ofColor & c, const unsigned int version) {
			ar & BOOST_SERIALIZATION_NVP(c.r);
			ar & BOOST_SERIALIZATION_NVP(c.g);
			ar & BOOST_SERIALIZATION_NVP(c.b);
			ar & BOOST_SERIALIZATION_NVP(c.a);
		};

		template<class Archive>
		void serialize(Archive & ar, ofPoint & p, const unsigned int version) {
			ar & BOOST_SERIALIZATION_NVP(p.x);
			ar & BOOST_SERIALIZATION_NVP(p.y);
			ar & BOOST_SERIALIZATION_NVP(p.z);
		};

		template<class Archive>
		void serialize(Archive & ar, ofRectangle & r, const unsigned int version) {
			ar & BOOST_SERIALIZATION_NVP(r.x);
			ar & BOOST_SERIALIZATION_NVP(r.y);
			ar & BOOST_SERIALIZATION_NVP(r.height);
			ar & BOOST_SERIALIZATION_NVP(r.width);
		};
	};
};

