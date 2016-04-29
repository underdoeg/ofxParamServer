#include "ofxParamServerJson.h"

static std::map<std::string, ofxParamCastOrCreateFunc> paramCastOrCreateHandlers;
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
	json["children"] = {};
	for(auto p:params){
		json["children"].push_back(toJson(*p));
	}
}

template<typename Type>
void toJson(ofAbstractParameter& p, Json& json){
	ofParameter<Type> param = p.cast<Type>();
	json["value"] = param.get();
}

template<typename Type>
void toJsonMinMax(ofAbstractParameter& p, Json& json){
	ofParameter<Type> param = p.cast<Type>();
	json["value"] = param.get();
	fillMinMax(param, json);
}



///////////////////////////////////////// FROM JSON HELPERS

template<typename Type>
void jsonToGeneric(ofAbstractParameter* p, Json& json){
	ofParameter<Type>* param = static_cast<ofParameter<Type>*>(p);
	param->set(json["value"].get<Type>());
	if(json.find("min") != json.end()){
		param->setMin(json["min"].get<Type>());
	}
	if(json.find("max") != json.end()){
		param->setMax(json["max"].get<Type>());
	}
}

template<typename VectorType>
VectorType jsonArrayToVector(Json& j){
	VectorType v;
	if(j.size() < VectorType::DIM)
		return v;
	for(unsigned i=0; i<VectorType::DIM; i++){
		v[i] = j[i];
	}
	return v;
}


////////////////////////////////////////////////////////////////

template<typename VectorType>
void toJsonVecArray(const VectorType& in, Json& json){
	std::vector<float> v;
	for(unsigned i=0; i<VectorType::DIM; i++){
		v.push_back(in[i]);
	}
	json = v;
}

template<typename VectorType>
void toJsonVector(ofAbstractParameter& p, Json& json){
	ofParameter<VectorType> param = p.cast<VectorType>();
	toJsonVecArray<VectorType>(param.get(), json["value"]);
	toJsonVecArray<VectorType>(param.getMin(), json["min"]);
	toJsonVecArray<VectorType>(param.getMax(), json["max"]);
}


template<typename VectorType>
void jsonToVectorGeneric(ofAbstractParameter* p, Json& json){
	ofParameter<VectorType>* param = static_cast<ofParameter<VectorType>*>(p);
	param->set(jsonArrayToVector<VectorType>(json["value"]));
	if(json.find("min") != json.end()){
		param->setMin(jsonArrayToVector<VectorType>(json["min"]));
	}
	if(json.find("max") != json.end()){
		param->setMax(jsonArrayToVector<VectorType>(json["max"]));
	}
}

template<typename VectorType>
void addVectorType(std::string name){
	ofxParamServerAddType(typeid(ofParameter<VectorType>).name(), name, &toJsonVector<VectorType>, &jsonToVectorGeneric<VectorType>, &ofxParamServerCastOrCreate<ofParameter<VectorType>>);
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
	paramCastOrCreateHandlers[jsonTypeNames[typeid(ofParameterGroup).name()]] = &ofxParamServerCastOrCreate<ofParameterGroup>;

	//add all basic types
	addTypeGeneric<bool>("bool");

	addTypeGenericMinMax<int>("int");
	addTypeGenericMinMax<unsigned>("unsigned");
	addTypeGenericMinMax<float>("float");
	addTypeGenericMinMax<double>("double");
	addTypeGenericMinMax<long>("long");

	addTypeGeneric<std::string>("string");

	addVectorType<ofVec2f>("vec2");
	addVectorType<ofVec3f>("vec3");
	addVectorType<ofVec4f>("vec4");
}

