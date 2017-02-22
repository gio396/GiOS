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