#ifndef __LIST_H__
#define __LIST_H__

#include <common.h>
#include <assert.h>

#define OFFSET_OF(type, mbr) (int32)(&((type*)0)->mbr)

#define CONTAINER_OF(ptr, type, mbr) ({\
    const typeof( ((type *)0)->mbr ) *__mptr = (ptr);\
    (type *)( (int8 *)__mptr - OFFSET_OF(type,mbr) );})

#define FOR_EACH_LIST(it, head)\
    for(it = head; it->next; it = it->next)

struct slist_node
{
  struct slist_node *next;
};

struct slist_root
{
  struct slist_node *slist_node;
};

struct dlist_node
{
  struct dlist_node *next;
  struct dlist_node *prev;
};

struct dlist_root
{
  struct dlist_node *dlist_node;
};

void
slist_insert_head(struct slist_root *head, struct slist_node *new_head);

void
slist_insert_after(struct slist_node *prev, struct slist_node *cur);

void
dlist_insert_head(struct dlist_root *head, struct dlist_node *new_head);

struct dlist_node*
dlist_get_tail(struct dlist_node *from);

#endif