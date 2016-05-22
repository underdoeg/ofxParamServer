#ifndef OFXPARAMSERVERJSON_H
#define OFXPARAMSERVERJSON_H

#include "ofMain.h"
#include "json.hpp"
#include "ofxParamServerString.h"

// for convenience
using Json = nlohmann::json;

Json toJson(ofParameterGroup& params);
Json toJson(ofAbstractParameter& param);

std::vector<shared_ptr<ofAbstractParameter>> syncToJson(std::string jsonStr, std::vector<shared_ptr<ofAbstractParameter>> existingParams = {});
std::vector<shared_ptr<ofAbstractParameter>> syncToJson(Json json, std::vector<shared_ptr<ofAbstractParameter>> existingParams = {});


void fromString(ofAbstractParameter& param, std::string value);
/////////////////////////////////////////////////////////////////////////

using ofxParamToJsonFunc = std::function<void(ofAbstractParameter&, Json&)>;
using ofxParamFromJsonFunc = std::function<void(ofAbstractParameter*, Json&)>;
using ofxParamCastOrCreateFunc = std::function<shared_ptr<ofAbstractParameter>(shared_ptr<ofAbstractParameter>)>;
using ofxParamFromStringFunc = std::function<void(ofAbstractParameter&, std::string&)>;

template<typename Type>
shared_ptr<ofAbstractParameter> ofxParamServerCastOrCreate(shared_ptr<ofAbstractParameter> param){
	if(!param)
		return shared_ptr<ofAbstractParameter>(new Type());

	if(!dynamic_pointer_cast<Type>(param)){
		return shared_ptr<ofAbstractParameter>(new Type());
	}
	return param;
}

void ofxParamServerAddType(std::string typeName, std::string niceName, ofxParamToJsonFunc toJson, ofxParamFromJsonFunc fromJson, ofxParamCastOrCreateFunc castOrCreateFunc, ofxParamFromStringFunc fromStringFunc);

template<typename Type>
void ofxParamServerAddType(std::string niceName,ofxParamToJsonFunc toJson, ofxParamFromJsonFunc fromJson, ofxParamFromStringFunc fromStringFunc){
	ofxParamServerAddType(typeid(Type).name(), niceName, toJson, fromJson, &ofxParamServerCastOrCreate<Type>, fromStringFunc);
}

template<typename Type>
void ofxParamServerAddType(ofxParamToJsonFunc toJson, ofxParamFromJsonFunc fromJson, ofxParamFromStringFunc fromStringFunc){
	ofxParamServerAddType<Type>(typeid(Type).name(), toJson, fromJson, fromStringFunc);
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
