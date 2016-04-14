#include "ofxParamServer.h"
#include "ofxParamServerJson.h"

static int answerHttp(void *cls, struct MHD_Connection *connection,
					  const char *url, const char *method,
					  const char *version, const char *upload_data,
					  size_t *upload_data_size, void **con_cls){

	ofxParamServer* paramServer = (ofxParamServer*)cls;

	Json j = toJson(paramServer->getParameters());

	const char *page = j.dump(4).c_str();
	struct MHD_Response *response;
	int ret;

	response = MHD_create_response_from_buffer (strlen (page), (void *) page, MHD_RESPMEM_PERSISTENT);
	ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
	MHD_destroy_response (response);

	return ret;
}


///////////////////////////

ofxParamServer::ofxParamServer():params(nullptr){

}

void ofxParamServer::setup(ofParameterGroup& p, int port, int httpPort){
	params = &p;
	httpDaemon = MHD_start_daemon (MHD_USE_SELECT_INTERNALLY, httpPort, NULL, NULL, &answerHttp, this, MHD_OPTION_END);
}

ofParameterGroup &ofxParamServer::getParameters(){
	if(!params) {
		throw "No parameters set";
	}

	return *params;
}
