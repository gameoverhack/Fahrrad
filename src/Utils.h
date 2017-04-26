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

