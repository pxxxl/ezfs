#include"../include/dir_tree.h"
#include"../include/ezfs_util.h"

extern dir_node_p dir_tree = NULL;

static int has_sub_node(dir_node_p node){
    if(node->type == file || node->num_children == 0){
        return 0;
    }
    return 1;
}

//link the "next" pointer across the folder
static void link_pointer_across_folders(dir_node_p root){
    if(root == NULL || root->num_children == 0){
        return;
    }

    int count = 0;
    int debug_line = 0;
    dir_node_p head = root;

    //get the count of the "has_node" folders
    while(head != NULL){
        dir_node_p cur   = head;
        dir_node_p saver = head;
        head = NULL;
        while(cur != NULL){
            if(has_sub_node(cur)){
                count++;
                if(count == 1){
                    head = cur->children[0];
                }
            }
            cur = cur->next;
        }
        dir_node_p* folders = (dir_node_p*)calloc(count, sizeof(dir_node_p));
        DEBUG_PRINT("link folder... height %d, folders to link count %d\n", debug_line, count);
        debug_line++;

        cur = saver;
        count = 0;
        while(cur != NULL){
            if(has_sub_node(cur)){
                folders[count] = cur;
                count++;
            }
            cur = cur->next;
        }

        for(int i = 0; i < count - 1; i++){
            dir_node_p pri = folders[i + 1];
            dir_node_p pre = folders[i];
            pre->children[pre->num_children - 1]->next = pri->children[0];
            pri->children[0]->last = pre->children[pre->num_children - 1];
            DEBUG_PRINT("link folder... set %s next-> %s\n", pre->children[pre->num_children - 1]->name, pri->children[0]->name);
        }

        count = 0;
        free(folders);
    }
}

//use allocated node and the DIR* which represent the node, build children of the node recursively
static void build_primary_dir_tree_helper(dir_node_p node, DIR* root, dir_node_p root_dir){
    if(root == NULL){
        return;
    }

    //initialize linked list utilities
    struct dirent* ent;
    dir_node_p dumb_head = (dir_node_p)malloc(sizeof(dir_node_t));
    dir_node_p cursor    = dumb_head;
    dir_node_p prior     = NULL;

    node->num_children = 0;

    //content check loop
    while((ent = readdir(root)) != NULL) {
        if (ent->d_type != DT_DIR) {
            //the ent is a file
            prior = (dir_node_p) malloc(sizeof(dir_node_t));
            prior->name = (char *) malloc(sizeof(char) * MAX_DIR_NAME_LENGTH);
            memcpy(prior->name, ent->d_name, MAX_DIR_NAME_LENGTH);
            prior->path = (char*) malloc(sizeof (char) * MAX_DIR_NAME_LENGTH);
            sprintf(prior->path, "%s%s", node->path, prior->name);
            prior->type = file;
            cursor->next = prior;
            prior->last = cursor;
            prior->root = root_dir;
            cursor = prior;
            node->num_children++;
            DEBUG_PRINT("build primary tree... parent: %s, file: %s\n", node->path, prior->name);
        } else {
            //the ent is a folder
            if (strncmp(ent->d_name, ".", 1) == 0) {
                continue;
            } else {
                //handle the node before dir and the dir itself
                prior = (dir_node_p) malloc(sizeof(dir_node_t));
                prior->name = (char *) malloc(sizeof(char) * MAX_DIR_NAME_LENGTH);
                memcpy(prior->name, ent->d_name, MAX_DIR_NAME_LENGTH);
                prior->path = (char*) malloc(sizeof (char) * MAX_DIR_NAME_LENGTH);
                sprintf(prior->path, "%s%s/", node->path, prior->name);
                prior->type = folder;
                cursor->next = prior;
                prior->last = cursor;
                prior->root = root_dir;
                cursor = prior;
                node->num_children++;
                DEBUG_PRINT("build primary tree... parent: %s, folder: %s\n", node->path, prior->name);

                //handle children of the dir
                char dir_name_buffer[MAX_DIR_NAME_LENGTH];
                sprintf(dir_name_buffer, "%s%s/", node->path, prior->name);
                DIR *this_dir = Opendir(dir_name_buffer);
                build_primary_dir_tree_helper(prior, this_dir, root_dir);
                Closedir(this_dir);
            }
        }
    }
    if(prior != NULL)
        prior->next = NULL;
    if(dumb_head->next != NULL)
        dumb_head->next->last = NULL;

    //now attach the children on the node
    node->children = (dir_node_p*)malloc(sizeof(dir_node_t) * node->num_children);
    cursor = dumb_head->next;
    for(int i = 0; i < node->num_children; i++){
        node->children[i] = cursor;
        DEBUG_PRINT("build primary tree... attach %s to %s\n", cursor->name, node->path);
        cursor = cursor->next;
    }

    free(dumb_head);
}

