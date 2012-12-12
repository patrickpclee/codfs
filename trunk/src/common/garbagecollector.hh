#ifndef GARBAGECOLLECTOR_HH_
#define GARBAGECOLLECTOR_HH_

#include <list>
#include "../protocol/message.hh"

using namespace std;

class GarbageCollector {
public:

	/**
	 * static method for Singleton implementation
	 * @return reference to instance of singleton segment
	 */

	static GarbageCollector& getInstance() {
		static GarbageCollector instance; // Guaranteed to be destroyed
		// Instantiated on first use
		return instance;
	}

	/**
	 * Constructor
	 */

	GarbageCollector();

	/**
	 * Destructor
	 */

	~GarbageCollector();

	/**
	 * Obtain a piece of memory
	 * @param ptr Pointer to segment to delete
	 */

	void addToDeleteList(Message* ptr);

	/**
	 * Start the garbage collector
	 */

	void start();

private:
	// Dont forget to declare these two. You want to make sure they
	// are unaccessable otherwise you may accidently get copies of
	// your singleton appearing.

	GarbageCollector(GarbageCollector const&); // Don't Implement
	void operator=(GarbageCollector const&); // Don't implement

	list<Message*> _pendingDeleteList;
	mutex _pendingDeleteListMutex;
};

#endif /* GARBAGECOLLECTOR_HH_ */
