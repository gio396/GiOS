#define RB_BLACK 1
#define RB_RED 0

typedef i32 rbcolor;

struct rb_node
{
  struct rb_node *parent;
  rbcolor color;

  struct rb_node *left;
  struct rb_node *right;
};

struct rb_root
{
  struct rb_node *root;
};

#define DEFAULT_RB_TREE {.root = NULL}
#define INIT_RBTREE(name) struct rb_root name = DEFAULT_RB_TREE 

#define RB_PARENT(nod) ((nod) -> parent)

void
rbtree_insert(struct rb_root *root, struct rb_node *node);

void
rb_link_node(struct rb_node *new, struct rb_node *parent, struct rb_node **link);