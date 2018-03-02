#include "rbtree.h"

#include <string.h>

void
rbtree_insert(struct rb_root *root, struct rb_node *node)
{
  struct rb_node *parent = RB_GET_PARENT(node), *gparent, *tmp;

  while (1)
  {
    if (!parent)
    {
      /*
       *  n is root
       */
      //case 1
      RB_SET_COLOR(node, RB_BLACK);
      break;
    } 
    else if (RB_GET_COLOR(parent) == RB_BLACK)
    {
      /*
       *  P
       *  |
       *  n
       */
      //case 2
      break;
    }

    gparent = RB_GET_PARENT(parent);

    tmp = gparent -> right;
    if (parent != tmp)
    {
      if (tmp && RB_GET_COLOR(tmp) == RB_RED)
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
        RB_SET_COLOR(tmp, RB_BLACK);
        RB_SET_COLOR(parent, RB_BLACK);
        RB_SET_COLOR(gparent, RB_RED);

        node = gparent;
        parent = RB_GET_PARENT(node);

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
          RB_SET_PARENT(tmp, parent);
        }

        RB_SET_PARENT(parent, node);
        parent = node;
        //???
        gparent -> left = node;
        RB_SET_PARENT(node, gparent);
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
        RB_SET_PARENT(tmp, gparent);
      }
      
      RB_SET_COLOR(parent, RB_BLACK);
      RB_SET_COLOR(gparent, RB_RED);

      struct rb_node *old = RB_GET_PARENT(gparent);
      RB_SET_PARENT(gparent, parent);
      RB_SET_PARENT(parent, old);
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
      if (tmp && RB_GET_COLOR(tmp) == RB_RED)
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
        RB_SET_COLOR(tmp, RB_BLACK);
        RB_SET_COLOR(parent, RB_BLACK);
        RB_SET_COLOR(gparent, RB_RED);
        node = gparent;
        parent = RB_GET_PARENT(node);

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
          RB_SET_PARENT(tmp, parent);
        }

        RB_SET_PARENT(parent, node);
        parent = node;
        //???
        gparent -> right = node;
        RB_SET_PARENT(node, gparent);
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
        RB_SET_PARENT(tmp, gparent);
      }

      RB_SET_COLOR(parent, RB_BLACK);
      RB_SET_COLOR(gparent, RB_RED);

      struct rb_node *old = RB_GET_PARENT(gparent);
      RB_SET_PARENT(gparent, parent);
      RB_SET_PARENT(parent, old);
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
rb_link_node(struct rb_node *new, struct rb_node *parent, struct rb_node **link)
{
  *link = new;

  ZERO_STRUCT(new, struct rb_node);
  RB_SET_PARENT_COLOR(new, parent, RB_RED);
}
