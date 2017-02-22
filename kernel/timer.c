#include "timer.h"

#include <arch/x86/irq.h>
#include <arch/x86/pit.h>
#include <arch/x86/apic.h>
#include <arch/x86/framebuffer.h>
#include <arch/x86/page.h>

#include <memory.h>
#include <list.h>

#define MAX_TIMERS  4096 / sizeof(struct timer_list_entry)
#define BITMAP_SIZE MAX_TIMERS >> 2

global uint32 a = 0;
global uint32 bit_map[BITMAP_SIZE];

struct
{
  uint8 *mem_start;
  struct slist_root head;
} main_queue;

void
timer_handler(/*regs*/);
  
void
queue_sub_timer(uint32 passed)
{
  struct slist_node *it, *head;
  struct slist_root *head_root;

  head_root = &main_queue.head;
  head = head_root->slist_node;

  FOR_EACH_LIST(it, head)
  { 
    struct timer_list_entry *tle = CONTAINER_OF(it, struct timer_list_entry, node);

    if (tle -> timer < passed)
    {
      tle -> timer = 0;
    }
    else
    {
      tle -> timer -= passed;
    }
  }
}

struct timer_list_entry *
allocate_new_entry()
{
  uint32 i, zr;

  zr = 0;  

  for (i = 0; i < BITMAP_SIZE; i++)
  {
    for (zr = 0; zr < 32; zr++)
    {
      if ((bit_map[i] & (0x1 << zr)) == 0)
      {
        bit_map[i] |= (0x1 << zr);
        return &((struct timer_list_entry*)main_queue.mem_start)[(i << 5) + zr];
      }
    }
  }

  return NULL;
}

void
queue_free(struct timer_list_entry *entry)
{
  uint32 diff, indx,bit;

  diff = entry - (struct timer_list_entry*)main_queue.mem_start;
  indx = diff >> 5;
  bit = diff & 31;


  bit_map[indx] ^= ((0x1) << bit);
}

void
timer_init()
{
  main_queue.head.slist_node = NULL;
  main_queue.mem_start = kalloc();
  memset(bit_map, 0x00, BITMAP_SIZE * sizeof(uint32));

  irq_set_handler(0, timer_handler);
}

struct slist_node *
queue_find_loc(struct slist_node *entry)
{
  struct slist_root head_root;
  struct slist_node *it, *prev, *head;
  struct timer_list_entry *in_tle, *tle;

  head_root = main_queue.head;
  head = head_root.slist_node;

  in_tle = CONTAINER_OF(entry, struct timer_list_entry, node); 

  FOR_EACH_LIST(it, head)
  {
    tle = CONTAINER_OF(it, struct timer_list_entry, node);

    if (in_tle -> timer < tle -> timer)
    {
      return prev; 
    }

    prev = it;
  }

  return it;
}

void
queue_add_timer(struct timer_list_entry entry)
{
  struct slist_root *head_root;
  struct slist_node *prev, *new_entry, *head;
  struct timer_list_entry *tle_queue_entry, *tle_head;
  uint16 count;
  uint32 passed;

  tle_queue_entry = allocate_new_entry();
  new_entry = &tle_queue_entry->node;

  if (new_entry)
  {
    head_root = &main_queue.head;
    head = head_root->slist_node; 

    if (head != NULL)
    {
      memcpy(&entry, tle_queue_entry, sizeof(struct timer_list_entry));
      
      prev = queue_find_loc(new_entry);

      if (prev == NULL)
      {
        count = pit_get_current_count();

        if (count < tle_queue_entry->timer)
        {
          prev = head;
        }
        else
        {
          tle_head = CONTAINER_OF(head, struct timer_list_entry, node);

          passed = tle_head->timer - count;
          queue_sub_timer(passed);

          slist_insert_head(head_root, new_entry);
          pit_interrupt_in(tle_queue_entry->timer);
        }
      }

      slist_insert_after(prev, new_entry);
    }
    else
    { 
      new_entry->next = NULL;

      head_root->slist_node = new_entry;
      pit_interrupt_in(tle_queue_entry->timer);
    }
  }
}

void
timer_handler(/*regs*/)
{
  struct slist_root *head_root;
  struct slist_node *head;
  struct timer_list_entry* tle_cur;
  uint32 passed;

  head_root = &main_queue.head;
  head = head_root->slist_node;

  if(head == NULL)
    return;

  tle_cur = CONTAINER_OF(head, struct timer_list_entry, node);

  //handle this timer

  passed = tle_cur->timer;
  queue_free(tle_cur);

  head_root->slist_node = head->next;
  head = head_root->slist_node;

  if(head != NULL)
  {
    queue_sub_timer(passed);

    while(head && tle_cur->timer == 0)
    {
      tle_cur = CONTAINER_OF(head, struct timer_list_entry, node);

      head_root->slist_node = head->next;
      queue_free(tle_cur);

      head = head_root->slist_node;
    }

    tle_cur = CONTAINER_OF(head, struct timer_list_entry, node);

    if (head)
      pit_interrupt_in(tle_cur->timer);
  }
}