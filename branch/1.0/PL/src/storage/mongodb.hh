#ifndef __MONGO_DB_HH__
#define __MONGO_DB_HH__

#include "../common/define.hh"

//#include "mongo/db/jsobj.h"
#include "mongo/client/dbclient.h"


#include <string>
#include <vector>
using namespace std;

#include <stdint.h>

class MongoDB {
public:

	/**
	 * @brief	Default Constructor, Read Settings from Config
	 */
	MongoDB ();

	/**
	 * @brief	Constructor, Read Settings from Parameters
	 *
	 * @param	host	Address of the MongoDB Server
	 * @param	port	Port of the MongoDB Server
	 * @param	database	Database to Use
	 */
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

	/**
	 * @brief	Read from the MongoDB
	 *
	 * @param	querySegment	Query
	 *
	 * @return	List of Result
	 */
	vector<mongo::BSONObj> read (mongo::Query querySegment);

	/**
	 * @brief	Read One Result from the MongoDB
	 *
	 * @param	querySegment	Query
	 *
	 * @return	Result (First if Multiple)
	 */
	mongo::BSONObj readOne (mongo::Query querySegment);

	/**
	 * @brief	Insert to the MongoDB
	 *
	 * @param	insertSegment	BSON to Insert
	 */
	void insert (mongo::BSONObj insertSegment);
	
	/**
	 * @brief	Update Record in MongoDB
	 *
	 * @param	querySegment	Query to Specify Record to Update
	 * @param	updateSegment	Segment to Update
	 */
	void update (mongo::Query querySegment, mongo::BSONObj updateSegment);

	/**
	 * @brief	Push Value to a Field of a Record
	 *
	 * @param	querySegment	Query to Specify Field and Record to Update
	 * @param	pushSegment	Segment to Push
	 */
	void push (mongo::Query querySegment, mongo::BSONObj pushSegment);

	/**
	 * @brief	Find a Record and Update It
	 *
	 * @param	querySegment	Query to Specify Record to Update
	 * @param	updateSegment	Segment to Update
	 *
	 * @return	Result
	 */
	mongo::BSONObj findAndModify (mongo::BSONObj querySegment, mongo::BSONObj updateSegment);

	void removeField (mongo::Query querySegment, string field);

	/**
	 * @brief	Remove a Record
	 *
	 * @param	querySegment	Query to Specify Record to Remove
	 */
	void remove (mongo::Query querySegment);

private:
	/// User
	string _user;

	/// Password
	string _password;

	///	Address of the MongoDB Server
	string _host;

	/// Port of the MongoDB Server
	uint32_t _port;

	/// Database to Use
	string _database;

	/// Collection to Use
	string _collection;

	/// Connection to the MongoDB
	//mongo::DBClientConnection _connection;
	//mongo::ScopedDbConnection _connection;
};

#endif
