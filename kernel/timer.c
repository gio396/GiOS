#include "timer.h"

//revrite this mess.

#include <arch/x86/irq.h>
#include <arch/x86/framebuffer.h>
#include <arch/x86/page.h>

#include <memory.h>
#include <list.h>

#define MAX_TIMERS  4096 / sizeof(struct timer_list_entry)
#define BITMAP_SIZE MAX_TIMERS >> 2

global uint32 bit_map[BITMAP_SIZE];

struct
{
  uint8 *mem_start;
  struct slist_root head;
} main_queue;

struct
{
  uint32 irq_num;
  void (*interrupt_in)(uint32 time);
  uint32 (*timer_count)(void);
} timer_info;

void
timer_handler(/*regs*/);
  
void
timer_print_list(void)
{
  struct slist_node *head;
  struct slist_node *it;
  struct timer_list_entry *tle;

  head = main_queue.head.slist_node;

  FOR_EACH_LIST(it, head)
  {
    tle = CONTAINER_OF(it, struct timer_list_entry, node);
    printk(&state, "timer: %08X -> ", tle -> timer);
  }

  printk(&state, "NULL\n");
}

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

void
queue_sub_head_time(void)
{
  struct slist_node *head;
  struct timer_list_entry *tle;
  uint32 passed_time;

  head = main_queue.head.slist_node;

  if (head != NULL)
  {
    tle = CONTAINER_OF(head, struct timer_list_entry, node);

    passed_time = tle -> timer - timer_info.timer_count();
    queue_sub_timer(passed_time); 
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
timer_init(uint32 irq, void *interrupt_function, void *timer_count)
{
  main_queue.head.slist_node = NULL;
  main_queue.mem_start = kalloc();
  memset(bit_map, 0x00, BITMAP_SIZE * sizeof(uint32));

  timer_info.irq_num = irq;
  timer_info.interrupt_in = interrupt_function;
  timer_info.timer_count = timer_count;

  irq_set_handler(irq, timer_handler);
}

struct slist_node *
queue_find_loc(struct slist_node *entry)
{
  struct slist_root head_root;
  struct slist_node *it, *prev, *head;
  struct timer_list_entry *in_tle, *tle;

  head_root = main_queue.head;
  head = head_root.slist_node;
  prev = NULL;

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

  return prev;
}

void
queue_add_timer(struct timer_list_entry *new_entry)
{
  struct slist_node *entry_position;

  if (new_entry !=  NULL)
  {
    queue_sub_head_time();

    entry_position = queue_find_loc(&new_entry -> node);

    if (entry_position == NULL)
    {
      printk(&state, "NEW_HEAD\n");
      slist_insert_head(&main_queue.head, &new_entry -> node);

      timer_info.interrupt_in(new_entry -> timer);
    }
    else
    {
      slist_insert_after(entry_position, &new_entry -> node);
    }
  }

  timer_print_list();
}

void
timer_handler(/*regs*/)
{
  struct slist_root *head_root;
  struct slist_node *head;
  struct timer_list_entry *tle;

  head_root = &main_queue.head;
  head = head_root -> slist_node;
  tle = CONTAINER_OF(head, struct timer_list_entry, node);

  //handle this timer
  tle -> function_callback(tle -> callback_arg);

  queue_free(tle);
  queue_sub_timer(tle -> timer);

  head = head -> next;

  while (head)
  {
    tle = CONTAINER_OF(head, struct timer_list_entry, node);

    if (tle -> timer == 0)
    {
      //handle this timers too.
      tle -> function_callback(tle -> callback_arg);
      
      head = head -> next;
      queue_free(tle);

      continue;
    }

    timer_info.interrupt_in(tle -> timer);
    break;
  }

  main_queue.head.slist_node = head;
}

void
new_timer(uint32 time, timer_function_proc function, uint32 callback_arg)
{
  struct timer_list_entry *new_tle;

  new_tle = allocate_new_entry();

  new_tle -> timer = time;
  new_tle -> function_callback = function;
  new_tle -> callback_arg = callback_arg;

  queue_add_timer(new_tle);
}

