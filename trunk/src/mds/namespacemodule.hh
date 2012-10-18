#ifndef __NAMESPACE_MODULE_HH__
#define __NAMESPACE_MODULE_HH__

#include "../common/metadata.hh"

#include <stdint.h>
#include <string>
#include <vector>

class NameSpaceModule {
public:
	/**
	 * @brief	Default Constructor, Read Setting From Config
	 */
	NameSpaceModule ();

	/**
	 * @brief	Create a File with Client ID and Path
	 *
	 * @param	clientId	ID of the Client
	 * @param	path	Path of the File
	 */
	uint32_t createFile (uint32_t clientId, const string &path);

//	/**
//	 * @brief	Delete a File with Client ID and Path
//	 *
//	 * @param	clientId	ID of the Client
//	 * @param	path	Path of the File
//	 */
//	void deleteFile (string path, uint32_t clientId);

	/**
	 * @brief	Open a File
	 *
	 * @param	clientId	ID of the Client
	 * @param	path	Path of the File
	 */
	uint32_t openFile (uint32_t clientId, const string &path);

	/**
	 * @brief	List Folder
	 *
	 * @param	clientId	ID of the Client
	 * @param	path	Path to the Folder
	 */
	vector<FileMetaData> listFolder (uint32_t clientId, const string &inpath);

	/**
	 * @brief	Delete File
	 *
	 * @param	clientId	ID of the Client
	 * @param	path	File Path
	 */
	void deleteFile (uint32_t clientId, const string& path);
private:
	/**
	 * @brief	Covert Path to Real One
	 *
	 * @param	path	Path
	 */
	string convertPath (const string &path);

	/// Base Path of the Namespace Tree
	string _basePath;
};
#endif
