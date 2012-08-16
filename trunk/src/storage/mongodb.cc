#include "mongodb.hh"

#include "../common/debug.hh"

#include "../config/config.hh"

using namespace mongo;

extern ConfigLayer* configLayer;

/**
 * @brief	Default Constructor, Read Settings from Config
 */
MongoDB::MongoDB ()
{
	// TODO: Handle Exception 
	_host = configLayer->getConfigString("MetaData>MongoDB>Host");
	if(configLayer->getConfigInt("MetaData>MongoDB>Port") == -1)
		_port = 27017;
	else
		_port = configLayer->getConfigInt("MetaData>MongoDB>Port");
	_database = configLayer->getConfigString("MetaData>MongoDB>Database");
	_user = configLayer->getConfigString("MetaData>MongoDB>User");
	_password = configLayer->getConfigString("MetaData>MongoDB>Password");
}

/**
 * @brief	Constructor, Read Settings from Parameters
 */
MongoDB::MongoDB (string host, uint32_t port, string database) :
	_host(host), _port(port), _database(database)
{
}

/**
 * @brief	Connect to the Data Base
 */
void MongoDB::connect (bool writeConcern)
{
	_connection.connect(_host + ":" + to_string(_port));
	if(writeConcern)
		_connection.setWriteConcern(W_NORMAL);
	string errMsg;
	_connection.auth(_database, _user, _password, errMsg, false);
	
	return ;
}

/**
 * @brief	Set the Collection
 */
void MongoDB::setCollection(string collection)
{
	_collection = collection;

	return ;
}

/**
 * @brief	Read from the MongoDB
 */
vector<BSONObj> MongoDB::read (Query queryObject)
{
	unique_ptr<DBClientCursor> cursor =
		_connection.query(_database +"." + _collection, queryObject);
	vector<BSONObj> result;
	BSONObj tempObj;
	while( cursor->more() ) {
		tempObj = cursor->next();
		result.push_back(tempObj);
	}
	
	return result;
}

/**
 * @brief	Read One Result from the MongoDB
 */
BSONObj MongoDB::readOne (Query queryObject)
{
	return read(queryObject).at(0);
}

/**
 * @brief	Insert to the MongoDB
 */
void MongoDB::insert (BSONObj insertObject)
{
	_connection.insert(_database + "." + _collection, insertObject);

	return ;
}

/**
 * @brief	Update Record in MongoDB
 */
void MongoDB::update (Query queryObject, BSONObj updateObject)
{
	_connection.update(_database + "." + _collection, queryObject, updateObject, true);

	return ;
}

/**
 * @brief	Push Value to a Field of a Record
 */
void MongoDB::push (Query queryObject, BSONObj pushObject)
{
	_connection.update(_database + "." + _collection, queryObject, pushObject, true);
}

BSONObj MongoDB::findAndModify (BSONObj queryObject, BSONObj updateObject)
{
	BSONObj result;
	_connection.runCommand(_database, BSON ("findandmodify" << _collection << "query" << queryObject << "update" << updateObject), result);

	return result.getObjectField("value");
};

void MongoDB::remove (Query queryObject)
{
	_connection.remove(_database + "." + _collection, queryObject);

	return ;

}
