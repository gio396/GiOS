#include <common.h>

#include <list.h>


struct task_struct 
{
  uint32 kernel_stack;
  uint32 kernel_esp0;
  uint32 kernel_ss0;

  struct dlist_node node;
};