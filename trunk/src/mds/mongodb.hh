#ifndef __MONGO_DB_HH__
#define __MONGO_DB_HH__

#include "mongo/client/dbclient.h"
using namespace mongo;

#include <stdint.h>

class MongoDB {
public:
	MongoDB (char* host);
	uint32_t connect();
private:
	char* _host;
	DBClientConnection _connection;
};

#endif
