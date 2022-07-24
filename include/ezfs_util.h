#ifndef EZFS_EZFS_UTIL_H
#define EZFS_EZFS_UTIL_H

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <fuse.h>
#include <limits.h>

#define DEBUG

#ifdef DEBUG
#define DEBUG_PRINT(...) do{ fprintf( stderr, __VA_ARGS__ ); } while( 0 )
#else
#define DEBUG_PRINT(...) do{ } while ( 0 )
#endif

struct dirent * Readdir(DIR * dir);
DIR* Opendir(const char* path);
int Closedir(DIR* dirp);
int Open(char* filename, int flags, mode_t mode);
int Close(int fd);
ssize_t Read(int fd, void* buffer, size_t size);


#endif //EZFS_EZFS_UTIL_H
