#ifndef __LRU_CACHE_HH__
#define __LRU_CACHE_HH__

#include <list>
//#include <pair>
#include <stdint.h>
#include <stdexcept>
#include <unordered_map>
#include <thread>
#include "../common/debug.hh"

using namespace std;

template<typename keyType, typename valueType> class LruCache {
public:
	typedef unordered_map<keyType,
			pair<valueType, typename list<keyType>::iterator> > valueMapType;

	LruCache(uint32_t size) :
			_size(size) {

	}
	;

	valueType& get(const keyType &key) {
		std::lock_guard<std::mutex> lk(_cacheMutex);
		typename valueMapType::iterator it = _valueMap.find(key);
		if (it == _valueMap.end())
			throw out_of_range("Element Not Found");
		_accessTimeList.splice(_accessTimeList.end(), _accessTimeList,
				((*it).second.second));
		return (*it).second.first;
	}
	;

	typename valueMapType::iterator find(const keyType &key) {
		std::lock_guard<std::mutex> lk(_cacheMutex);
		return _valueMap.find(key);
	}

	void insert(const keyType &key, const valueType &value) {
		std::lock_guard<std::mutex> lk(_cacheMutex);

		if (_valueMap.count(key) != 0) {
			typename valueMapType::iterator it = _valueMap.find(key);
			_accessTimeList.splice(_accessTimeList.end(), _accessTimeList,
					(*it).second.second);
			(*it).second.first = value;
			return;
		}

		if (_valueMap.size() == _size)
			pop_back();

		typename list<keyType>::iterator it = _accessTimeList.insert(
				_accessTimeList.end(), key);
		_valueMap.insert(make_pair(key, make_pair(value, it)));
	}
	;

	uint32_t count(const keyType &key) {
		std::lock_guard<std::mutex> lk(_cacheMutex);
		return _valueMap.count(key);
	}

	~LruCache() {
		std::lock_guard<std::mutex> lk(_cacheMutex);
		_valueMap.clear();
		_accessTimeList.clear();
	}
	;

	void remove(const keyType &key) {
		std::lock_guard<std::mutex> lk(_cacheMutex);
		typename valueMapType::iterator it = _valueMap.find(key);
        _valueMap.erase(it);
        _accessTimeList.remove(key);
	}

	void pop_back() {
		typename valueMapType::iterator it = _valueMap.find(
				_accessTimeList.front());
		_valueMap.erase(it);
		_accessTimeList.pop_front();
	}
	;

	uint32_t _size;
	list<keyType> _accessTimeList;
	valueMapType _valueMap;
	mutex _cacheMutex;
};

#endif
