/*
 *  SerializationUtils.h
 *
 *  Created by gameover on 20/06/12.
 *  Copyright 2012 Matthew Gingold. All rights reserved.
 *
 */

#ifndef _H_SERIALIZATIONUTILS
#define	_H_SERIALIZATIONUTILS

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/functional/hash.hpp>

#include "ofMain.h"

 /*
// this nomenclature lets you serialize classes
// even if you don't want to modify the class itself

namespace boost {
	namespace serialization {

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

//BOOST_CLASS_EXPORT_GUID(vector<ofColor>, "vector<ofColor>")
//BOOST_CLASS_EXPORT_GUID(vector<ofPoint>, "vector<ofPoint>")
//BOOST_CLASS_EXPORT_GUID(vector<ofRectangle>, "vector<ofRectangle>")


// example of how to use internally...
// ... with split load and save functions
class SomeClass {
public:
	// etc....

	SomeType someTypeVariable; // can be anything!!

	friend class boost::serialization::access;

	template<class Archive>
	void save(Archive & ar, const unsigned int version) const {

		ar & BOOST_SERIALIZATION_NVP(someTypeVariable); // saves someTypeVariable

	};

	template<class Archive>
	void load(Archive & ar, const unsigned int version) {

		ar & BOOST_SERIALIZATION_NVP(someTypeVariable); // loads someTypeVariable

		// do something with loaded vars

	};

	BOOST_SERIALIZATION_SPLIT_MEMBER()

};

// example of how to use internally...
// ... with single load/save function
class SomeClass {
public:
	// etc....

	SomeType someTypeVariable; // can be anything!!

	friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version){

		ar & BOOST_SERIALIZATION_NVP(someTypeVariable); // loads/saves someTypeVariable

	};

};

// ... and then elsewhere can call something like
SomeClass myClass;
Serializer.saveClass("somepath", myClass, ARCHIVE_BINARY);
...
Serializer.loadClass("somepath", myClass, ARCHIVE_BINARY);

// ... or internally
Serializer.saveClass("somepath", (*this), ARCHIVE_BINARY);
...
Serializer.loadClass("somepath", (*this), ARCHIVE_BINARY);

*/


enum ArchiveType{
    ARCHIVE_TEXT,
    ARCHIVE_BINARY,
    ARCHIVE_XML
};

class _Serializer {
public:
    
    template <class C>
    bool loadClass(string filePath, C &someClass, ArchiveType archiveType) {
        ofLogNotice() << "Loading class data: " << filePath << endl;
        std::ifstream ifs(ofToDataPath(filePath).c_str(), std::fstream::binary | std::fstream::in);
        if(ifs.fail()){
            ofLogError() << "Could not open file stream for loading: " << filePath << endl;
            return false;
        }
        try {
            ofLogNotice() << "Loading from...";
            switch (archiveType) {
                case ARCHIVE_TEXT:
                {
                    ofLogNotice() << "ARCHIVE_TEXT" << endl;
                    boost::archive::text_iarchive ia(ifs);
                    ia >> BOOST_SERIALIZATION_NVP(someClass);
                    break;
                }
                case ARCHIVE_BINARY:
                {
                    ofLogNotice() << "ARCHIVE_BINARY" << endl;
                    boost::archive::binary_iarchive ia(ifs);
                    ia >> BOOST_SERIALIZATION_NVP(someClass);
                    break;
                }
                case ARCHIVE_XML:
                {
                    ofLogNotice() << "ARCHIVE_XML" << endl;
                    boost::archive::xml_iarchive ia(ifs);
                    ia >> BOOST_SERIALIZATION_NVP(someClass);
                    break;
                }
            }
            
            return true;
        } catch (boost::archive::archive_exception e) {
            ofLogError() << "Error serializing class from file: " << filePath << " - " << e.what() << endl;
            return false;
        }
        
    }
    
    template <class C>
    bool saveClass(string filePath, C &someClass, ArchiveType archiveType) {
        ofLogNotice() << "Saving class data: " << filePath << endl;
        std::ofstream ofs(ofToDataPath(filePath).c_str(), std::ostream::binary | std::ostream::out);
        if(ofs.fail()){
            ofLogError() << "Could not open file stream for saving: " << filePath << endl;
            return false;
        }
        try {
            ofLogNotice() << "Saving to...";
            switch (archiveType) {
                case ARCHIVE_TEXT:
                {
                    ofLogNotice() << "ARCHIVE_TEXT" << endl;
                    boost::archive::text_oarchive oa(ofs);
                    oa << BOOST_SERIALIZATION_NVP(someClass);
                    break;
                }
                case ARCHIVE_BINARY:
                {
                    ofLogNotice() << "ARCHIVE_BINARY" << endl;
                    boost::archive::binary_oarchive oa(ofs);
                    oa << BOOST_SERIALIZATION_NVP(someClass);
                    break;
                }
                case ARCHIVE_XML:
                {
                    ofLogNotice() << "ARCHIVE_XML" << endl;
                    boost::archive::xml_oarchive oa(ofs);
                    oa << BOOST_SERIALIZATION_NVP(someClass);
                    break;
                }
            }
            return true;
        } catch (std::exception e) {
            ofLogError() << "Error serializing class to file: " << filePath << " - " << e.what() << endl;
            return false;
        }
    }

};

static _Serializer Serializer;

#endif
