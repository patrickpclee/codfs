#ifndef __CONFIG_HH__
#define __CONFIG_HH__
#include "tinyxml.hh"
#include "../common/define.hh"


class ConfigLayer{
	public:
		ConfigLayer();
		ConfigLayer(const char* configPath, const char* commonConfigPath=DEFAUTT_COMMON_CONFIG);

		const char* getConfigString(const char* propertyTree);
		long long getConfigLong(const char* propertyTree);
		int getConfigInt(const char* propertyTree);
		bool getConfigBool(const char* propertyTree);

		void setConfigString(const char* propertyTree, const char* text);
		void setConfigLong(const char* propertyTree, long long longValue);
		void setConfigInt(const char* propertyTree, int intValue);
		void setConfigBool(const char* propertyTree, bool boolValue);
		
		~ConfigLayer();
	private:
		void init(const char* configPath, const char* commonConfigPath);
		TiXmlElement* advanceToElement(const char* propertyTree);

		bool inited_;
		bool _commonInited;

		TiXmlDocument* doc_;
		TiXmlHandle* configHandle_;
		///Root node of Config
		TiXmlElement* configElement_;

		TiXmlDocument* _commonDoc;
		TiXmlHandle* _commonConfigHandle;
		///Root node of Common Config
		TiXmlElement* _commonConfigElement;
};
#endif
