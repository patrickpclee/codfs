#ifndef __CLIENT_COMMUNICATOR_HH__
#define __CLIENT_COMMUNICATOR_HH__

class ClientCommunicator : public Communicator {
public:
	void display ();
	void osdIdListRequest ();
	void objectMessageHandler ();
private:
};
#endif
