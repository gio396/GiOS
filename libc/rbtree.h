#ifndef __RB_TREE_H__
#define __RB_TREE_H__

#define RB_BLACK 1
#define RB_RED   0

#include <common.h>

typedef size_t rbcolor;

struct rb_node
{
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

#define RB_GET_PARENT(nod) ((struct rb_node*)(((nod) -> color) & ~(0x1)))
#define RB_GET_COLOR(n) ((n) -> color & 0x1)
#define RB_SET_COLOR(n, c) (n) -> color = ((((n) -> color & ~(0x1)) | (c)))
#define RB_SET_PARENT(n, p) ((n) -> color = ((n) -> color & 0x1) | (size_t)(p))
#define RB_SET_PARENT_COLOR(n, p, c) ((n) -> color = ((size_t)(p) | (c)))
#define RB_BLACK 1
#define RB_RED   0

void
rbtree_insert(struct rb_root *root, struct rb_node *node);

void
rb_link_node(struct rb_node *new, struct rb_node *parent, struct rb_node **link);

//Creating new node types and insertion example
//Avoids calling comparison function for each walked node.
/*
struct my_val
{
  int32 val;
  struct rb_node node;
};

void
my_val_insert(struct rb_root* root, struct my_val *new)
{
  struct rb_node **link = &root -> root;
  struct rb_node *parent = NULL;

  while (*link)
  {
    parent = *link;
    struct my_val *tmp = CONTAINER_OF(parent, struct my_val, node);

    if (new -> val > tmp -> val)
    {
      link = &(parent -> right);
    }
    else
    {
      link = &(parent -> left);
    }
  }

  rb_link_node(&new -> node, parent, link);
  rbtree_insert(root, &new -> node);
}
*/

#endif
