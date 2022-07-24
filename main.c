#include"include/ezfs_util.h"
#include"include/dir_tree.h"
#include"fuse.h"

extern dir_node_p dir_tree;
extern struct fuse_operations do_oper;

int main(int argc, char* argv[]) {
    char** buffer = (char**)malloc(512 * sizeof (char*));
    dir_node_p dir = NULL;
    for(int i = 0; i < argc; i++){
        if(strcmp(argv[i], "-mergestart") == 0){
            int num_layer = 0;
            for(int j = 1; strcmp(argv[i + j], "-mergeend"); j++){
                buffer[j - 1] = (char*)malloc(512 * sizeof (char));
                strcpy(buffer[j - 1], argv[i + j]);
                num_layer = j;
            }
            if(num_layer != 0){
                dir = init(buffer[0], buffer + 1, num_layer - 1);
            }
            break;
        }
    }
    dir_tree = dir;
    return fuse_main(argc, argv, &do_oper);
}
