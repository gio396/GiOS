#ifndef __LIST_H__
#define __LIST_H__

#include <common.h>
#include <assert.h>

#define FOR_EACH_LIST(it, head)\
  for(it = head; it->next; it = it->next)

struct slist_header
{
  struct slist_header *next;
};

struct dlist_header
{
  struct dlist_header *prev;
  struct dlist_header *next;
};

void
dlist_insert_head(struct dlist_header *prev, struct dlist_header *cur)
{
  if(prev)
    prev->next = cur;

  cur->prev = prev;
  cur->next = NULL;
}

void
slist_insert_head(struct slist_header *prev, struct slist_header *cur)
{
  if(prev)
    prev->next = cur;
}

struct dlist_header*
dlist_get_tail(struct dlist_header *from)
{
  assert1(from);

  while(from->prev)
    from = from->prev;

  return from;
}

#define DLIST_INSERT_HEAD(prev, cur)\
  dlist_insert_head((struct dlist_header*)(prev), (struct dlist_header*)(cur))

#define SLIST_INSERT_HEAD(prev, cur)\
  slist_header((struct slist_header*)(prev), (struct slist_header*)(next))

#define DLIST_GET_TAIL(from, type)\
  (type*)dlist_get_tail((struct dlist_header*)from);

#endif