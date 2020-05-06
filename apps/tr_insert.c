#include<xinu.h>
#include<tree.h>

struct node *ROOT;
struct node *EMPTYNODE;
int index=0;
int prev;


void rbTreeRotation(struct node *rotateNode){
    while(rotateNode->parent->color_t == RED){
        if(rotateNode->parent->parent->left == rotateNode->parent){
            if(rotateNode->parent->parent->right->color_t == RED){
                rotateNode->parent->color_t = BLACK;
                rotateNode->parent->parent->right->color_t = BLACK;
                rotateNode->parent->parent->color_t = RED;
                rotateNode = rotateNode->parent->parent;
            }else{
                if(rotateNode == rotateNode->parent->right){
                    rotateNode = rotateNode->parent;
                    rbFixL(rotateNode);
                }
                rotateNode->parent->color_t = BLACK;
                rotateNode->parent->parent->color_t = RED;
                rbFixR(rotateNode->parent->parent);
            }
        }
    else{
        if(rotateNode->parent->parent->left->color_t == RED){
            rotateNode->parent->color_t = BLACK;
            rotateNode->parent->parent->color_t = RED;
            rotateNode->parent->parent->left->color_t = BLACK;
            rotateNode = rotateNode->parent->parent;
        }else{
            if(rotateNode == rotateNode->parent->left){
                rotateNode = rotateNode->parent;
                rbFixR(rotateNode);
            }
            rotateNode->parent->color_t = BLACK;
            rotateNode->parent->parent->color_t = RED;
            rbFixL(rotateNode->parent->parent);
        }
    }
    }
    ROOT->color_t = BLACK;

}


void rbFixR(struct node *rotateRNode){

    struct node *n;
    n = rotateRNode->left;
    rotateRNode->left = n->right;
    if(n->right->key != -1){
        n->right->parent = rotateRNode;
    }
    n->parent = rotateRNode->parent;
    if(n->parent->key == -1){
        ROOT = n;
    }else if(rotateRNode == rotateRNode->parent->left){
        rotateRNode->parent->left = n;
    }else{
        rotateRNode->parent->right = n;
    }

    n->right = rotateRNode;
    rotateRNode->parent = n;
}

void rbFixL(struct node *rotateLNode){
    struct node *n;
    n = rotateLNode->right;
    rotateLNode->right = n->left;
    if(n->left->key != -1){
        n->left->parent = rotateLNode;
    }
    n->parent = rotateLNode->parent;
    if(n->parent->key == -1){
        ROOT = n;
    }else if(rotateLNode == rotateLNode->parent->left){
        rotateLNode->parent->left = n;
    }else{
        rotateLNode->parent->right = n;
    }
    n->left = rotateLNode;
    rotateLNode->parent = n;

}

void tr_insert(int key){

    struct node *n;
    n = getmem(sizeof(struct node));
    n->key = key;
    n->color_t = RED;
    n->left = EMPTYNODE;
    n->right = EMPTYNODE;
    struct node *n1;
    struct node *n2;
    n1 = ROOT;
    n2 = EMPTYNODE;
    while(n1->key != -1){
        n2 = n1;
        if(n->key <= n1->key){
            n1 = n1->left;
        }else{
            n1 = n1->right;
        }
    }
    if(n2->key == -1){
        ROOT = n;
    }
    else if(n->key <= n2->key){
        n2->left = n;
    }else {
        n2->right  = n;
    }

    n->parent = n2;
    rbTreeRotation(n);
    kprintf("Inserting %d \n", key);
}

void tr_fixDelete(struct node *fixNode){
    struct node *n;
    while(fixNode!=ROOT && fixNode->color_t==BLACK){
        if(fixNode == fixNode->parent->left){
            n = fixNode->parent->right;
            if(n->color_t == RED){
                n->color_t = BLACK;
                fixNode->parent->color_t = RED;
                rbFixL(fixNode->parent);
                n = fixNode->parent->right;
            }

            if(n->left->color_t == BLACK && n->right->color_t==BLACK){
                n->color_t = RED;
                fixNode->parent->color_t = BLACK;
                fixNode = fixNode->parent;

            }else{
                if(n->right->color_t == BLACK){
                    n->color_t = RED;
                    n->left->color_t = BLACK;
                    rbFixR(n);
                    n = fixNode->parent->right;
                }
                n->color_t = fixNode->parent->color_t;
                fixNode->parent->color_t = BLACK;
                fixNode->right->color_t = BLACK;
                rbFixL(fixNode->parent);
                fixNode = ROOT;
            }
        }
        else{
            n = fixNode->parent->left;
            if(n->color_t == RED){
                n->color_t = BLACK;
                fixNode->parent->color_t = BLACK;
                rbFixR(fixNode->parent);
                n = fixNode->parent->left;

            }
            if(n->left->color_t == BLACK && n->right->color_t == BLACK){
                n->color_t = RED;
                fixNode->parent->color_t = BLACK;
                fixNode = fixNode->parent;
            }else{
                if(n->left->color_t == BLACK){
                    n->color_t = RED;
                    n->right->color_t = BLACK;
                    rbFixL(n);
                    n = fixNode->parent->left;

                }
                n->color_t = fixNode->parent->color_t;
                fixNode->parent->color_t = BLACK;
                n->left->color_t = BLACK;
                rbFixR(fixNode->parent);
                fixNode = ROOT;
            }
        }
    }

    fixNode->color_t = BLACK;

}


struct node *minimumNode(struct node *deleteNode){
    while(deleteNode->left->key != -1){
        deleteNode = deleteNode->left;
    }
    return deleteNode;
}

void tr_delete(struct node *deleteNode){
    struct node *n;
    struct node *temp;
    int color;

    temp = deleteNode;
    color = deleteNode->color_t;
    if(deleteNode->left->key == -1){
        n = deleteNode->right;
        tr_transplant(deleteNode, deleteNode->right);
    }else if(deleteNode->right->key == -1){
        n = deleteNode->left;
        tr_transplant(deleteNode, deleteNode->left);
    }else{
        temp = minimumNode(deleteNode->right);
        color = temp->color_t;
        n = temp->right;
        if(temp->parent == deleteNode){
            n->parent = temp;
        }else{
            tr_transplant(temp, temp->right);
            temp->right = deleteNode->right;
            temp->right->parent = temp;
        }
        tr_transplant(deleteNode, temp);
        temp->left = deleteNode->left;
        temp->left->parent = temp;
        temp->color_t = deleteNode->color_t;

    }

    if(color == BLACK){
        tr_fixDelete(n);
    }

}

void tr_transplant(struct node *deleteNode, struct node *deleteChildNode){
    if(deleteNode->parent->key == -1){
        ROOT = deleteChildNode;
    }else if(deleteNode == deleteNode->parent->left){
        deleteNode->parent->left = deleteChildNode;
    }else{
        deleteNode->parent->right = deleteChildNode;
    }

    deleteChildNode->parent = deleteNode->parent;

}

void printTree(struct node *temp){
if (temp != -1){
    printTree(temp->left);
    kprintf("%d\t", temp -> key);
    printTree(temp->right);

void storeProcess(){
    kprintf("\n Process ID: %d\n", getpid());

    index++;
    prev = getpid();
    tr_insert(getpid());

    if(index == 2){
            tr_search(prev);
        }
    if(index >= 4){
        printTree(ROOT);
    }
    suspend(getpid());
   
}
}
}    
	