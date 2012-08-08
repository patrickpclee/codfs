#include "mongodb.hh"

#include "../config/config.hh"

extern ConfigLayer* configLayer;

MongoDB::MongoDB ()
{
	// TODO: Handle Exception 
	_host = configLayer->getConfigString("MetaData>MongoDB>Host");
	_database = configLayer->getConfigString("MetaData>MongoDB>Database");
}

MongoDB::MongoDB (string host, string database) :
	_host(host), _database(database)
{
}

/**
 * @brief	Connect to the Data Base
 */
void MongoDB::connect ()
{
	_connection.connect(_host);
	_connection.setWriteConcern(W_NORMAL);
	
	return ;
}

vector<BSONObj> MongoDB::read (string collection, Query queryObject)
{
	auto_ptr<DBClientCursor> cursor =
		_connection.query(_database +"." + collection, queryObject);
	vector<BSONObj> result;
	BSONObj tempObj;
	while( cursor->more() ) {
		tempObj = cursor->next();
		result.push_back(tempObj);
	}
	
	return result;
}

void MongoDB::insert (string collection, BSONObj insertObject)
{
	_connection.insert(_database + "." + collection, insertObject);

	return ;
}

void MongoDB::update (string collection, Query queryObject, BSONObj updateObject)
{
	_connection.update(_database + "." + collection, queryObject, updateObject);

	return ;
}
