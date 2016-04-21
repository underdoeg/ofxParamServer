#ifndef OFXPARAMSERVER_H
#define OFXPARAMSERVER_H

#include "ofMain.h"
#include "ofxOsc.h"
#include "microhttpd.h"
#include "ofxParamServerCommon.h"

#include <ofxOscParameterSync.h>

class ofxParamServer{
public:
	ofxParamServer();
	void setup(ofParameterGroup& params, const std::string clientIp="127.0.0.1", int portLocal=OSC_PORT_SERVER, int portClients=OSC_PORT_CLIENT, int httpPort=HTTP_PORT);

	ofParameterGroup& getParameters();

private:
	void onParamChanged(ofAbstractParameter& param);
	void update(ofEventArgs& args);

	ofParameterGroup* params;
	MHD_Daemon* httpDaemon;

	ofxOscParameterSync sync;
};

#endif // OFXPARAMSERVER_H
