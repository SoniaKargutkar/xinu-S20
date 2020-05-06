#include<xinu.h>
#include<tree.h>

void newTree(){
    ROOT = getmem(sizeof(struct node));
    ROOT->key = -1;
     EMPTYNODE = getmem(sizeof(struct node));
        EMPTYNODE->key = -1;
        ROOT->left = EMPTYNODE;
        ROOT->right = EMPTYNODE;
   
}