/*
 * concurrentqueue.hh
 * http://www.justsoftwaresolutions.co.uk/threading/implementing-a-thread-safe-queue-using-condition-variables.html
 * Modified to use c++0x std::thread instead of thread
 */

#ifndef CONCURRENTQUEUE_HH_
#define CONCURRENTQUEUE_HH_

#include <thread>

using namespace std;

template<typename Data>
class ConcurrentQueue {
private:
	std::queue<Data> the_queue;
	mutable mutex the_mutex;
	condition_variable the_condition_variable;
public:
	void push(Data const& data) {
		unique_lock<mutex> lk(the_mutex);
		the_queue.push(data);
		lk.unlock();
		the_condition_variable.notify_one();
	}

	bool empty() const {
		unique_lock<mutex> lk(the_mutex);
		return the_queue.empty();
	}

	bool try_pop(Data& popped_value) {
		unique_lock<mutex> lk(the_mutex);
		if (the_queue.empty()) {
			return false;
		}

		popped_value = the_queue.front();
		the_queue.pop();
		return true;
	}

	void wait_and_pop(Data& popped_value) {
		unique_lock<mutex> lk(the_mutex);
		while (the_queue.empty()) {
			the_condition_variable.wait(lk);
		}

		popped_value = the_queue.front();
		the_queue.pop();
	}

};

#endif /* CONCURRENTQUEUE_HH_ */
