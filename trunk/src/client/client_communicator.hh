#ifndef __CLIENT_COMMUNICATOR_HH__
#define __CLIENT_COMMUNICATOR_HH__

#include "../communicator/communicator.hh"

class ClientCommunicator : public Communicator {
public:
	void display ();
	void osdIdListRequest ();
	void objectMessageHandler ();
private:
};
#endif
