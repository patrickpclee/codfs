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
	 * @param	clientId	Client ID
	 * @param	path	Path of the File
	 */
	uint32_t createFile (uint32_t clientId, const string &path);

	/**
	 * @brief	Delete a File with Client ID and Path
	 *
	 * @param	clientId	Client ID
	 * @param	path	Path of the File
	 */
	void deleteFile (uint32_t clientId, const string &path);

	/**
	 * @brief	Rename a File with Client ID and Path
	 *
	 * @param	clientId	Client ID
	 * @param	path	File Path
	 * @param	newPath	New File Path
	 */
	void renameFile (uint32_t clientId, const string &path, const string &tmpPath);

	/**
	 * @brief	Open a File
	 *
	 * @param	clientId	Client ID
	 * @param	path	Path of the File
	 */
	uint32_t openFile (uint32_t clientId, const string &path);

	/**
	 * @brief	List Folder
	 *
	 * @param	clientId	Client ID
	 * @param	path	Path to the Folder
	 */
	vector<FileMetaData> listFolder (uint32_t clientId, const string &inpath);

};
#endif