//build dir tree, without linking the layers
static dir_node_p build_dir_tree(const char* path){
    DIR* root = Opendir(path);
    dir_node_p node = (dir_node_p)malloc(sizeof(dir_node_t));
    node->next = NULL;
    node->name = "/";
    node->path = (char*)path;
    node->type = folder;
    node->lower= NULL;
    DEBUG_PRINT("build primary tree... root declared: %s\n", path);

    build_primary_dir_tree_helper(node, root, node);
    link_pointer_across_folders(node);
    return node;
}

//merge 2 folders recursively, without checking the root dir name. Parameters are dir-trees
static void merge_2_folders(dir_node_p upper, dir_node_p lower){
    DEBUG_PRINT("merge folder... %s, %s\n", upper->path, lower->path);
    for(int i = 0; i < upper->num_children; i++){
        for(int j = 0; j < lower->num_children; j++){
            dir_node_p up  = upper->children[i];
            dir_node_p low = lower->children[j];
            if(strcmp(up->name, low->name) == 0 && up->type == low->type){
                up->lower = low;
                if(up->type == folder){
                    merge_2_folders(up, low);
                }else{
                    DEBUG_PRINT("merge file... %s, %s\n", up->path, low->path);
                }
            }
        }
    }
}

//merge the different layers
static void merge_layers(dir_node_p* layers, unsigned num_layers){
    for(int i = 0; i < num_layers - 1; i++){
        merge_2_folders(layers[i], layers[i+1]);
        DEBUG_PRINT("complete merge of %s, %s\n\n", layers[i]->path, layers[i+1]->path);
    }
}

//init the dir_tree
dir_node_p init(const char* upper_path, const char** lower_paths, unsigned lower_path_num){
    char** dirs = (char**)calloc(lower_path_num + 1, sizeof(char*));
    dir_node_p* dir_trees = (dir_node_p*)calloc(lower_path_num + 1, sizeof(dir_node_p));

    dirs[0] = (char*)upper_path;
    for(int i = 1; i <= lower_path_num; i++){
        dirs[i] = (char*)lower_paths[i - 1];
    }

    for(int i = 0; i < lower_path_num + 1; i++){
        dir_trees[i] = build_dir_tree(dirs[i]);
        DEBUG_PRINT("\n");
    }

    merge_layers(dir_trees, lower_path_num + 1);
    DEBUG_PRINT("init complete\n");
    return dir_trees[0];
}

dir_node_p access_within_layer (const dir_node_t* root, const char* path){
    char source[MAX_DIR_HEIGHT][MAX_DIR_NAME_LENGTH];
    char buffer[MAX_DIR_HEIGHT][MAX_DIR_NAME_LENGTH];
    strcpy(source[0], path);
    int count = 0;
    do{
        strcpy(buffer[count], basename(source[count]));
        strcpy(source[count + 1], dirname(source[count]));
        count++;
    }while(strcmp(buffer[count - 1], "/") != 0);

    const dir_node_t* cursor = root;
    for(int i = count - 2; i >= 0; i--){
        int found = 0;
        for(int j = 0; j < cursor->num_children; j++){
            dir_node_p current = cursor->children[j];
            if(strcmp(current->name, buffer[i]) == 0 && current->type == folder){
                cursor = current;
                found = 1;
                break;
            }
        }
        if(!found){
            DEBUG_PRINT("access within layer error : invalid path\n");
            return NULL;
        }
    }
    DEBUG_PRINT("access within layer: %s\n", path);
    return (dir_node_p)cursor;
}

