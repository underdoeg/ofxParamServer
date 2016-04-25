#ifndef OFXPARAMSERVERJSON_H
#define OFXPARAMSERVERJSON_H

#include "ofMain.h"
#include "json.hpp"

// for convenience
using Json = nlohmann::json;

Json toJson(ofParameterGroup& params);
Json toJson(ofAbstractParameter& param);

std::vector<ofAbstractParameter*> syncToJson(std::string jsonStr, ofParameterGroup& params);
std::vector<ofAbstractParameter*> syncToJson(Json json, ofParameterGroup& params);


using ofxParamToJsonFunc = std::function<void(ofAbstractParameter&, Json&)>;
using ofxParamFromJsonFunc = std::function<ofAbstractParameter*(Json&)>;


void ofxParamServerAddType(std::string typeName, std::string niceName, ofxParamToJsonFunc toJson, ofxParamFromJsonFunc fromJson);

template<typename Type>
void ofxParamServerAddType(std::string niceName,ofxParamToJsonFunc toJson, ofxParamFromJsonFunc fromJson){
	ofxParamServerAddType(typeid(Type).name(), niceName, toJson, fromJson);
}

template<typename Type>
void ofxParamServerAddType(ofxParamToJsonFunc toJson, ofxParamFromJsonFunc fromJson){
	ofxParamServerAddType(typeid(Type).name(), typeid(Type).name(), toJson, fromJson);
}

//template<typename Type>;
//void addCustomParamServerType(std::function<);

#endif // OFXPARAMSERVERJSON_H
