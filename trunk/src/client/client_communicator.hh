#ifndef __CLIENT_COMMUNICATOR_HH__
#define __CLIENT_COMMUNICATOR_HH__

#include <vector>

#include "../communicator/communicator.hh"
#include "../common/metadata.hh"

class ClientCommunicator : public Communicator {
public:
	void display ();
	void osdIdListRequest ();
	void objectMessageHandler ();

	vector<FileMetaData> listFolderData (uint32_t clientId, string path);
private:
};
#endif
