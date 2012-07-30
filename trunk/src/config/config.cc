#include "config.hh"
#include <string.h>

ConfigLayer::ConfigLayer(){	
	init(DEFAULT_CONFIG_PATH, DEFAUTT_COMMON_CONFIG);
}

ConfigLayer::ConfigLayer(const char* configPath, const char* commonConfigPath){
	init(configPath, commonConfigPath);
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

///TODO: Init with both specific and common Config
///TODO: Throw Execepion
void ConfigLayer::init(const char* configPath, const char* commonConfigPath){
	doc_ = new TiXmlDocument(configPath);
	inited_ = doc_->LoadFile();
	if(!inited_){
		fprintf(stderr,"Failed to load %s\n",configPath);
		return;
	} else {
		fprintf(stderr,"Inited with %s\n",configPath);
	}
	configHandle_ = new TiXmlHandle(doc_);
	configElement_ = configHandle_->FirstChild(XML_ROOT_NODE).Element();
	if(configElement_ == NULL){
		fprintf(stderr,"Failed to load2 %s\n",configPath);
		return;
	}
	delete(configHandle_);
	configHandle_ = new TiXmlHandle(configElement_);

	_commonDoc = new TiXmlDocument(commonConfigPath);
	_commonInited = _commonDoc->LoadFile();
	if(!_commonInited){
		fprintf(stderr,"Failed to load %s\n",commonConfigPath);
		return;
	} else {
		fprintf(stderr,"Inited with %s\n",commonConfigPath);
	}
	_commonConfigHandle = new TiXmlHandle(_commonDoc);
	_commonConfigElement = _commonConfigHandle->FirstChild(XML_ROOT_NODE).Element();
	if(_commonConfigElement == NULL){
		fprintf(stderr,"Failed to load2 %s\n",commonConfigPath);
		return;
	}
	delete(_commonConfigHandle);
	_commonConfigHandle = new TiXmlHandle(_commonConfigElement);
}

///TODO: Use strtok_r() for thread safety
///TODO: Read from Common Config if "Common" Tag is encountered
TiXmlElement* ConfigLayer::advanceToElement(const char* propertyTree){
	char propertyTree_[strlen(propertyTree)+1];
	char* namePtr;
	bool found = false;

	strcpy(propertyTree_,propertyTree);

	namePtr = strtok(propertyTree_,">");
	TiXmlHandle tempHandle = TiXmlHandle(configElement_);
	TiXmlElement* tempElement = tempHandle.Element();
	while (namePtr != NULL){
		tempHandle = tempHandle.FirstChild(namePtr);
		tempElement = tempHandle.Element();
		if(tempElement == NULL){
			fprintf(stderr,"%s: %s Not in Specific Config, Cont. to Common Config\n",propertyTree,namePtr);
			break;
		}
		namePtr = strtok(NULL,">");
	}

	if(tempElement != NULL)
		found = true;
	strcpy(propertyTree_,propertyTree);
	// Search in Common Config
	if(!found){
		namePtr = strtok(propertyTree_,">");
		tempHandle = TiXmlHandle(_commonConfigElement);
		tempElement = tempHandle.Element();
		while (namePtr != NULL){
			tempHandle = tempHandle.FirstChild(namePtr);
			tempElement = tempHandle.Element();
			if(tempElement == NULL){
				fprintf(stderr,"%s: %s Not in Common Config\n",propertyTree,namePtr);
				break;
			}
			namePtr = strtok(NULL,">");
		}
	}

	return tempElement;
}

ConfigLayer::~ConfigLayer(){
	doc_->SaveFile();
	_commonDoc->SaveFile();
}
