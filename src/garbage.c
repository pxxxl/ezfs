#include"../include/dir_tree.h"
#include"../include/ezfs_util.h"

static int own_the_same_name(struct dirent* A, struct dirent* B){
    if(A->d_name == B->d_name){
        return 1;
    }else{
        return 0;
    }
}

dir_node_p first_sub_folder(dir_node_p fold){
    if(fold == NULL){
        return NULL;
    }
    for(int i = 0; i < fold->num_children; i++){
        if(fold->children[i]->type == folder){
            return fold->children[i];
        }
    }
    return NULL;
}

dir_node_p last_sub_folder(dir_node_p fold){
    if(fold == NULL){
        return NULL;
    }
    for(int i = fold->num_children - 1; i > -1; i--){
        if(fold->children[i]->type == folder){
            return fold->children[i];
        }
    }
    return NULL;
}

