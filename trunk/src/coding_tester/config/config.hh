#ifndef __CONFIG_HH__
#define __CONFIG_HH__
#include "tinyxml.hh"

#define DEFAULT_CONFIG_PATH	"config.xml"

class ConfigLayer{
	public:
		ConfigLayer();
		ConfigLayer(const char* configPath);

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
		void init(const char* configPath);
		TiXmlElement* advanceToElement(const char* propertyTree);

		bool inited_;
		TiXmlDocument* doc_;
		TiXmlHandle* configHandle_;
		TiXmlElement* configElement_;

};
#endif