dir_node_p access_cross_layer (const dir_node_t* root, const char* path){
    char source[MAX_DIR_HEIGHT][MAX_DIR_NAME_LENGTH];
    char buffer[MAX_DIR_HEIGHT][MAX_DIR_NAME_LENGTH];
    strcpy(source[0], path);
    int count = 0;
    do{
        strcpy(buffer[count], basename(source[count]));
        strcpy(source[count + 1], dirname(source[count]));
        count++;
    }while(strcmp(buffer[count - 1], "/") != 0);

    const dir_node_t* cursor = root;
    for(int i = count - 2; i >= 0; i--){
        int found = 0;
        for(int j = 0; j < cursor->num_children; j++){
            dir_node_p current = cursor->children[j];
            if(strcmp(current->name, buffer[i]) == 0 && current->type == folder){
                cursor = current;
                found = 1;
                break;
            }
        }
        if(!found && cursor->lower != NULL){
            i++;
            cursor = cursor->lower;
        }else if(!found && cursor->lower == NULL){
            DEBUG_PRINT("Access across layer error : invalid path\n");
            return NULL;
        }
    }
    DEBUG_PRINT("access across layer: %s\n", path);
    return (dir_node_p)cursor;
}

//if there are upper node in upper layer, return it
//else return NULL
dir_node_p access_in_upper_layer(const dir_node_t* root, const char* path){
    dir_node_p cursor = access_within_layer(root, path);
    if(cursor == NULL){
        return NULL;
    }else
        return cursor;
}

dir_node_p insert_node(const dir_node_t* root, const char* dir_path, const char* name, enum node_type type){
    dir_node_p fold = access_cross_layer(root, dir_path);
    if(fold == NULL){
        return NULL;
    }
    fold->num_children++;
    if(fold->num_children == 1){
        fold->children = (dir_node_p*) calloc(fold->num_children, sizeof (dir_node_p));
    }else{
        fold->children = (dir_node_p*) reallocarray(fold->children, fold->num_children, sizeof (dir_node_p));
    }
    dir_node_p target = (dir_node_p) malloc(sizeof (dir_node_t));
    target->num_children = 0;
    target->children = NULL;
    target->type = type;
    target->name = (char *) malloc(sizeof(char) * MAX_DIR_NAME_LENGTH);
    memcpy(target->name, name, MAX_DIR_NAME_LENGTH);
    target->path = (char*) malloc(sizeof (char) * MAX_DIR_NAME_LENGTH);
    target->root = fold->root;
    sprintf(target->path, "%s%s", dir_path, name);

    fold->children[fold->num_children - 1] = target;
    //now complete the current layer
    if(fold->num_children == 1){
        dir_node_p pre_link = NULL;
        dir_node_p pri_link = NULL;
        dir_node_p cursor = fold->last;
        while(cursor != NULL){
            if(has_sub_node(cursor)){
                pre_link = cursor;
                break;
            }
            cursor = cursor->last;
        }
        cursor = fold->next;
        while(cursor != NULL){
            if(has_sub_node(cursor)){
                pri_link = cursor;
                break;
            }
            cursor = cursor->next;
        }
        if(pre_link != NULL && pri_link != NULL){
            dir_node_p pre_target = pre_link->children[pre_link->num_children - 1];
            dir_node_p pri_target = pri_link->children[0];
            target->next = pri_target;
            target->last = pre_target;
            pre_target->next = target;
            pri_target->last = target;
        }else if(pri_link == NULL && pre_link != NULL){
            dir_node_p pre_target = pre_link->children[pre_link->num_children - 1];
            target->next = NULL;
            target->last = pre_target;
            pre_target->next = target;
        }else if(pre_link != NULL && pri_link == NULL){
            dir_node_p pri_target = pri_link->children[0];
            target->next = pri_target;
            target->last = NULL;
            pri_target->last = target;
        }else{
            target->next = NULL;
            target->last = NULL;
        }
    }else{
        dir_node_p pre_target = fold->children[fold->num_children - 2];
        target->next = pre_target->next;
        pre_target->next = target;
        target->last = pre_target;
    }

    //now link the layers
    dir_node_p root_dir = target->root;
    dir_node_p pre_root_dir = root_dir;
    dir_node_p pri_root_dir = root_dir->next;
    while(pre_root_dir->next != root_dir && pre_root_dir != NULL){
        pre_root_dir = pre_root_dir->lower;
    }


}

