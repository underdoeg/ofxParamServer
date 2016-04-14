#include "ofxParamClient.h"



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

}

ofxParamClient::~ofxParamClient(){

}

void ofxParamClient::setup(string ip, int oscP, int httpP){
	serverIp = ip;
	oscPort = oscP;
	httpPort = httpP;
	sync();
}

void ofxParamClient::sync(){
	std::stringstream url;
	url << serverIp << ":" << httpPort;
	std::string res = curlRead(url.str());
	std::vector<ofAbstractParameter*> params = syncToJson(res, getParams());
}

ofParameterGroup &ofxParamClient::getParams(){
	return paramGroup;
}
