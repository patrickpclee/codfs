#include "dummycache.hh"

uint32_t DummyCache::read (uint32_t id)
{
	throw new CacheMissException();
	return 0;
}

uint32_t DummyCache::write (uint32_t id, char* data)
{
	throw new CacheMissException();
	return 0;
}

uint32_t DummyCache::createEntry (uint32_t id)
{
	throw new CacheMissException();
	return 0;
}

void DummyCache::deleteEntry (uint32_t id)
{
	throw new CacheMissException();
	return ;
}
