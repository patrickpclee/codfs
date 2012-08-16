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

	MongoDB (string host, uint32_t port, string database);

	/**
	 * @brief	Connect to the Data Base
	 */
	void connect(bool writeConcern = true);

	/**
	 * @brief	Set the Collection
	 *
	 * @param	collection	Collection to Use
	 */
	void setCollection(string collection);

	vector<mongo::BSONObj> read (mongo::Query queryObject);
	mongo::BSONObj readOne (mongo::Query queryObject);
	void insert (mongo::BSONObj insertObject);
	void update (mongo::Query queryObject, mongo::BSONObj updateObject);
	void push (mongo::Query queryObject, mongo::BSONObj updateObject);
	void remove (mongo::Query queryObject);
	mongo::BSONObj findAndModify (mongo::BSONObj queryObject, mongo::BSONObj updateObject);

private:
	string _user;
	string _password;
	string _host;
	uint32_t _port;
	string _database;
	string _collection;

	mongo::DBClientConnection _connection;
};

#endif
