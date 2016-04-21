#include "ofxParamServer.h"
#include "ofxParamServerJson.h"

static int answerHttp(void *cls, struct MHD_Connection *connection,
					  const char *url, const char *method,
					  const char *version, const char *upload_data,
					  size_t *upload_data_size, void **con_cls){

	ofxParamServer* paramServer = (ofxParamServer*)cls;

	Json j = toJson(paramServer->getParameters());

	std::string pageStr = j.dump();
	struct MHD_Response *response;
	int ret;

	response = MHD_create_response_from_buffer(pageStr.size(), (void *)pageStr.c_str(), MHD_RESPMEM_MUST_COPY);

	std::string mime = "application/json";

	MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, mime.c_str());

	ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
	MHD_destroy_response(response);

	return ret;
}


///////////////////////////

ofxParamServer::ofxParamServer():params(nullptr){

}

void ofxParamServer::setup(ofParameterGroup& p, const std::string clientIp, int portLocal, int portClients, int httpPort){
	params = &p;
	httpDaemon = MHD_start_daemon (MHD_USE_SELECT_INTERNALLY, httpPort, NULL, NULL, &answerHttp, this, MHD_OPTION_END);
	//ofAddListener(params->parameterChangedE(), this, &ofxParamServer::onParamChanged);

	sync.setup(getParameters(), portLocal, clientIp, portClients);

	ofAddListener(ofEvents().update, this, &ofxParamServer::update);
}

ofParameterGroup &ofxParamServer::getParameters(){
	if(!params) {
		throw "No parameters set";
	}

	return *params;
}

void ofxParamServer::onParamChanged(ofAbstractParameter &param){
	ofLogNotice("ofxParameterServer") << "param changed";
}

void ofxParamServer::update(ofEventArgs& args){
	sync.update();
}
