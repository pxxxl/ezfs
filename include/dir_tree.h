#ifndef EZFS_DIR_TREE_H
#define EZFS_DIR_TREE_H

#define MAX_DIR_HEIGHT 512
#define MAX_DIR_NAME_LENGTH 512

enum node_type{
  folder,
  file
};

typedef struct dir_node{
    enum node_type    type;
    char*             name;
    char*             path;
    struct dir_node*  lower;
    struct dir_node*  next;
    struct dir_node*  last;
    struct dir_node*  root;
    unsigned          num_children;
    struct dir_node** children;
}dir_node_t, *dir_node_p, *const dir_node_cp;

dir_node_p init(const char* upper_path, const char** lower_paths, unsigned lower_path_num);
dir_node_p access_within_layer (const dir_node_t* root, const char* path);
dir_node_p access_cross_layer (const dir_node_t* root, const char* path);
int insert_node(const dir_node_t* root, const char* dir_path, const char* name, enum node_type type);
int delete_node(const dir_node_t* root, const char* dir_path, const char* name);//1 stand for user need to delete the actual node//don't forget to recursively delete
int has_the_same_filename(const dir_node_t* root, const char* dir_path, const char* name, enum node_type type);
dir_node_p modify_access(const dir_node_t* root, const char* path);
int rename_node(const dir_node_t* root, const char* path, const char* newname);
int move_node(const dir_node_t* root, const char* path, const char* newpath);
#endif

//the requirations the tree should follow

/* access_within_layer : use the path to get the dir_node_p.if cannot found return NULL;
 * access_cross_layer  : use the path to try to find thedir_node_p in every layer, if cannott found return NULL;
 * insert_node         : insert a node at upper layer, if failed return -1
 * delete_node         : delete a node(upper), and hide nodes below;
 * has_...
 * modify_access       : if upper layer, just like access cross, if lower, just copy and overload
 * rename a file, if failed return -1
 * move a file, if failed return -1( cannot mode it to the lower folder)
 *
 *
 *
 *
 * */