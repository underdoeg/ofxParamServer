#ifndef OFXPARAMCLIENT_H
#define OFXPARAMCLIENT_H

#include "ofMain.h"
#include "ofxParamServerCommon.h"
#include "ofxParamServerJson.h"
#include "curl/curl.h"

class ofxParamClient{
public:
    ofxParamClient();
	~ofxParamClient();

	void setup(std::string serverIp="127.0.0.1", int oscPort=OSC_PORT, int httpPort=HTTP_PORT);

	void sync();

	ofParameterGroup& getParams();

private:
	std::string serverIp;
	int oscPort;
	int httpPort;
	ofParameterGroup paramGroup;
	std::vector<ofAbstractParameter*> params;
};

#endif // OFXPARAMCLIENT_H
