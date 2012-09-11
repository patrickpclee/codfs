/*
 * concurrentmap.hh
 */

#ifndef CONCURRENTMAP_HH_
#define CONCURRENTMAP_HH_

#include <thread>

template<class K, class V, class Compare = std::less<K>,
		class Allocator = std::allocator<std::pair<const K, V> > >
class ConcurrentMap {
private:
	std::mutex _m;

public:
	// allow users to access underlying _map if they really need to
	std::map<K, V, Compare, Allocator> _map;

	void set(K key, V value) {
		std::lock_guard<std::mutex> lk(this->_m);
		this->_map[key] = value;
	}

	V & get(K key) {
		std::lock_guard<std::mutex> lk(this->_m);
		return this->_map[key];
	}

	bool empty() {
		std::lock_guard<std::mutex> lk(this->_m);
		return this->_map.empty();
	}

	bool count(K key) {
		std::lock_guard<std::mutex> lk(this->_m);
		return (bool)this->_map.count(key);
	}

	void erase(K key) {
		std::lock_guard<std::mutex> lk(this->_m);
		_map.erase(key);
	}

	V pop (K key) {
		std::lock_guard<std::mutex> lk(this->_m);
		V value = _map[key];
		_map.erase(key);
		return value;
	}

	void increment (K key) {
		std::lock_guard<std::mutex> lk(this->_m);
		_map[key]++;
	}

	void decrement (K key) {
		std::lock_guard<std::mutex> lk(this->_m);
		_map[key]--;
	}

};

#endif /* CONCURRENTMAP_HH_ */
