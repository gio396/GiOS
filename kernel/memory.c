#include "memory.h"

#include <macros.h>
#include <list.h>
#include <string.h>

#include <arch/x86/page.h>
#include <arch/x86/framebuffer.h>

#define DEFAULT_ALLIGNMENT 4

struct slist_root free_list;

struct memory_list_header
{
  size_t size;
  struct slist_node self;
};

struct alloc_header
{
  size_t size;
};

size_t
get_allign_offset(void *addr, size_t allignment, size_t offset)
{
  u8 *alligned_addr = (u8*)((size_t)((u8*)addr + allignment - 1) & ~(allignment - 1));
  size_t dist = alligned_addr - (u8*)addr;

  if (dist < offset)
  {
    dist += ((offset + allignment - 1) / allignment) * allignment;
  }

  return dist - offset;
}

b8
free_list_add_page()
{
  void *next_page = kalloc();

  if (!next_page)
    return 0;

  struct memory_list_header *header = (struct memory_list_header*)next_page;
  header -> size = kb(4);

  if (free_list.slist_node)
    header -> self.next = free_list.slist_node;

  free_list.slist_node = &header -> self;

  return 1;
}

void*
alligned_alloc(size_t size, size_t allignment)
{
  assert1((allignment & (allignment - 1)) == 0);
  //TODO(gio):
  assert1(size < kb(4));

  if (free_list.slist_node == NULL)
  {
    if (!free_list_add_page())
    {
      return NULL;
    }
  }

  struct slist_node *last = NULL;
  struct slist_node *it   = NULL;
  FOR_EACH_LIST(it, free_list.slist_node)
  {
    struct memory_list_header *header = CONTAINER_OF(it, struct memory_list_header, self);
    size_t offset = get_allign_offset((u8*)header, allignment, sizeof(struct alloc_header));

    if  (header -> size < size + offset)
    {
      last = it;
      continue;
    }

    struct alloc_header *ahead = (struct alloc_header*)((u8*)(header) + offset);
    ahead -> size = size + offset - sizeof(struct alloc_header);

    if (last)
    {
      last -> next = it -> next;
    }
    else
    {
      free_list.slist_node = it -> next;
    }

    return (void*)(ahead + 1);
  }

  if(!free_list_add_page())
  {
    return NULL;
  }
  
  it = free_list.slist_node;
  struct memory_list_header *header = CONTAINER_OF(it, struct memory_list_header, self);
  size_t offset = get_allign_offset((u8*)header, allignment, sizeof(struct alloc_header));

  if (header -> size < size + offset)
    return NULL;

  struct alloc_header *ahead = (struct alloc_header*)((u8*)(header) + offset);
  ahead -> size = size + offset - sizeof(struct alloc_header);

  free_list.slist_node = it -> next;
  return (void*)(ahead + 1);
}

void*
kmalloc(size_t size)
{
  return alligned_alloc(size, DEFAULT_ALLIGNMENT);
}

void*
kzmalloc(size_t size)
{
  void *ptr = alligned_alloc(size, DEFAULT_ALLIGNMENT);
  memset(ptr, 0, size);

  return ptr;
}


u64
tbeqw(u64 val)
{

  return 0;
}

u32
tbedw(u32 val)
{
  return (((val>>24) & 0x000000ff) | ((val>>8) & 0x0000ff00) | ((val<<8) & 0x00ff0000) | ((val<<24) & 0xff000000));
}

u16
tbew(u16 val)
{
  return ((val >> 8) & 0x00ff) | ((val << 8) | 0xff00);
}
