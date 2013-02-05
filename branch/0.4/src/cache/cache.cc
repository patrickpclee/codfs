#include <thread>
#include <sstream>
#include "cache.hh"

using namespace std;

template<class T>
T* Cache<T>::read(uint64_t id){
	std::string key = to_string(id);
	if (!_cacheMap.count(key)) {
		return NULL;
	}	
	return _cacheMap[key];
	/*
	   mm::cache_map<std::string, T*>::iterator _it = _cacheMap.find(key);
	   if (_it != _cacheMap.end()) {
	   return _it->second;
	   }
	   return NULL;
	 */
	//TODO cache missing
}

template<class T>
void Cache<T>::write(uint64_t id, T* data){
	_cacheMap.insert(to_string(id),data);
}

template<class T>
void Cache<T>::deleteEntry(uint64_t id){
	std::string key = to_string(id);
	if (!_cacheMap.count(key)) {
		return;
	}
	_cacheMap.erase (key);
	/*
	mm::cache_map<std:string, T*>::iterator _it = _cacheMap.find(key);
	if (_it != _cacheMap.end()) {
		_cacheMap.erase(_it->first);
		return;
	}
	*/
	//TODO cache missing
}

