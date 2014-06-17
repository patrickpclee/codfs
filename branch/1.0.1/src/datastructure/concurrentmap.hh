/*
 * concurrentmap.hh
 */

#ifndef CONCURRENTMAP_HH_
#define CONCURRENTMAP_HH_

#include <map>
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

	void clear() {
		std::lock_guard<std::mutex> lk(this->_m);
		_map.clear();
	}

	V pop (K key) {
		std::lock_guard<std::mutex> lk(this->_m);
		V value = _map[key];
		_map.erase(key);
		return value;
	}

	V increment (K key) {
		std::lock_guard<std::mutex> lk(this->_m);
		return ++_map[key];
	}

	V decrement (K key) {
		std::lock_guard<std::mutex> lk(this->_m);
		return --_map[key];
	}

    bool init(K key, V value) {
        std::lock_guard<std::mutex> lk(this->_m);
        if (this->_map.count(key) == 0) {
            this->_map[key] = value;
            return true;
        } else {
            return false;
        }
    }

    size_t size() {
		std::lock_guard<std::mutex> lk(this->_m);
        return _map.size();
    }

};

#endif /* CONCURRENTMAP_HH_ */
