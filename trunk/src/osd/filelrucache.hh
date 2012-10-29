#ifndef FILELRUCACHE_HH_
#define FILELRUCACHE_HH_

#include "../cache/lru_cache.hh"

template<typename keyType, typename valueType> class FileLruCache: public LruCache<
		keyType, valueType> {
	typedef unordered_map<keyType,
			pair<valueType, typename list<keyType>::iterator> > valueMapType;
public:
	FileLruCache(uint32_t size) : LruCache<keyType, valueType> (size) {

	}

	~FileLruCache() {

	}
private:
	void pop_back() {
		typename valueMapType::iterator it = this->_valueMap.find(
				this->_accessTimeList.front());
		this->_valueMap.erase(it);
		fclose(this->_accessTimeList.front());
		this->_accessTimeList.pop_front();
	}
	;
};

#endif /* FILELRUCACHE_HH_ */
