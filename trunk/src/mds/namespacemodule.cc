#include "namespacemodule.hh"

#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>

/**
 * @brief	List Folder
 *
 * Simply Perform Readdir
 * @todo	Read File ID
 */
vector<FileMetaData> NameSpaceModule::listFolder (uint32_t clientId, string path)
{
	vector<FileMetaData> folderData;

	FileMetaData tempFileMetaData;
	struct stat tempFileStat;
	string tempFilePath;

	DIR *dir;
	struct dirent *ent;
	dir = opendir (path.c_str());
	if (dir != NULL) {

		/* print all the files and directories within directory */
		while ((ent = readdir (dir)) != NULL) {
//			printf ("%s\n", ent->d_name);
			tempFilePath = path + ent->d_name;
			stat(tempFilePath.c_str(),&tempFileStat);

			tempFileMetaData._path = ent->d_name;
			tempFileMetaData._size = tempFileStat.st_size;
			folderData.push_back(tempFileMetaData);
		}
		closedir (dir);
	} else {
		/* could not open directory */
		perror ("opendir()");
	}
	return folderData;
}


uint32_t NameSpaceModule::createFile(uint32_t clientId, string path)
{
	return 0;
}


uint32_t NameSpaceModule::newFileId(string path)
{
	return 0;
}


uint32_t NameSpaceModule::openFile(uint32_t clientId, string path)
{
	return 0;
}

