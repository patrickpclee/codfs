#ifndef FILELRUCACHE_HH_
#define FILELRUCACHE_HH_

#include "../cache/lru_cache.hh"
#include "../common/debug.hh"

template<typename keyType, typename valueType> class FileLruCache: public LruCache<
		keyType, valueType> {
	typedef unordered_map<keyType,
			pair<valueType, typename list<keyType>::iterator> > valueMapType;
public:
	FileLruCache(uint32_t size) : LruCache<keyType, valueType> (size) {

	}

	~FileLruCache() {

	}

	void insert(const keyType &key, const valueType &value) {
		std::lock_guard<std::mutex> lk(this->_cacheMutex);

		if (this->_valueMap.count(key) != 0) {
			typename valueMapType::iterator it = this->_valueMap.find(key);
			this->_accessTimeList.splice(this->_accessTimeList.end(), this->_accessTimeList,
					(*it).second.second);
			(*it).second.first = value;
			return;
		}

		debug("LRU Size %zu / %" PRIu32 "\n",this->_valueMap.size(), this->_size);
		if (this->_valueMap.size() == this->_size)
			pop_back();

		typename list<keyType>::iterator it = this->_accessTimeList.insert(
				this->_accessTimeList.end(), key);
		this->_valueMap.insert(make_pair(key, make_pair(value, it)));
	}
	;

	void pop_back() {
		typename valueMapType::iterator it = this->_valueMap.find(
				this->_accessTimeList.front());

		FILE* filePtr = (*it).second.first;
		fflush (filePtr);
		//fsync (fileno (filePtr));
		fclose(filePtr);
		debug ("CLOSE2: %s\n", (*it).first.c_str());

		this->_valueMap.erase(it);
		this->_accessTimeList.pop_front();
	}
	;
};

#endif /* FILELRUCACHE_HH_ */
