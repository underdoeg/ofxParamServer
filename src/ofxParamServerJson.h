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

#endif // OFXPARAMSERVERJSON_H
