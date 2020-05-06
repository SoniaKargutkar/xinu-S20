#define RED 0
#define BLACK 1

extern struct node{
	int key;
	int color_t; 
	struct node *parent;
	struct node *left;
	struct node *right;
};

extern struct node *ROOT;
extern struct node *EMPTYNODE;

extern void newTree(); 
extern void tr_insert(int key);
extern struct node *tr_search(int key);
extern void tr_delete(struct node *deleteNode);
extern void tr_fixDelete(struct node *fixNode); 
extern void tr_transplant(struct node *deleteNode, struct node *deleteChildNode); 
extern struct node *minimumNode(struct node *deleteNode); 
extern void storeProcess(); 
extern void rbFixR(struct node *n); 
extern void rbFixL(struct node *n); 
extern void rbTreeRotation(struct node *z); 
extern void printTree(struct node *n); 