void ofxParamServerAddType(string typeName, string niceName, ofxParamToJsonFunc toJson, ofxParamFromJsonFunc fromJson, ofxParamCastOrCreateFunc castOrCreateFunc){
	setupTypeHandlers();
	jsonTypeNames[typeName] = niceName;
	jsonToParamHandlers[niceName] = fromJson;
	paramToJsonHandlers[typeName] = toJson;
	paramCastOrCreateHandlers[niceName] = castOrCreateFunc;
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
/*
void printGroup(ofParameterGroup& group){
	for(auto p: group){
		ofLog() << p->getName();
		if(p->type() == typeid(ofParameterGroup).name()){
			printGroup(static_cast<ofParameterGroup&>(*p));
		}
	}
}
*/

//copied from ofPArameter
string escape(const string& _str){
	if(_str == "")
		return "no_name";

	std::string str(_str);

	ofStringReplace(str, " ", "_");
	ofStringReplace(str, "<", "_");
	ofStringReplace(str, ">", "_");
	ofStringReplace(str, "{", "_");
	ofStringReplace(str, "}", "_");
	ofStringReplace(str, "[", "_");
	ofStringReplace(str, "]", "_");
	ofStringReplace(str, ",", "_");
	ofStringReplace(str, "(", "_");
	ofStringReplace(str, ")", "_");
	ofStringReplace(str, "/", "_");
	ofStringReplace(str, "\\", "_");
	ofStringReplace(str, ".", "_");

	return str;
}


string joinList(vector<string> arr, string delimiter){
	if (arr.empty()) return "";

	string str;
	for (auto i : arr)
		str += i + delimiter;
	str = str.substr(0, str.size() - delimiter.size());
	return delimiter+str;
}

std::shared_ptr<ofAbstractParameter> findParamByPath(std::string path, std::vector<shared_ptr<ofAbstractParameter>> params){
	for(auto p: params){
		if(p && joinList(p->getGroupHierarchyNames(), "/") == path){
			return p;
		}
	}
	return nullptr;
}

std::vector<shared_ptr<ofAbstractParameter>> jsonToGroup(Json& json, std::string path="/", std::vector<shared_ptr<ofAbstractParameter>> existingParams = {}){
	setupTypeHandlers();

	if(json["type"] !="group"){
		ofLogWarning() << "Wrong json format" << json;
		return {};
	}

	std::string groupName = json["name"];
	std::string groupPath = path+escape(groupName);

	shared_ptr<ofParameterGroup> group = dynamic_pointer_cast<ofParameterGroup>(paramCastOrCreateHandlers["group"](findParamByPath(groupPath, existingParams)));
	std::vector<shared_ptr<ofAbstractParameter>> ret = {shared_ptr<ofAbstractParameter>(group)};

	group->setName(groupName);
	path += group->getEscapedName()+"/";

	//handle children
	for(auto& j: json["children"]){
		if(!j.is_null()){
			std::string type = j["type"];
			std::string paramName = j["name"];
			std::string paramPath = path+escape(paramName);

			shared_ptr<ofAbstractParameter> childParam = nullptr;

			if(type == "group"){
				auto childGroupParams = jsonToGroup(j, path, existingParams);
				ret.insert(ret.end(), childGroupParams.begin(), childGroupParams.end());
				childParam = childGroupParams[0];
			}else{
				if(jsonToParamHandlers.find(type) != jsonToParamHandlers.end()){
					childParam = paramCastOrCreateHandlers[type](findParamByPath(paramPath, existingParams));
					childParam->setName(paramName);
					jsonToParamHandlers[type](childParam.get(), j);
					ret.push_back(childParam);
				}else{
					ofLogWarning("ofxParamServer") << "unknwon json type " << type;
				}
			}

			//found a param, add to group
			if(childParam && !group->contains(childParam->getName()))
				group->add(*childParam.get());
		}
	}

	//check if we have to remove some parameters
	for(auto param: *group){
		if(findParamByPath(joinList(param->getGroupHierarchyNames(), "/"), ret) == nullptr){
			group->remove(*param);
		}
	}

	return ret;
}


std::vector<shared_ptr<ofAbstractParameter>> syncToJson(string jsonStr, std::vector<shared_ptr<ofAbstractParameter>> existingParams){
	if(!jsonStr.size())
		return {};
	return syncToJson(Json::parse(jsonStr), existingParams);
}

std::vector<shared_ptr<ofAbstractParameter>> syncToJson(Json json, std::vector<shared_ptr<ofAbstractParameter>> existingParams){
	setupTypeHandlers();
	std::vector<shared_ptr<ofAbstractParameter>> newParams = jsonToGroup(json, "/", existingParams);

	if(newParams.size()>0){
		ofParameterGroup* curParent = dynamic_cast<ofParameterGroup*>(newParams[0].get());
		for(auto p: existingParams){
			//if(p->type() == "group")
			//curParent = dynamic_cast<ofParameterGroup*>(p);

			if(std::find(newParams.begin(), newParams.end(), p) == newParams.end()){
				ofLogNotice("ofxParamServer") << "Deleting existing parameter " << joinList(p->getGroupHierarchyNames(), "/");
			}
		}
	}
	return newParams;
}
