#include "list.h"

void
dlist_insert_head(struct dlist_root *head, struct dlist_node *new_head)
{
  new_head->prev = NULL;

  if (head->dlist_node != NULL)
  {
    new_head->next = head->dlist_node;
    head->dlist_node->prev = new_head;
  }

  head->dlist_node = new_head;
}

void
slist_insert_head(struct slist_root *head, struct slist_node *new_head)
{
  new_head->next = head->slist_node;
  head->slist_node = new_head;
}

void
slist_insert_after(struct slist_node *prev, struct slist_node *in)
{
  if(prev)
  {
    in->next = prev->next;
    prev->next = in;
  }
}

struct dlist_node*
dlist_get_tail(struct dlist_node *from)
{
  assert1(from);

  while(from->next)
    from = from->next;

  return from;
}

void
dlist_insert(struct dlist_node *prev, struct dlist_node *new)
{
  new -> next = prev -> next;
  if (new -> next) new -> next -> prev = new;

  new -> prev = prev;
  prev -> next = new;
}

void
dlist_insert_tail(struct dlist_root *root, struct dlist_node *new)
{
  if (root -> dlist_node == NULL)
  {
    root -> dlist_node = new;
    return;
  }

  struct dlist_node *tail = dlist_get_tail(root -> dlist_node);
  dlist_insert(tail, new);
}