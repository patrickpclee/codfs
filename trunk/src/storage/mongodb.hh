#ifndef __MONGO_DB_HH__
#define __MONGO_DB_HH__

#define COLLECTION "ncvfs"
#include "mongo/client/dbclient.h"
using namespace mongo;

#include <stdint.h>

class MongoDB {
public:
	MongoDB (string host);
	uint32_t connect();
	void read(string table, uint64_t id, vector<uint64_t> data);
private:
	string _host;
	DBClientConnection _connection;
};

#endif
