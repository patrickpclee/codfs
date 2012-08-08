#ifndef __MONGO_DB_HH__
#define __MONGO_DB_HH__

//#define COLLECTION "ncvfs"

#include "mongo/client/dbclient.h"

using namespace mongo;

#include <string>
#include <vector>
using namespace std;

#include <stdint.h>

class MongoDB {
public:

	MongoDB ();

	MongoDB (string host, string database);

	/**
	 * @brief	Connect to the Data Base
	 */
	void connect();

	vector<BSONObj> read(string collection, Query queryObject);
	void insert (string collection, BSONObj insertObject);
	void update (string collection, Query queryObject, BSONObj updateObject);

private:
	string _user;
	string _password;
	string _host;
	string _database;

	DBClientConnection _connection;
};

#endif
