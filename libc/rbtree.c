#include "rbtree.h"

void
rbtree_insert(struct rb_root *root, struct rb_node *node)
{
  struct rb_node *parent = RB_PARENT(node), *gparent, *tmp;

  while (1)
  {
    if (!parent)
    {
      /*
       *  n is root
       */
      //case 1
      node -> color = RB_BLACK;
      break;
    } 
    else if (parent -> color == RB_BLACK)
    {
      /*
       *  P
       *  |
       *  n
       */
      //case 2
      break;
    }

    gparent = RB_PARENT(parent);

    tmp = gparent -> right;
    if (parent != tmp)
    {
      if (tmp && tmp -> color == RB_RED)
      {
        /*
         *     G         g
         *    / \       / \
         *   p   u  -> P   U
         *  /         /
         * n         n
         *
         *
         */
        //case 3
        tmp -> color = RB_BLACK;
        parent -> color = RB_BLACK;
        node -> color = RB_RED;

        node = gparent;
        parent = RB_PARENT(node);

        continue;
      }

      //else
      tmp = parent -> right;
      if (node == tmp)
      {
        /*
         *    G        G
         *   / \      / \
         *  p   U -> n   U
         *   \      /
         *    n    p
         *
         */
        //case 4
        tmp = node -> left;
        node -> left = parent;
        parent -> right = tmp;

        if (tmp)
        {
          tmp -> parent = parent;
        }

        parent -> parent = node;
        parent = node;
        //???
        gparent -> left = node;
        node -> parent = gparent;
        tmp = parent -> right;
      }

      /*
       *     G        P
       *    / \      / \
       *   p   U -> n   g
       *  /              \
       * n                U
       *
       */

      parent -> right = gparent;
      gparent -> left = tmp;

      if (tmp)
      {
        tmp -> parent = gparent;
      }
      parent -> color = RB_BLACK;
      gparent -> color = RB_RED;

      struct rb_node *old = gparent -> parent;
      gparent -> parent = parent;
      parent -> parent = old;
      if (old)
      {
        if (old -> left == gparent)
          old -> left = parent;
        else if (old -> right == gparent)
          old -> right = parent;
      }
      else
      {
        root -> root = parent;
      }
      break;
    }
    else
    {
      tmp = gparent -> left;
      if (tmp && tmp -> color == RB_RED)
      {
       /*
        *   G         g
        *  / \       / \
        * u   p  -> U   P
        *      \         \
        *       n         n
        *
        *
        */
        //case 3
        tmp -> color = RB_BLACK;
        parent -> color = RB_BLACK;
        gparent -> color = RB_RED;
        node = gparent;
        parent = RB_PARENT(node);

        continue;
      }

      //elses
      tmp = parent -> left;
      if (node == tmp)
      {
       /*
        *    G        G
        *   / \      / \
        *  U   P -> U   n
        *     /          \
        *    n            p
        *
        */
        //case 4
        tmp = node -> right;
        node -> right = parent;
        parent -> left = tmp;

        if (tmp)
        {
          tmp -> parent = parent;
        }

        parent -> parent = node;
        parent = node;
        //???
        gparent -> right = node;
        node -> parent = gparent;
        tmp = parent -> left;
      }

      /*
       *   G          P
       *  / \        / \
       * U   p  ->  g   n
       *      \    /     
       *       n  U      
       *
       */

      parent -> left = gparent;
      gparent -> right = tmp;

      if (tmp)
      {
        tmp -> parent = gparent;
      }

      parent -> color = RB_BLACK;
      gparent -> color = RB_RED;

      struct rb_node *old = gparent -> parent;
      gparent -> parent = parent;
      parent -> parent = old;
      if (old)
      {
        if (old -> left == gparent)
          old -> left = parent;
        else if (old -> right == gparent)
          old -> right = parent;
      }
      else
      {
        root -> root = parent;
      }
      break;
    }
  }
}

void
rb_link_node(struct rb_node *new, struct rb_node *parent, struct rb_node **link);
{
  new -> parent = parent;
  *link = new;

  new -> color = RB_RED;
  new -> left = new -> right = NULL;
}
