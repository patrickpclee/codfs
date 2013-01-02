/*
 * concurrentqueue.hh
 * http://www.drdobbs.com/parallel/writing-a-generalized-concurrent-queue/211601363
 */

#ifndef LOWLOCKQUEUE_HH_
#define LOWLOCKQUEUE_HH_

#include <thread>
#include "../common/define.hh"

using namespace std;

template<typename T>
struct LowLockQueue {
private:
	struct Node {
		Node(T* val) :
				value(val), next(NULL) {
		}
		T* value;
		atomic<Node*> next;
		char pad[CACHE_LINE_SIZE - sizeof(T*) - sizeof(atomic<Node*> )];
	};
	char pad0[CACHE_LINE_SIZE];

// for one consumer at a time
	Node* first;

	char pad1[CACHE_LINE_SIZE - sizeof(Node*)];

// shared among consumers
	atomic<bool> consumerLock;

	char pad2[CACHE_LINE_SIZE - sizeof(atomic<bool> )];

// for one producer at a time
	Node* last;

	char pad3[CACHE_LINE_SIZE - sizeof(Node*)];

// shared among producers
	atomic<bool> producerLock;

	char pad4[CACHE_LINE_SIZE - sizeof(atomic<bool> )];

public:
	LowLockQueue() {
		first = last = new Node(NULL);
		producerLock = consumerLock = false;
	}
	~LowLockQueue() {
		while (first != NULL) { // release the list
			Node* tmp = first;
			first = tmp->next;
			delete tmp->value; // no-op if null
			delete tmp;
		}
	}

	void push(const T& t) {
		Node* tmp = new Node(new T(t));
		while (producerLock.exchange(true)) {
		} // acquire exclusivity
		last->next = tmp; // publish to consumers
		last = tmp; // swing last forward
		producerLock = false; // release exclusivity
	}

	bool pop(T& result) {
		while (consumerLock.exchange(true)) {
		} // acquire exclusivity
		Node* theFirst = first;
		Node* theNext = first->next;
		if (theNext != NULL) { // if queue is nonempty
			T* val = theNext->value; // take it out
			theNext->value = NULL; // of the Node
			first = theNext; // swing first forward
			consumerLock = false; // release exclusivity
			result = *val; // now copy it back
			delete val; // clean up the value
			delete theFirst; // and the old dummy
			return true; // and report success
		}

		consumerLock = false; // release exclusivity
		return false; // report queue was empty
	}
};

#endif /* CONCURRENTQUEUE_HH_ */
