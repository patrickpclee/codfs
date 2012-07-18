#include "config.hh"
#include <string.h>

ConfigLayer::ConfigLayer(){	
	init(DEFAULT_CONFIG_PATH);
}

ConfigLayer::ConfigLayer(const char* configPath){
	init(configPath);
}

const char* ConfigLayer::getConfigString(const char* propertyTree){
	TiXmlElement* tempElement = advanceToElement(propertyTree);
	if(tempElement == NULL)
		return NULL;
	return tempElement->GetText();	
}

long long ConfigLayer::getConfigLong(const char* propertyTree){
	const char* configStr = getConfigString(propertyTree);
	if(configStr == NULL){
		return -1;
	}
	else return atol(configStr);
}

int ConfigLayer::getConfigInt(const char* propertyTree){
	const char* configStr = getConfigString(propertyTree);
	if(configStr == NULL){
		return -1;
	}
	else return atoi(configStr);
}

bool ConfigLayer::getConfigBool(const char* propertyTree){
	const char* configStr = getConfigString(propertyTree);
	if(configStr == NULL)
		return false;
	else {
		if(strcmp(configStr,"true") == 0)
			return true;
		else
			return false;
	}
}

void ConfigLayer::setConfigString(const char* propertyTree, const char* text){
	TiXmlElement* tempElement = advanceToElement(propertyTree);
	TiXmlNode* oldNode = tempElement->FirstChild();
	TiXmlText newText(text);
	tempElement->ReplaceChild(oldNode,newText);
}

void ConfigLayer::setConfigLong(const char* propertyTree, long long longValue){
	char tempString[1024] = {0};
	sprintf(tempString,"%lld",longValue);
	setConfigString(propertyTree,tempString);
}

void ConfigLayer::setConfigInt(const char* propertyTree, int intValue){
	char tempString[1024] = {0};
	sprintf(tempString,"%d",intValue);
	setConfigString(propertyTree,tempString);
}

void ConfigLayer::setConfigBool(const char* propertyTree, bool boolValue){
	if(boolValue)
	setConfigString(propertyTree,"true");
	else
	setConfigString(propertyTree,"false");
}

void ConfigLayer::init(const char* configPath){
	doc_ = new TiXmlDocument(configPath);
	inited_ = doc_->LoadFile();
	if(!inited_){
		fprintf(stderr,"Failed to load %s\n",configPath);
		return;
	} else {
		fprintf(stderr,"Inited with %s\n",configPath);
	}
	configHandle_ = new TiXmlHandle(doc_);
	configElement_ = configHandle_->FirstChild("NcfsConfig").Element();
	if(configElement_ == NULL){
		fprintf(stderr,"Failed to load2 %s\n",configPath);
		return;
	}
	delete(configHandle_);
	configHandle_ = new TiXmlHandle(configElement_);
}

TiXmlElement* ConfigLayer::advanceToElement(const char* propertyTree){
	char propertyTree_[strlen(propertyTree)+1];
	strcpy(propertyTree_,propertyTree);
	char* namePtr;
	namePtr = strtok(propertyTree_,">");
	TiXmlHandle tempHandle = TiXmlHandle(configElement_);
	TiXmlElement* tempElement = tempHandle.Element();
	while (namePtr != NULL){
		tempHandle = tempHandle.FirstChild(namePtr);
		tempElement = tempHandle.Element();
		if(tempElement == NULL){
			fprintf(stderr,"%s: %s not found\n",propertyTree,namePtr);
			return NULL;
		}
		namePtr = strtok(NULL,">");
	}
	return tempElement;
}

ConfigLayer::~ConfigLayer(){
	doc_->SaveFile();
}
