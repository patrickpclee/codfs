#include "mongodb.hh"

MongoDB::MongoDB (string host){
	_host = host;
}

uint32_t MongoDB::connect(){
	_connection.connect(_host);
}


