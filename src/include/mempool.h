
#ifndef INCLUDE_MEMPOOL_H
#define INCLUDE_MEMPOOL_H

#include "common.h"

#define _GNU_SOURCE
#include <sched.h>
#include <numa.h>
#include <numaif.h>
#include <pthread.h>
#include <sys/mman.h>

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


#endif  /* INCLUDE_MEMPOOL_H */