#ifndef __MONGO_DB_HH__
#define __MONGO_DB_HH__

//#define COLLECTION "ncvfs"

//#include "mongo/db/jsobj.h"
#include "mongo/client/dbclient.h"


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
	void connect(bool writeConcern = true);

	vector<mongo::BSONObj> read(string collection, mongo::Query queryObject);
	void insert (string collection, mongo::BSONObj insertObject);
	void update (string collection, mongo::Query queryObject, mongo::BSONObj updateObject);
	void remove (string collection, mongo::Query queryObject);
	mongo::BSONObj findAndModify (string collection, mongo::BSONObj queryObject, mongo::BSONObj updateObject);

private:
	string _user;
	string _password;
	string _host;
	string _database;

	mongo::DBClientConnection _connection;
};

#endif
