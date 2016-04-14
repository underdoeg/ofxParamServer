#include "ofxParamServerJson.h"


static std::map<std::string, std::function<Json(ofAbstractParameter&)>>paramToJsonHandlers;
static bool bParamHandlerSetup = false;

/////////////////////////////////////////////////////////

template<typename Type>
void fillMinMax(ofParameter<Type>& param, Json& j){
	j["min"] = param.getMin();
	j["max"] = param.getMax();
}

Json groupToJson(ofAbstractParameter& param){
	return toJson(static_cast<ofParameterGroup&>(param));
}

Json intToJson(ofAbstractParameter& p){
	ofParameter<int> param = p.cast<int>();
	Json ret;
	ret["type"] = "int";
	ret["value"] = param.get();
	fillMinMax(param, ret);
	return ret;
}

Json floatToJson(ofAbstractParameter& p){
	ofParameter<float> param = p.cast<float>();
	Json ret;
	ret["type"] = "float";
	ret["value"] = param.get();
	fillMinMax(param, ret);
	return ret;
}

////////////////////////////////////////////

template<typename Type>
static void addParamHandler(std::function<Json(ofAbstractParameter&)> handler){
	paramToJsonHandlers[typeid(ofParameter<Type>).name()] = handler;
}

static void setupParamHandlers(){
	paramToJsonHandlers[typeid(ofParameterGroup).name()] = &groupToJson;
	addParamHandler<int>(&intToJson);
	addParamHandler<float>(&floatToJson);
}

/////////////////////////////////////////////////

Json toJson(ofParameterGroup& params){
	setupParamHandlers();

	Json ret;
	ret["type"] = "group";
	for(auto p:params){
		ret["children"][p->getName()] = toJson(*p);
	}
	return ret;
}

Json toJson(ofAbstractParameter& param){
	setupParamHandlers();

	std::string type = param.type();
	if(paramToJsonHandlers.find(type) == paramToJsonHandlers.end()){
		ofLogWarning("ofxParamServer") << "Type " << type << " not implemented";
		return {};
	}
	return paramToJsonHandlers[type](param);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// FROM JSON
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void jsonCacheName(Json& json){
	for(auto& j: json){
		ofLog() << j;
	}
}

std::vector<ofAbstractParameter*> jsonToGroup(Json json, ofParameterGroup* parent=NULL){
	if(json["type"] !="group"){
		ofLogWarning() << "Wrong json format" << json;
		return {};
	}



	std::vector<ofAbstractParameter*> ret;
	return ret;
}

std::vector<ofAbstractParameter *> syncToJson(string jsonStr, ofParameterGroup &params){
	return syncToJson(Json::parse(jsonStr), params);
}

std::vector<ofAbstractParameter *> syncToJson(Json json, ofParameterGroup &params){
	jsonCacheName(json);
	return jsonToGroup(json);
}
