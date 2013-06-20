#ifndef RINGBUFFER_HH_
#define RINGBUFFER_HH_

#include <thread>
#include <malloc.h>
#include "../common/define.hh"

using namespace std;

template<typename T>
class RingBuffer {
	public:
		RingBuffer (uint64_t size) :
			_size(size + 1),
			_head(0),
			_tail(0)
		{
			//_buffer = (T*)memalign(getpagesize(), sizeof(T) * size);
			_buffer = (T*)malloc(sizeof(T) * _size);
		}

		void push(T ele) {
			unique_lock<mutex> lock(_mutex);

			_full.wait(lock, [this]() {return !RingBuffer<T>::checkFull();});

			_buffer[_head] = ele;
			_head = advance(_head);

			_empty.notify_one();
		}


		T pop() {
			unique_lock<mutex> lock(_mutex);

			_empty.wait(lock, [this]() {return !RingBuffer<T>::checkEmpty();});

			T tempEle = _buffer[_tail];
			_tail = advance(_tail);

			_full.notify_one();

			return tempEle;
		}
	private:
		inline uint64_t advance(uint64_t pos) {
			return (pos + 1) % _size;
		}

		bool checkEmpty() {
			return (_head == _tail);
		}

		bool checkFull() {
			return (advance(_head) == _tail);
		}
		uint64_t _size;
		uint64_t _head;
		uint64_t _tail;
		mutex _mutex;
		condition_variable _empty;
		condition_variable _full;
		T *_buffer;
};

#endif
