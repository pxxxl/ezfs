#include <fuse.h>
#include "../include/dir_tree.h"
#include "../include/ezfs_util.h"

#define FUSE_USE_VERSION 29

extern dir_node_p dir_tree;

static int complw_do_getattr( const char *path, struct stat *st ){
    dir_node_p cursor = access_cross_layer(dir_tree, path);
    int stat = lstat(cursor->path, st);
    DEBUG_PRINT("getattr called: path=%s\n", path);
    return stat;
}
static int complw_do_readlink(const char *path, char *link, size_t size)
{
    size_t stat;
    dir_node_p cursor = access_cross_layer(dir_tree, path);
    stat = readlink(cursor->path, link, size - 1);
    if (stat >= 0) {
        link[stat] = '\0';
        stat = 0;
    }
    DEBUG_PRINT("readlink called: path=%s, link=%s", path, link);
    return (int)stat;
}
static int complw_do_mknod(const char *path, mode_t mode, dev_t dev){
    int stat;
    char buffer[MAX_DIR_NAME_LENGTH];
    char buffer2[MAX_DIR_NAME_LENGTH];
    strcpy(buffer, path);
    strcpy(buffer2, path);
    char* dir = dirname(buffer);
    char* bas = basename(buffer2);
    if(has_the_same_filename(dir_tree, dir, bas, file)){
        return 0;
    }
    dir_node_p cursor = access_cross_layer(dir_tree, path);
    insert_node(dir_tree, dir, bas,file);
    if (S_ISREG(mode)) {
        stat = open(cursor->path, O_CREAT | O_EXCL | O_WRONLY, mode);
        if (stat >= 0)
            stat = close(stat);
    } else{
        if (S_ISFIFO(mode))
            stat = mkfifo(cursor->path, mode);
        else
            stat = mknod(cursor->path, mode, dev);
    }
    return stat;
}
static int complw_do_mkdir(const char *path, mode_t mode)
{
    char buffer[MAX_DIR_NAME_LENGTH];
    char buffer2[MAX_DIR_NAME_LENGTH];
    strcpy(buffer, path);
    strcpy(buffer2, path);
    char* dir = dirname(buffer);
    char* bas = basename(buffer2);
    if(has_the_same_filename(dir_tree, dir, bas, file)){
        return 0;
    }
    dir_node_p cursor = access_cross_layer(dir_tree, path);
    insert_node(dir_tree, dir, bas,folder);
    return mkdir(cursor->path, mode);
}
static int complw_do_unlink(const char *path)
{
    char buffer[MAX_DIR_NAME_LENGTH];
    char buffer2[MAX_DIR_NAME_LENGTH];
    strcpy(buffer, path);
    strcpy(buffer2, path);
    char* dir = dirname(buffer);
    char* bas = basename(buffer2);
    delete_node(dir_tree, dir, bas);
    dir_node_p cursor = access_cross_layer(dir_tree, path);
    return unlink(cursor->path);
}
static int complw_do_rmdir(const char *path)
{
    char buffer[MAX_DIR_NAME_LENGTH];
    char buffer2[MAX_DIR_NAME_LENGTH];
    strcpy(buffer, path);
    strcpy(buffer2, path);
    char* dir = dirname(buffer);
    char* bas = basename(buffer2);
    int need = delete_node(dir_tree, dir, bas);
    dir_node_p cursor = access_cross_layer(dir_tree, path);
    if(need){
        return rmdir(cursor->path);
    }
    return 0;
}
static int complw_do_symlink(const char *path, const char *link)
{
    char buffer[MAX_DIR_NAME_LENGTH];
    char buffer2[MAX_DIR_NAME_LENGTH];
    strcpy(buffer, link);
    strcpy(buffer2, link);
    char* dir = dirname(buffer);
    char* bas = basename(buffer2);
    int res = insert_node(dir_tree, dir, bas, file);
    if(res == -1){
        return 0;
    }
    dir_node_p cursor = access_cross_layer(dir_tree, link);
    return symlink(path, cursor->path);
}
static int dd_rename(const char *path, const char *newpath)
{
    //??
    char fpath[PATH_MAX];
    char fnewpath[PATH_MAX];
    bb_fullpath(fpath, path);
    bb_fullpath(fnewpath, newpath);

    return log_syscall("rename", rename(fpath, fnewpath), 0);
}
int do_link(const char *path, const char *newpath)
{
    //???
    char fpath[PATH_MAX], fnewpath[PATH_MAX];
    log_msg("\nbb_link(path=\"%s\", newpath=\"%s\")\n",
            path, newpath);
    bb_fullpath(fpath, path);
    bb_fullpath(fnewpath, newpath);
    return log_syscall("link", link(fpath, fnewpath), 0);
}
static int complw_do_chmod(const char *path, mode_t mode){
    dir_node_p cursor = access_cross_layer(dir_tree, path);
    return chmod(cursor->path, mode);
}
static int complw_do_chown(const char *path, uid_t uid, gid_t gid){
    dir_node_p cursor = access_cross_layer(dir_tree, path);
    return chown(cursor->path, uid, gid);
}
static int complw_do_truncate(const char *path, off_t newsize)
{
    dir_node_p cursor = access_cross_layer(dir_tree, path);
    return truncate(cursor->path, newsize);
}
static int complw_do_utime(const char *path, struct utimbuf *ubuf)
{
    dir_node_p cursor = access_cross_layer(dir_tree, path);
    return utime(cursor->path, ubuf);
}
static int complw_do_open(const char *path, struct fuse_file_info *fi)
{
    int stat = 0;
    int fd;
    dir_node_p cursor = access_cross_layer(dir_tree, path);
    fd = open(cursor->path, fi->flags);
    if (fd < 0){
        stat = -errno;
    }
    fi->fh = fd;
    return stat;
}
static int complw_do_read( const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi )
{
    dir_node_p cursor = access_cross_layer(dir_tree, path);
    return pread(fi->fh, buffer, size, offset);
}
static int do_write( const char *path, const char *buffer, size_t size, off_t offset, struct fuse_file_info *fi )
{
    dir_node_p cursor = access_cross_layer(dir_tree, path);
    return pread(fi->fh, buffer, size, offset);
}
static int complw_do_statfs(const char *path, struct statvfs *statv){
    dir_node_p cursor = access_cross_layer(dir_tree, path);
    return statvfs(cursor->path, statv);;
}
static int complw_do_flush(const char *path, struct fuse_file_info *fi){
    return 0;
}
static int complw_do_release(const char *path, struct fuse_file_info *fi){
    return close(fi->fh);
}
static int complw_do_fsync(const char *path, int datasync, struct fuse_file_info *fi)
{
#ifdef HAVE_FDATASYNC
    if (datasync)
	return log_syscall("fdatasync", fdatasync(fi->fh), 0);
    else
#endif
    return fsync(fi->fh);
}
static int complw_do_opendir(const char *path, struct fuse_file_info *fi)
{
    int stat = 0;
    dir_node_p cursor = access_cross_layer(dir_tree, path);
    DIR* dp = opendir(cursor->path);
    if(dp == NULL){
        stat = -errno;
    }
    fi->fh = (intptr_t) dp;
    return stat;
}
static int do_readdir( const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi )
{
    filler( buffer, ".", NULL, 0 ); // Current Directory
    filler( buffer, "..", NULL, 0 ); // Parent Directory

    dir_node_p cursor = access_cross_layer(dir_tree, path);
    for(int i = 0; i < cursor->num_children; i++){
        filler( buffer, cursor->children[i]->name, NULL, 0);
    }

    return 0;
}
static int complw_do_releasedir(const char *path, struct fuse_file_info *fi)
{
    closedir((DIR *) (uintptr_t) fi->fh);
    return 0;
}
static int complw_do_fsyncdir(const char *path, int datasync, struct fuse_file_info *fi)
{
    return 0;
}
static void *complw_do_init(struct fuse_conn_info *conn)
{
    return 0;
}
static void complw_do_destroy(void *userdata){
}
static int complw_do_access(const char *path, int mask){
    int stat = 0;
    dir_node_p cursor = access_cross_layer(dir_tree, path);
    stat = access(cursor->path, mask);
    if (stat < 0)
        stat = -errno;
    return stat;
}
static int complw_do_ftruncate(const char *path, off_t offset, struct fuse_file_info *fi){
    int stat = 0;
    dir_node_p cursor = access_cross_layer(dir_tree, path);
    stat = ftruncate(fi->fh, offset);
    if (stat < 0)
        stat = -errno;
    return stat;
}
static int complw_do_fgetattr(const char *path, struct stat *statbuf, struct fuse_file_info *fi)
{
    int stat = 0;
    if (!strcmp(path, "/"))
        return complw_do_getattr(path, statbuf);

    stat = fstat(fi->fh, statbuf);
    if (stat < 0)
        stat = -errno;
    return stat;
}

extern struct fuse_operations do_oper = {
        .getattr = do_getattr,
        .readlink = do_readlink,
        .getdir = NULL,
        .mknod = do_mknod,
        .mkdir = do_mkdir,
        .unlink = do_unlink,
        .rmdir = do_rmdir,
        .symlink = do_symlink,
        .rename = do_rename,
        .link = do_link,
        .chmod = do_chmod,
        .chown = do_chown,
        .truncate = do_truncate,
        .utime = do_utime,
        .open = do_open,
        .read = do_read,
        .write = do_write,
        .statfs = do_statfs,
        .flush = do_flush,
        .release = do_release,
        .fsync = do_fsync,
        .opendir = do_opendir,
        .readdir = do_readdir,
        .releasedir = do_releasedir,
        .fsyncdir = do_fsyncdir,
        .init = do_init,
        .destroy = do_destroy,
        .access = do_access,
        .ftruncate = do_ftruncate,
        .fgetattr = do_fgetattr
};