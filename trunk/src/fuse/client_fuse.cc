#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <errno.h>

#include "client.hh"
#include "../config/config.hh"

Client* client;

ConfigLayer* configLayer;

void* ncvfs_init(struct fuse_conn_info *conn)
{
	return NULL;
}

static int ncvfs_open(const char *path, struct fuse_file_info *fi)
{
	(void) fi;

	if(strcmp(path, "/") != 0)
		return -ENOENT;

	return 0;
}

static int ncvfs_read(const char *path, char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{
	(void) buf;
	(void) offset;
	(void) fi;

	if(strcmp(path, "/") != 0)
		return -ENOENT;

	if (offset >= (1ULL << 32))
		return 0;

	return size;
}

static int ncvfs_write(const char *path, const char *buf, size_t size,
		      off_t offset, struct fuse_file_info *fi)
{
	(void) buf;
	(void) offset;
	(void) fi;

	if(strcmp(path, "/") != 0)
		return -ENOENT;

	return size;
}

void ncvfs_destroy(void* userdata)
{

}

struct ncvfs_fuse_operations: fuse_operations
{
	ncvfs_fuse_operations ()
	{
		init	= ncvfs_init;
		open	= ncvfs_open;
		read	= ncvfs_read;
		write	= ncvfs_write;
		destroy = ncvfs_destroy;
	}
};

static struct ncvfs_fuse_operations ncvfs_oper;

int main(int argc, char *argv[])
{
	configLayer = new ConfigLayer("clientconfig.xml");
	client = new Client();
	return fuse_main(argc, argv, &ncvfs_oper, NULL);
}
