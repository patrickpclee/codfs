#include "mongodb.hh"

#include "../common/debug.hh"

#include "../config/config.hh"

using namespace mongo;

extern ConfigLayer* configLayer;

MongoDB::MongoDB ()
{
	// TODO: Handle Exception 
	_host = configLayer->getConfigString("MetaData>MongoDB>Host");
	_database = configLayer->getConfigString("MetaData>MongoDB>Database");
	_user = configLayer->getConfigString("MetaData>MongoDB>User");
	_password = configLayer->getConfigString("MetaData>MongoDB>Password");
}

MongoDB::MongoDB (string host, string database) :
	_host(host), _database(database)
{
}

/**
 * @brief	Connect to the Data Base
 */
void MongoDB::connect (bool writeConcern)
{
	_connection.connect(_host);
	if(writeConcern)
		_connection.setWriteConcern(W_NORMAL);
	string errMsg;
	_connection.auth(_database, _user, _password, errMsg, false);
	
	return ;
}

vector<BSONObj> MongoDB::read (string collection, Query queryObject)
{
	unique_ptr<DBClientCursor> cursor =
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
	_connection.update(_database + "." + collection, queryObject, updateObject, true);

	return ;
}

void MongoDB::remove (string collection, Query queryObject)
{
	_connection.remove(_database + "." + collection, queryObject);

	return ;

}

void MongoDB::push (string collection, Query queryObject, BSONObj pushObject)
{
	_connection.update(_database + "." + collection, queryObject, pushObject, true);
}

BSONObj MongoDB::findAndModify (string collection, BSONObj queryObject, BSONObj updateObject)
{
	//BSONObj commandObject = BSON ("findandmodify" << collection << modifyObject);
//	BSONObj commandObject = BSON ("findandmodify" << "File Meta Data" << "query" << BSON ("id" << "config") << "update" << BSON ("$inc" << BSON ("fileId" << 1)));
//	debug("%s\n",commandObject.jsonString().c_str());
	BSONObj result;
	_connection.runCommand(_database, BSON ("findandmodify" << collection << "query" << queryObject << "update" << updateObject), result);
	//_connection.runCommand(_database,BSON ("findandmodify" << "File Meta Data" << "query" << BSON ("id" << "config") << "update" << BSON ("$inc" << BSON ("fileId" << 1))),result);

	return result.getObjectField("value");
};
