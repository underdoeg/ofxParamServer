#ifndef OFXPARAMSERVER_H
#define OFXPARAMSERVER_H

#include "ofMain.h"
#include "ofxOsc.h"
#include "microhttpd.h"
#include "ofxParamServerCommon.h"

class ofxParamServer{
public:
	ofxParamServer();
	void setup(ofParameterGroup& params, int port=7676, int httpPort=HTTP_PORT);

	ofParameterGroup& getParameters();

private:
	ofParameterGroup* params;
	MHD_Daemon* httpDaemon;
};

#endif // OFXPARAMSERVER_H
