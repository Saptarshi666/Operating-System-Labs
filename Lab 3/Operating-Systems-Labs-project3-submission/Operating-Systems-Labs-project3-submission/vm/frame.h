#ifndef VM_FRAME_H
#define VM_FRAME_H

#include <stdint.h>
#include "filesys/off_t.h"
#include "hash.h"
#include "threads/palloc.h"
#include "threads/thread.h"
struct struct_frame {
  void* page;             // real page
  struct thread* thread;  // owner thread
  uint32_t* pte;          // Page Table Entry of the frame's page
  void* vaddr;            // address of page
  struct hash_elem hash_elem;
  struct lock llock;        // lock for sync access to frame_pages list
  struct list frame_pages;  // list of pages sharing same frame
  struct list_elem page_elem;
  int pin;
  int num;
};

void vm_frame_init(void);
struct struct_page* get_page_from_frame(void* fr, uint32_t* page_dir);
bool set_page_in_frame(void* f, struct struct_page* p);
unsigned frame_hash(const struct hash_elem* e_, void* aux UNUSED);
bool frame_comparator(const struct hash_elem* first_,
                      const struct hash_elem* second_,
                      void* aux UNUSED);
static void delete_frame(struct struct_frame* f);
static struct struct_frame* find_frame(void* pg);
static bool add_frame(void* pg);
void* get_frame(enum palloc_flags flags);
void* frame_lookup(off_t block_id);
bool set_frame(void* f, struct struct_page* p);
void free_frame(void* address, uint32_t* page_dir);
void pin(void* address);
void unpin(void* address);
static bool find_page_to_evict(struct struct_frame* f);
static struct struct_frame* get_next_page_for_eviction(void);
static void circular_evict(void);
static void move_to_next_eviction(void);
static void remove_evict_pointer(struct struct_frame* frame_to_evict);
#endif
