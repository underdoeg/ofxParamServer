#include "ofxParamServerJson.h"

static std::map<std::string, ofxParamToJsonFunc> paramToJsonHandlers;
static std::map<std::string, ofxParamFromJsonFunc> jsonToParamHandlers;
static std::map<std::string, std::string> jsonTypeNames;
static bool bHandlersSetup = false;


///////////////////////////////////// TO JSON HELPERS

std::string getTypename(std::string name){
	std::string ret = name;
	if(jsonTypeNames.find(ret) != jsonTypeNames.end()){
		ret = jsonTypeNames[ret];
	}
	return ret;
}

template<typename Type>
std::string getTypename(){
	return getTypename(typeid(Type).name());
}

template<typename Type>
std::string getTypename(Type& t){
	return getTypename<Type>();
}


template<typename Type>
void fillMinMax(ofParameter<Type>& param, Json& j){
	j["min"] = param.getMin();
	j["max"] = param.getMax();
}

void groupToJson(ofAbstractParameter& param, Json& json){
	ofParameterGroup& params = static_cast<ofParameterGroup&>(param);
	//json["type"] = "group";
	json["children"] = {};
	for(auto p:params){
		json["children"].push_back(toJson(*p));
	}
}

template<typename Type>
void toJson(ofAbstractParameter& p, Json& json){
	ofParameter<Type> param = p.cast<Type>();
	//json["type"] = getTypename<Type>();
	json["value"] = param.get();
}

template<typename Type>
void toJsonMinMax(ofAbstractParameter& p, Json& json){
	ofParameter<Type> param = p.cast<Type>();
	//json["type"] = getTypename<Type>();
	json["value"] = param.get();
	fillMinMax(param, json);
}


///////////////////////////////////////// FROM JSON HELPERS


template<typename Type>
void fillMinMax(ofParameter<Type>* param, Json& j){
	param->setMin(j["min"].get<Type>());
	param->setMax(j["max"].get<Type>());
}

//

template<typename Type>
ofAbstractParameter* jsonToGeneric(Json& json){
	ofParameter<Type>* ret = new ofParameter<Type>();
	ret->set(json["value"].get<Type>());
	if(json.find("min") != json.end()){
		ret->setMin(json["min"].get<Type>());
	}
	if(json.find("max") != json.end()){
		ret->setMax(json["max"].get<Type>());
	}
	return ret;
}




///////////////////////////////////////////////////////////////

template<typename Type>
void addTypeGeneric(std::string name){
	ofxParamServerAddType<ofParameter<Type>>(name, &toJson<Type>, &jsonToGeneric<Type>);
}

template<typename Type>
void addTypeGenericMinMax(std::string name){
	ofxParamServerAddType<ofParameter<Type>>(name, &toJsonMinMax<Type>, &jsonToGeneric<Type>);
}

void setupTypeHandlers(){
	if(bHandlersSetup)
		return;

	bHandlersSetup = true;

	//groups are special
	paramToJsonHandlers[typeid(ofParameterGroup).name()] = &groupToJson;
	jsonTypeNames[typeid(ofParameterGroup).name()] = "group";

	//ofxParamServerAddType<ofParameterGroup>("group", &groupToJson, NULL);


	//add all basic types
	addTypeGeneric<bool>("bool");

	addTypeGenericMinMax<int>("int");
	addTypeGenericMinMax<unsigned>("unsigned");
	addTypeGenericMinMax<float>("float");
	addTypeGenericMinMax<double>("double");
	addTypeGenericMinMax<long>("long");

	addTypeGeneric<std::string>("string");


}

void ofxParamServerAddType(string typeName, string niceName, ofxParamToJsonFunc toJson, ofxParamFromJsonFunc fromJson){
	setupTypeHandlers();
	jsonTypeNames[typeName] = niceName;
	jsonToParamHandlers[niceName] = fromJson;
	paramToJsonHandlers[typeName] = toJson;
}


////////////////////////////////////////////////////////////////////

Json toJson(ofAbstractParameter& param){
	setupTypeHandlers();


	if(param.isReadOnly()){
		ofLogWarning("ofxParamServerJson") << "Read Only parameters are not implemented";
		return {};
	}

	std::string type = param.type();
	if(paramToJsonHandlers.find(type) == paramToJsonHandlers.end()){
		ofLogWarning("ofxParamServerJson") << "Type " << type << " not implemented";
		return {};
	}

	Json json;
	json["name"] = param.getName().size() ? param.getName() : "no name";
	json["type"] = getTypename(param.type());
	paramToJsonHandlers[type](param, json);
	return json;
}

Json toJson(ofParameterGroup& params){
	return toJson(static_cast<ofAbstractParameter&>(params));
}


///////////////////////////////////////////////////////////////////

std::vector<ofAbstractParameter*> jsonToGroup(Json& json, ofParameterGroup* group = nullptr){
	setupTypeHandlers();

	if(json["type"] !="group"){
		ofLogWarning() << "Wrong json format" << json;
		return {};
	}

	if(!group){
		group = new ofParameterGroup;
	}

	group->setName(json["name"].get<std::string>());

	std::vector<ofAbstractParameter*> ret = {group};

	//add children
	for(auto& j: json["children"]){
		if(!j.is_null()){
			std::string type = j["type"];
			ofLog() << type;
			if(type == "group"){
				std::vector<ofAbstractParameter*> params = jsonToGroup(j);
				ret.insert(ret.end(), params.begin(), params.end());
				group->add(*params[0]);
			}else{
				if(jsonToParamHandlers.find(type) != jsonToParamHandlers.end()){
					ofAbstractParameter* param = jsonToParamHandlers[type](j);
					param->setName(j["name"]);
					group->add(*param);
					ret.push_back(param);
				}else{
					ofLogWarning("ofxParamServer") << "unknwon json type " << type;
				}
			}
		}else{
			ofLogWarning("ofxParamServer") << "json is null -> SKIPPING";
		}
	}

	return ret;
}

std::vector<ofAbstractParameter *> syncToJson(string jsonStr, ofParameterGroup &params){
	if(!jsonStr.size())
		return {};
	return syncToJson(Json::parse(jsonStr), params);
}

std::vector<ofAbstractParameter *> syncToJson(Json json, ofParameterGroup &params){
	setupTypeHandlers();
	return jsonToGroup(json, &params);
}
