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

//

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

	return ret;

	/*
	ofParameterGroup* group = dynamic_cast<ofParameterGroup*>(parentGroup.get());

	if(!group){
		group = new ofParameterGroup;
		ofLog() << "new group";
	}

	group->setName(json["name"].get<std::string>());



	path += group->getEscapedName()+"/";

	for(auto p: existingParams){
		//ofLog() << p->getName();
	}


	//std::vector<shared_ptr<ofAbstractParameter>> toAdd;

	//add children
	for(auto& j: json["children"]){
		if(!j.is_null()){
			std::string type = j["type"];
			std::string paramName = j["name"];
			std::string paramPath = path+escape(paramName);
			//can be null
			ofAbstractParameter* existingParam = findParamByPath(paramPath, existingParams);

			if(type == "group"){
				ofParameterGroup* existingGroup = dynamic_cast<ofParameterGroup*>(paramCastOrCreateHandlers[type](existingParam));
				//if(existingGroup)
					//existingGroup->clear();

				std::vector<shared_ptr<ofAbstractParameter>> params = jsonToGroup(j, shared_ptr<ofAbstractParameter>(existingGroup), path, existingParams);
				//ret.insert(ret.end(), params.begin(), params.end());

				group->add(*existingGroup);

				//toAdd.push_back(shared_ptr<ofAbstractParameter>(existingGroup));
			}else{
				if(jsonToParamHandlers.find(type) != jsonToParamHandlers.end()){
					shared_ptr<ofAbstractParameter> param = shared_ptr<ofAbstractParameter>(paramCastOrCreateHandlers[type](existingParam));
					param->setName(paramName);
					jsonToParamHandlers[type](param.get(), j);

					group->add(*param);
					//toAdd.push_back(param);
					ofLog() << "PUSHING BACK " << param->getName();
					ret.push_back(param);
				}else{
					ofLogWarning("ofxParamServer") << "unknwon json type " << type;
				}
			}
		}else{
			ofLogWarning("ofxParamServer") << "json is null -> SKIPPING";
		}
	}

	//group->clear();
	group->setName(json["name"].get<std::string>());

	*/
	//for(auto param: toAdd){
	/*
		if(!group->contains(param->getName()))
			group->add(*param);
		param->getGroupHierarchyNames();
		*/
	//}
	/*
	for(auto p: ret){
		//ofLog() << p->getName();
	}
	*/

}


std::vector<shared_ptr<ofAbstractParameter>> syncToJson(string jsonStr, std::vector<shared_ptr<ofAbstractParameter>> existingParams){
	if(!jsonStr.size())
		return {};
	return syncToJson(Json::parse(jsonStr), existingParams);
}

std::vector<shared_ptr<ofAbstractParameter>> syncToJson(Json json, std::vector<shared_ptr<ofAbstractParameter>> existingParams){
	setupTypeHandlers();
	//std::vector<shared_ptr<ofAbstractParameter>> newParams = jsonToGroup(json, "/", existingParams);
	/*
	ofParameterGroup* curParent = &params;
	for(auto p: existingParams){
		//if(p->type() == "group")
			//curParent = dynamic_cast<ofParameterGroup*>(p);

		if(std::find(newParams.begin(), newParams.end(), p) == newParams.end()){
			ofLogNotice("ofxParamServer") << "Deleting existing parameter " << joinList(p->getGroupHierarchyNames(), "/");
		}
	}



	/*
	for(auto p: newParams){
		if(p->type() == typeid(ofParameterGroup).name()){
			ofParameterGroup* group = dynamic_cast<ofParameterGroup*>(p);
			for(auto child: *group){
				ofLog() << joinList(child->getGroupHierarchyNames(), "/");
			}
		}
		ofLog() << joinList(p->getGroupHierarchyNames(), "/");
	}
	*/

	return jsonToGroup(json, "/", existingParams);
}
