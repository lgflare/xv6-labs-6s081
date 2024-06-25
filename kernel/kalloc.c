// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
  uint8 mem_ref_cnt[(PHYSTOP - KERNBASE) / PGSIZE];
} kmem;

void init_ref(uint64 pa) {
  //acquire(&kmem.lock);
  kmem.mem_ref_cnt[(pa - KERNBASE) / PGSIZE] = 1;
  //release(&kmem.lock);
  //printf("pa %p ref %x\n", pa, mem_ref_cnt[(pa - KERNBASE) / PGSIZE]);
}

void increase_ref(uint64 pa) {
  //printf("ref %d\n", mem_ref_cnt[(pa - KERNBASE) / PGSIZE]);
  if (search_ref(pa) == 255)
    panic("too many refs");
  acquire(&kmem.lock);
  kmem.mem_ref_cnt[(pa - KERNBASE) / PGSIZE] += 1;
  //printf("increase pa %p ref %x\n", pa, kmem.mem_ref_cnt[((uint64)pa - KERNBASE) / PGSIZE]);
  release(&kmem.lock);
}

void decrease_ref(uint64 pa) {
  //printf("array addr = %p\n", &mem_ref_cnt);
  if (kmem.mem_ref_cnt[(pa - KERNBASE) / PGSIZE] != 0)
  {
    //acquire(&kmem.lock);
    kmem.mem_ref_cnt[(pa - KERNBASE) / PGSIZE] -= 1;
    //release(&kmem.lock);
  }
  //printf("pa %p ref cnt %d\n", pa, mem_ref_cnt[(pa - KERNBASE) / PGSIZE]);
}

uint8 search_ref(uint64 pa) {
  return kmem.mem_ref_cnt[(pa - KERNBASE) / PGSIZE];
}

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  //printf("start: %p end: %p\n", pa_start, pa_end);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
  { 
    //printf("p: %p\n", p);
    kfree(p);
  }
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;
  //uint32 idx = ((uint64)pa - KERNBASE) / PGSIZE;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  //printf("pa %p ref %x\n", pa, search_ref((uint64)pa));
  //if (search_ref((uint64)pa) <= 1)
  acquire(&kmem.lock);
  decrease_ref((uint64)pa);
  if (search_ref((uint64)pa) == 0)
  {
    release(&kmem.lock);
    memset(pa, 1, PGSIZE);
    r = (struct run*)pa;
    acquire(&kmem.lock);
    r->next = kmem.freelist;
    kmem.freelist = r;
    release(&kmem.lock);
  } else 
    release(&kmem.lock);

  /*
  acquire(&kmem.lock);
  decrease_ref((uint64)pa);
  release(&kmem.lock);
  if (search_ref((uint64)pa) == 0)
  {
    memset(pa, 1, PGSIZE);
    r = (struct run*)pa;
    acquire(&kmem.lock);
    r->next = kmem.freelist;
    kmem.freelist = r;
    release(&kmem.lock);
  }
   */
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;
  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r) {
    kmem.mem_ref_cnt[((uint64)r - KERNBASE) / PGSIZE] = 1;
    kmem.freelist = r->next;
  }
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  //printf("r = %p\n", r);
  return (void*)r;
}