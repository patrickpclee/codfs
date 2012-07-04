#include "mongodb.hh"

MongoDB::MongoDB (char* host){
	_host = (char*) malloc (strlen(host)+1);
	strncpy(_host,host,strlen(host));
}

uint32_t MongoDB::connect(){
	_connection.connect(_host);
}


