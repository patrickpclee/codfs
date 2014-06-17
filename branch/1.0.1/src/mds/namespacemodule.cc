#include "namespacemodule.hh"

#include "../common/debug.hh"
#include "../common/enums.hh"

#include "../config/config.hh"

#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>

extern ConfigLayer* configLayer;

/**
 * @brief	Default Constructor, Read Setting From Config
 */
NameSpaceModule::NameSpaceModule () {
}

/**
 * @brief	List Folder
 *
 * Simply Perform Readdir
 * @todo	Read File ID
 */
vector<FileMetaData> NameSpaceModule::listFolder (uint32_t clientId, const string &inpath)
{
	vector<FileMetaData> folderData;
	string path = inpath;
	FileMetaData tempFileMetaData;
	struct stat tempFileStat;
	string tempFilePath;

	DIR *dir;
	struct dirent *ent;
	dir = opendir (path.c_str());

	if (*path.end() != '/')
		path = path + '/';
	if (dir != NULL) {

		/* print all the files and directories within directory */
		while ((ent = readdir (dir)) != NULL) {
			tempFileMetaData._id = 0;
//			printf ("%s\n", ent->d_name);
			tempFilePath = path + ent->d_name;
//			debug("path: %s\n",tempFilePath.c_str());
			stat(tempFilePath.c_str(),&tempFileStat);

			tempFileMetaData._path = ent->d_name;
			tempFileMetaData._size = tempFileStat.st_size;
			if (S_ISDIR(tempFileStat.st_mode))
				tempFileMetaData._fileType = FOLDER;
			else
				tempFileMetaData._fileType = NORMAL;
//			debug("name: %s size: %d\n",ent->d_name,(int)tempFileStat.st_size);
			folderData.push_back(tempFileMetaData);
		}
		closedir (dir);
	} else {
		/* could not open directory */
		perror ("opendir()");
	}
	return folderData;
}


/**
 * @brief	Create a File with Client ID and Path
 */
uint32_t NameSpaceModule::createFile(uint32_t clientId, const string &path)
{
	return 0;
}


/**
 * @brief	Open a File
 */
uint32_t NameSpaceModule::openFile(uint32_t clientId, const string &path)
{
	return 0;
}

void NameSpaceModule::deleteFile (uint32_t clientId, const string &path)
{
	return ;	
}


void NameSpaceModule::renameFile (uint32_t clientId, const string &path, const string &newPath)
{
	return ;
}

