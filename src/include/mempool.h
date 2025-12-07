
#ifndef INCLUDE_MEMPOOL_H
#define INCLUDE_MEMPOOL_H

#include "common.h"

#define _GNU_SOURCE
#include <sched.h>
#include <numa.h>
#include <numaif.h>
#include <pthread.h>
#include <sys/mman.h>


#define A_SPAN          4096
#define B_SPAN          8192
#define C_SPAN          1GB
 
struct mempool_node
{
    uint8_t *base;
    size_t size;
    size_t offset;
    int node;
};


struct mempool_sys
{
    struct mempool_node *pools;
    int num_nodes;
    size_t pool_size;
};

struct thread_args
{
    struct mempool_node *pool;
    int node;
    size_t size;
};

#define align_up(x, a)              \
    ((x + a - 1) & ~(a - 1))


/* generate a number of fixed-sized blocks for size categories A, B, C */
static void ndp_allocate_fixed_blocks(unsigned int, unsigned int, unsigned int);

/* populate fixed-size blocks into registry for management */
static void ndp_fixed_block_register();

/* register fixed-size blocks for management/monitoring */
static void ndp_register_fixed_block();

/**
 * deregister fixed-size block
 * 
 * @brief deregistering a fixed-block is functionally equiv. to removing
 *        it from the active list. The block iteself is marked as un-
 *        allocated, indicating it's availability for use by other threads
 */
static void ndp_deregister_fixed_block();


#endif  /* INCLUDE_MEMPOOL_H */