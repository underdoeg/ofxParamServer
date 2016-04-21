#include "ofxParamServerJson.h"


static std::map<size_t, std::string> typeNames;
static bool bTypenamesSetup = false;

void setupTypenames(){
	if(bTypenamesSetup)
		return;

	////////////////////////////////
	/////////// ADD NEW TYPES HERE
	typeNames[typeid(bool).hash_code()] = "bool";
	typeNames[typeid(int).hash_code()] = "int";
	typeNames[typeid(float).hash_code()] = "float";
	typeNames[typeid(double).hash_code()] = "double";
	typeNames[typeid(std::string).hash_code()] = "string";

	bTypenamesSetup = true;
}

std::string getTypename(size_t hash){
	if(typeNames.find(hash) != typeNames.end())
		return typeNames[hash];
	return "unknown";
}

template<typename Type>
std::string getTypename(){
	return getTypename(typeid(Type).hash_code());
}

template<typename Type>
std::string getTypename(Type& t){
	return getTypename<Type>();
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// TO JSON
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static std::map<std::string, std::function<void(ofAbstractParameter&, Json&)>>paramToJsonHandlers;
static bool bParamHandlerSetup = false;

/////////////////////////////////////////////////////////

std::string getName(ofAbstractParameter& param){
	std::string name = param.getName();
	if(!name.size())
		name = "no name";
	return name;
}

template<typename Type>
void fillMinMax(ofParameter<Type>& param, Json& j){
	j["min"] = param.getMin();
	j["max"] = param.getMax();
}

void groupToJson(ofAbstractParameter& param, Json& json){
	ofParameterGroup& params = static_cast<ofParameterGroup&>(param);
	json["type"] = "group";
	json["children"] = {};
	for(auto p:params){
		json["children"][getName(*p)] = toJson(*p);
	}
}

template<typename Type>
void toJson(ofAbstractParameter& p, Json& json){
	ofParameter<Type> param = p.cast<Type>();
	json["type"] = getTypename<Type>();
	json["value"] = param.get();
}

template<typename Type>
void toJsonMinMax(ofAbstractParameter& p, Json& json){
	ofParameter<Type> param = p.cast<Type>();
	json["type"] = getTypename<Type>();
	json["value"] = param.get();
	fillMinMax(param, json);
}


////////////////////////////////////////////

template<typename Type>
static void addParamHandler(std::function<void(ofAbstractParameter&, Json&)> handler){
	paramToJsonHandlers[typeid(ofParameter<Type>).name()] = handler;
}

template<typename Type>
static void addParamHandler(){
	paramToJsonHandlers[typeid(ofParameter<Type>).name()] = &toJson<Type>;
}

template<typename Type>
static void addParamHandlerMinMax(){
	paramToJsonHandlers[typeid(ofParameter<Type>).name()] = &toJsonMinMax<Type>;
}

static void setupParamHandlers(){
	if(bParamHandlerSetup)
		return;

	setupTypenames();

	paramToJsonHandlers[typeid(ofParameterGroup).name()] = &groupToJson;

	////////////////////////////////
	/////////// ADD NEW TYPES HERE
	addParamHandler<bool>();
	addParamHandlerMinMax<int>();
	addParamHandlerMinMax<float>();
	addParamHandler<std::string>();

	bParamHandlerSetup = true;
}

/////////////////////////////////////////////////


Json toJson(ofAbstractParameter& param){
	setupParamHandlers();


	if(param.isReadOnly()){
		ofLogWarning("ofxParamServer") << "Read Only parameters are not implemented";
		return {};
	}

	std::string type = param.type();
	if(paramToJsonHandlers.find(type) == paramToJsonHandlers.end()){
		ofLogWarning("ofxParamServer") << "Type " << type << " not implemented";
		return {};
	}

	Json json;
	json["name"] = param.getName().size() ? param.getName() : "no name";
	paramToJsonHandlers[type](param, json);
	return json;
}

Json toJson(ofParameterGroup& params){
	return toJson(static_cast<ofAbstractParameter&>(params));
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// FROM JSON
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void jsonCacheName(Json& json){
	for (Json::iterator it = json.begin(); it != json.end(); ++it) {
		if(it.value().is_object()){
			it.value()["nameCached"] = it.key();
			jsonCacheName(it.value());
		}
	}
}

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
		ret->setMin(json["max"].get<Type>());
	}
	return ret;
}

static std::map<std::string, std::function<ofAbstractParameter*(Json&)>> jsonHandlers;
static bool bJsonHandlersSetup = false;


///////////////////////////

template<typename Type>
void addJsonHandler(){
	jsonHandlers[getTypename<Type>()] = &jsonToGeneric<Type>;
}

void setupJsonHandlers(){
	if(bJsonHandlersSetup)
		return;

	setupTypenames();

	////////////////////////////////
	/////////// ADD NEW TYPES HERE
	//jsonHandlers["int"] = &jsonTo<int>;
	//jsonHandlers["bool"] = &jsonTo<bool>;
	addJsonHandler<bool>();
	addJsonHandler<int>();
	addJsonHandler<float>();
	addJsonHandler<double>();
	addJsonHandler<std::string>();

	bJsonHandlersSetup = true;
}

std::vector<ofAbstractParameter*> jsonToGroup(Json& json, ofParameterGroup* group = nullptr){
	setupJsonHandlers();

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
			if(type == "group"){
				std::vector<ofAbstractParameter*> params = jsonToGroup(j);
				ret.insert(ret.end(), params.begin(), params.end());
				group->add(*params[0]);
			}else{
				if(jsonHandlers.find(type) != jsonHandlers.end()){
					ofAbstractParameter* param = jsonHandlers[type](j);
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


////////////

std::vector<ofAbstractParameter *> syncToJson(string jsonStr, ofParameterGroup &params){
	if(!jsonStr.size())
		return {};
	return syncToJson(Json::parse(jsonStr), params);
}

std::vector<ofAbstractParameter *> syncToJson(Json json, ofParameterGroup &params){
	setupJsonHandlers();
	return jsonToGroup(json, &params);
}