//1. dir_path must be accessible at the upper layer
dir_node_p insert_node_in_upper_layer(const dir_node_t* root, const char* dir_path, const char* name, enum node_type type){
    dir_node_p fold = access_within_layer(root, dir_path);
    if(fold == NULL){
        return NULL;
    }
    fold->num_children++;
    if(fold->num_children == 1){
        fold->children = (dir_node_p*) calloc(fold->num_children, sizeof (dir_node_p));
    }else{
        fold->children = (dir_node_p*) reallocarray(fold->children, fold->num_children, sizeof (dir_node_p));
    }
    dir_node_p target = (dir_node_p) malloc(sizeof (dir_node_t));
    target->num_children = 0;
    target->children = NULL;
    target->type = type;
    target->name = (char *) malloc(sizeof(char) * MAX_DIR_NAME_LENGTH);
    memcpy(target->name, name, MAX_DIR_NAME_LENGTH);
    target->path = (char*) malloc(sizeof (char) * MAX_DIR_NAME_LENGTH);
    target->root = fold->root;
    sprintf(target->path, "%s%s", dir_path, name);

    fold->children[fold->num_children - 1] = target;
    //now complete the current layer
    if(fold->num_children == 1){
        dir_node_p pre_link = NULL;
        dir_node_p pri_link = NULL;
        dir_node_p cursor = fold->last;
        while(cursor != NULL){
            if(has_sub_node(cursor)){
                pre_link = cursor;
                break;
            }
            cursor = cursor->last;
        }
        cursor = fold->next;
        while(cursor != NULL){
            if(has_sub_node(cursor)){
                pri_link = cursor;
                break;
            }
            cursor = cursor->next;
        }
        if(pre_link != NULL && pri_link != NULL){
            dir_node_p pre_target = pre_link->children[pre_link->num_children - 1];
            dir_node_p pri_target = pri_link->children[0];
            target->next = pri_target;
            target->last = pre_target;
            pre_target->next = target;
            pri_target->last = target;
        }else if(pri_link == NULL && pre_link != NULL){
            dir_node_p pre_target = pre_link->children[pre_link->num_children - 1];
            target->next = NULL;
            target->last = pre_target;
            pre_target->next = target;
        }else if(pre_link == NULL && pri_link != NULL){
            dir_node_p pri_target = pri_link->children[0];
            target->next = pri_target;
            target->last = NULL;
            pri_target->last = target;
        }else{
            target->next = NULL;
            target->last = NULL;
        }
    }else{
        dir_node_p pre_target = fold->children[fold->num_children - 2];
        target->next = pre_target->next;
        pre_target->next = target;
        target->last = pre_target;
    }
    //didn't link the layer
    return target;
}

void delete_node(const dir_node_t* root, const char* dir_path, const char* name){
    char buffer[MAX_DIR_NAME_LENGTH];
    sprintf(buffer, "%s%s", dir_path, name);
    dir_node_p real_node = access_cross_layer(root, buffer);
    dir_node_p upper_node = access_within_layer(root, buffer);

    if(real_node == upper_node){
        dir_node_p dir = access_within_layer(root, dir_path);
        dir->num_children--;
        dir_node_p * p = (dir_node_p*)calloc(dir->num_children, sizeof(dir_node_p));
        for(int i = 0, j = 0; i <= dir->num_children; i++){
            if(dir->children[i] != real_node){
                p[j] = dir->children[i];
                j++;
            }
        }
        free(real_node);
        free(dir->children);
        dir->children = p;
    }else{
        dir_node_p dir = access_within_layer(root, dir_path);
        dir->num_children++;
        dir->children = (dir_node_p*) reallocarray(dir->children, dir->num_children, sizeof (dir_node_p));
        dir_node_p p = (dir_node_p) malloc(sizeof(dir_node_t));
        p->name = (char *) malloc(sizeof(char) * MAX_DIR_NAME_LENGTH);
        memcpy(p->name, real_node->name, MAX_DIR_NAME_LENGTH);
        p->path = (char*) malloc(sizeof (char) * MAX_DIR_NAME_LENGTH);
        sprintf(p->path, "%s%s", dir->path, p->name);
        p->type = file;
        p->root = dir->root;
        dir->children[dir->num_children - 1] = p;
    }
}

int has_own_this_filename(const dir_node_t* root, const char* dir_path, const char* name, enum node_type type){
    dir_node_p p = access_cross_layer(root, dir_path);
    for(int i = 0; i< p->num_children; i++){
        dir_node_p cur = p->children[i];
        if(strcmp(cur->name, name)==0&&cur->type == type){
            return 0;
        }
    }
    return 1;
}