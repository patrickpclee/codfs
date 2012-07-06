#include "mongodb.hh"

MongoDB::MongoDB (string host){
	_host = host;
}

uint32_t MongoDB::connect(){
	_connection.connect(_host);
}

void MongoDB::read(string table, uint64_t id, vector<uint64_t>data){
	string targetTable = COLLECTION + (string)"." + table;
	auto_ptr<DBClientCursor> cursor =
		_connection.query(targetTable, QUERY( "id" << (long long)id ) );
	while( cursor->more() ) {
		BSONObj p = cursor->next();
		cout << p.getStringField("name") << endl;
	}

}
