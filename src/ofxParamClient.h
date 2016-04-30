#ifndef OFXPARAMCLIENT_H
#define OFXPARAMCLIENT_H

#include "ofMain.h"
#include "ofxParamServerCommon.h"
#include "ofxParamServerJson.h"
#include "curl/curl.h"

#include <ofxOscParameterSync.h>

class ofxParamClient{
public:
    ofxParamClient();
	~ofxParamClient();

	void setup(std::string serverIp="127.0.0.1", int oscPortLocal=OSC_PORT_CLIENT, int oscPortServer=OSC_PORT_SERVER, int httpPort=HTTP_PORT);

	void sync();
	void sendAll();

	void update();

	bool isSynced();

	void forceServerSync();

	ofParameterGroup& getParams();

	void save(std::string path);
	void load(std::string path);

private:
	void onParamChanged(ofAbstractParameter& param);

	std::string serverIp;
	int oscPortLocal;
	int oscPortServer;
	int httpPort;

	std::vector<shared_ptr<ofAbstractParameter>> params;

	ofxOscParameterSync paramSync;
};

#endif // OFXPARAMCLIENT_H
