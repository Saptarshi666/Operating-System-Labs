#ifndef VM_SWAP_H
#define VM_SWAP_H

#include <stdbool.h>
#include <stddef.h>

void init_swap();
size_t store_swap_page(void* address);
void load_swap_page(size_t ind, void* address);
void free_swap_page(size_t ind);
#endif