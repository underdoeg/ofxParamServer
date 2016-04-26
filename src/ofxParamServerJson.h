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
using ofxParamFromJsonFunc = std::function<void(ofAbstractParameter*, Json&)>;
using ofxParamCastOrCreateFunc = std::function<ofAbstractParameter*(ofAbstractParameter*)>;

template<typename Type>
ofAbstractParameter* ofxParamServerCastOrCreate(ofAbstractParameter* param){
	ofAbstractParameter* ret = dynamic_cast<Type*>(param);
	if(!ret){
		if(param) //was wrong type
			delete param;
		ret = new Type();
	}
	return ret;
}

void ofxParamServerAddType(std::string typeName, std::string niceName, ofxParamToJsonFunc toJson, ofxParamFromJsonFunc fromJson, ofxParamCastOrCreateFunc castOrCreateFunc);

template<typename Type>
void ofxParamServerAddType(std::string niceName,ofxParamToJsonFunc toJson, ofxParamFromJsonFunc fromJson){
	ofxParamServerAddType(typeid(Type).name(), niceName, toJson, fromJson, &ofxParamServerCastOrCreate<Type>);
}

template<typename Type>
void ofxParamServerAddType(ofxParamToJsonFunc toJson, ofxParamFromJsonFunc fromJson){
	ofxParamServerAddType<Type>(typeid(Type).name(), toJson, fromJson);
}

template<typename Type>
void ofxParamServerAddType(){
	ofLogError("ofxParamServerAddType") <<  "you must provide an overrided version of this function with your parameter type. Like this:";
	ofLogError("ofxParamServerAddType") <<  "template<>";
	ofLogError("ofxParamServerAddType") <<  "ofxParamServerAddType<YourType>(){....}";
}

//template<typename Type>;
//void addCustomParamServerType(std::function<);

#endif // OFXPARAMSERVERJSON_H
