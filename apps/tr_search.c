#include<xinu.h>
#include<tree.h>

struct node *tr_search(int key){
    struct node *n;
    n = ROOT;
    while(n->key != -1 && n->key!=key){
        if(key < n->key){
            n = n->left;
        }else{
            n = n->right;
        }
    }
    kprintf("\n Searching for: %d\n\n", n->key);
    tr_delete(n);
    kprintf("\n Delete the searched value.. \n");
    return n;
}