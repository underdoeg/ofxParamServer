#include "ofxParamClient.h"
#include "ofxParamServerUtils.h"


size_t curlReadCallback(void *contents, size_t size, size_t nmemb, std::string *s){
	size_t newLength = size*nmemb;
	size_t oldLength = s->size();
	try{
		s->resize(oldLength + newLength);
	}catch(std::bad_alloc &e){
		return 0;
	}

	std::copy((char*)contents,(char*)contents+newLength,s->begin()+oldLength);
	return size*nmemb;
}

std::string curlRead(std::string url){
	std::string ret;
	CURL* curl = curl_easy_init();
	if(curl){
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlReadCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ret);

		CURLcode res = curl_easy_perform(curl);
		if(res != CURLE_OK)
			ofLogWarning("ofxParamClient") << "sync failed (" << curl_easy_strerror(res) << ")";

		curl_easy_cleanup(curl);
	}else{
		ofLogError("ofxParamClient") << "Could not init cURL";
	}
	return ret;
}

/////////////////////////////////////////////////

ofxParamClient::ofxParamClient(){
	bSynced = false;

}

ofxParamClient::~ofxParamClient(){

}

void ofxParamClient::setup(string ip, int oscPLocal, int oscPRemote, int httpP){
	serverIp = ip;
	oscPortLocal = oscPLocal;
	oscPortServer = oscPRemote;
	httpPort = httpP;
	sync();
/*
	ofEvents().update.newListener([&](ofEventArgs&){
		if(isSynced()){

		};
	});
*/
}

void ofxParamClient::sync(){

	//ofRemoveListener(getParams().parameterChangedE(), this, &ofxParamClient::onParamChanged);
	//

	bSynced = false;
	std::stringstream url;
	url << serverIp << ":" << httpPort;
	std::string res = curlRead(url.str());
	if(res.size() == 0)
		return;

	bSynced = true;
	params = syncToJson(res, getParams());
	//paramGroup = static_cast<ofParameterGroup*>(params[0]);

	//ofAddListener(getParams().parameterChangedE(), this, &ofxParamClient::onParamChanged);

	paramSync.setup(getParams(), oscPortLocal, serverIp, oscPortServer);
}

void ofxParamClient::update(){
	if(isSynced()){
		paramSync.update();
	}
}

bool ofxParamClient::isSynced(){
	return bSynced;
}

ofParameterGroup &ofxParamClient::getParams(){
	return paramGroup;
}

void ofxParamClient::onParamChanged(ofAbstractParameter &param){
	std::string path = getPath(param);
}
