#include "../include/ezfs_util.h"

struct dirent* Readdir(DIR* dir){
    struct dirent* result = readdir(dir);
    if(result == NULL){
        write(STDERR_FILENO, "error : Readdir\n", 25);
        exit(1);
    }
    return result;
}

DIR* Opendir(const char* path){
    DIR* result = opendir(path);
    if(result == NULL){
        write(STDERR_FILENO, "error : Opendir\n", 25);
        write(STDERR_FILENO, path, 65535);
        exit(1);
    }
    return result;
}

int Closedir(DIR* dirp){
    int result = closedir(dirp);
    if(result == -1){
        write(STDERR_FILENO, "error : Closedir\n", 25);
        exit(1);
    }
    return result;
}

int Open(char* filename, int flags, mode_t mode){
    int fd = open(filename, flags, mode);
    if(fd == -1){
        char error_description[512];
        sprintf(error_description, "error : Open , %s", filename);
        write(STDERR_FILENO, error_description, strlen(error_description));
        exit(1);
    }
    return fd;
}

int Close(int fd){
    int fd_n = close(fd);
    if(fd == -1){
        char* error_description = "error : Close";
        write(STDERR_FILENO, error_description, strlen(error_description));
        exit(1);
    }
    return fd_n;
}

ssize_t Read(int fd, void* buffer, size_t size){
    ssize_t s = read(fd, buffer, size);
    if(s == -1){
        char* error_description = "error : Read";
        write(STDERR_FILENO, error_description, strlen(error_description));
        exit(1);
    }
    return s;
}