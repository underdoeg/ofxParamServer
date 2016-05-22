#ifndef OFXPARAMSERVERUTILS_H
#define OFXPARAMSERVERUTILS_H

#include "ofMain.h"

static std::string getPath(ofAbstractParameter& param){
	std::string ret = "";
	for(auto name: param.getGroupHierarchyNames()){
		ret += "/"+name;
	}
	return ret;
}



#endif // OFXPARAMSERVERUTILS_H
