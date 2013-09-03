#include "mongodb.hh"

#include "../common/debug.hh"

#include "../config/config.hh"

using namespace mongo;

class DbAuthHook : public DBConnectionHook
{
	public:
		DbAuthHook(string database, string user, string password, bool writeConcern)
		{
			_database = database;
			_user = user;
			_password = password;
			_writeConcern = writeConcern;
		}
    void onCreate(DBClientBase *conn)
    {
        //authenticate
        string err;
        try
        {
            if(conn->auth(_database, _user, _password, err, false) == false)
            {
                cout << "[TERM]Unable to authenticate to db" << endl;
                exit(-1);
            }  
			if(_writeConcern)
				conn->setWriteConcern(W_NORMAL);
        }
        catch(std::exception &ex)
        {
            cout << "[TERM]Error while authenticating with db" << endl;
            exit(-1);
        }
    }
    void onHandedOut(DBClientBase *conn)
    {
    }
    void onDestroy(DBClientBase *conn)
    {
    }
	private:
		string _database;
		string _user;
		string _password;
		bool _writeConcern;

};

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

	DbAuthHook *dbHook = new DbAuthHook(_database, _user, _password, true);
	pool.addHook(dbHook);
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
	//_connection->connect(_host + ":" + to_string(_port));
	//if(writeConcern)
	//	_connection->setWriteConcern(W_NORMAL);
	//string errMsg;
	//_connection->auth(_database, _user, _password, errMsg, false);
	
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
vector<BSONObj> MongoDB::read (Query querySegment)
{

	ScopedDbConnection* _conn = ScopedDbConnection::getScopedDbConnection(_host);
	DBClientBase* _connection = _conn->get();
	unique_ptr<DBClientCursor> cursor =
		_connection->query(_database +"." + _collection, querySegment);
	vector<BSONObj> result;
	BSONObj tempObj;
	while( cursor->more() ) {
		tempObj = cursor->next().copy();
		result.push_back(tempObj);
	}
	_conn->done();

	return result;
}

/**
 * @brief	Read One Result from the MongoDB
 */
BSONObj MongoDB::readOne (Query querySegment)
{
	ScopedDbConnection* _conn = ScopedDbConnection::getScopedDbConnection(_host);
	DBClientBase* _connection = _conn->get();
	BSONObj result = _connection->findOne(_database + "." + _collection, querySegment);
	_conn->done();

	return result.copy();
}

/**
 * @brief	Insert to the MongoDB
 */
void MongoDB::insert (BSONObj insertSegment)
{
	ScopedDbConnection* _conn = ScopedDbConnection::getScopedDbConnection(_host);
	DBClientBase* _connection = _conn->get();
	_connection->insert(_database + "." + _collection, insertSegment);
	_conn->done();

	pool.flush();
	return ;
}

/**
 * @brief	Update Record in MongoDB
 */
void MongoDB::update (Query querySegment, BSONObj updateSegment)
{
	ScopedDbConnection* _conn = ScopedDbConnection::getScopedDbConnection(_host);
	DBClientBase* _connection = _conn->get();
	_connection->update(_database + "." + _collection, querySegment, updateSegment, true);
	_conn->done();

	pool.flush();
	return ;
}

/**
 * @brief	Push Value to a Field of a Record
 */
void MongoDB::push (Query querySegment, BSONObj pushSegment)
{
	ScopedDbConnection* _conn = ScopedDbConnection::getScopedDbConnection(_host);
	DBClientBase* _connection = _conn->get();
	_connection->update(_database + "." + _collection, querySegment, pushSegment, true);
	_conn->done();
	pool.flush();
}

BSONObj MongoDB::findAndModify (BSONObj querySegment, BSONObj updateSegment)
{
	ScopedDbConnection* _conn = ScopedDbConnection::getScopedDbConnection(_host);
	DBClientBase* _connection = _conn->get();
	BSONObj result;
	_connection->runCommand(_database, BSON ("findandmodify" << _collection << "query" << querySegment << "update" << updateSegment), result);
	_conn->done();

	pool.flush();
	return result.getObjectField("value").copy();
};

void MongoDB::removeField (Query querySegment, string field)
{
	ScopedDbConnection* _conn = ScopedDbConnection::getScopedDbConnection(_host);
	DBClientBase* _connection = _conn->get();
	BSONObj unsetSegment = BSON ("$unset" << BSON (field << "1"));
	_connection->update(_database + "." + _collection, querySegment, unsetSegment, true);
	_conn->done();

	pool.flush();
}

void MongoDB::remove (Query querySegment)
{
	ScopedDbConnection* _conn = ScopedDbConnection::getScopedDbConnection(_host);
	DBClientBase* _connection = _conn->get();
	_connection->remove(_database + "." + _collection, querySegment);
	_conn->done();

	pool.flush();
	return ;
}

