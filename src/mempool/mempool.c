/*******************************************************************************
 * @file               mempool.c
 * @brief              Memory pool managed by the abstraction layer for process storage.
 * @author             Maurice Green
 * @date               November 16, 2025
 * @copyright          (C) 2025 Trace Systems, LLC.  All rights reserved.
 *
 * @details            Mempool is the NUMA aware mempory pool created and managed
 *                     by NUMA-DP used by processes to store and load data. Each
 *                     process uses the NUMA-DP memory allocator, instead of the
 *                     traditional malloc() call. The use of NUMA-DPs allocator
 *                     instead of malloc() allows NUMA-DP to avoid heap fragmentation,
 *                     or page faults. Memory is pinned to NUMA nodes to avoid page
 *                     faults and provide fast allocations.  Fixed-size blocks of
 *                     memory chunks/buffers are created from the initial pool, to
 *                     optimize allocation requests by reducing internal fragmentation.
 * 
 *                     An effort is made to learn per-process memory access patterns
 *                     to further optimize by reducing external fragmentation.
 * 
 *                     Zero-Copy and Kernel bypass interact with DMA controllers using 
 *                     XDP-like operations to support fast transfer of data.
 * 
 *                     Other common Data Plane optimizations are used, such as huge pages
 *                     ---
 *                     NUMA Aware
 * 
 *                     On NUMA architectures, all memory is not equal. Memory accesses that
 *                     are physicaly further aware from the executing CPU will take longer,
 *                     than those close to the CPU.  Traditional memory allocators provide
 *                     no control over where memory is allocated. NUMA-DP colocates memory 
 *                     with the NUMA node of the executing CPU.
 * 
 *                     This is achieved by forcing scheduled threads to request NUMA node
 *                     access. Furthermore, if NUMA nodes are exposed to the ACPI tables,
 *                     NUMA-DP can remain NUMA aware in virtual contexts.
 * 
 *                     Furthermore NUMA awareness is not limit to application data structures,
 *                     The entirety of the converged service API is NUMA aware, so even 
 *                     memory allocated for internal, NUMA-DP structures is colocated with the 
 *                     NUMA node in proximity to the CPU of execution.
 *
 * @revision           November 16, 2025 - Maurice Green - init
 ******************************************************************************/

#include "mempool.h"


/** Get Pool Size
 *  Get target pool size based on total memory
 *  
 *  @brief using sysconf() to get pages and page_size, take the product of these 
 *         variables and multiply the fraction to get the target pool size.
 *  
 *  @param fraction - a percentage multiplier representing the fraction of total memory
 *                     that should be pooled.
 * 
 *  @return size_t target - total memory * fraction
 */
static size_t get_pool_size(double fraction)
{
    long pages = sysconf(_SC_PHYS_PAGES);
    long page_size = sysconf(_SC_PAGESIZE);
    if (pages <= 0 || page_size <= 0)
        return 0;
    
    double total = (double)pages * (double)page_size;
    double target = total * fraction;
    return (size_t)target;
}


static bool check_numa(void)
{
    bool ret = true;

    if (numa_available() != 0) {
        printf("NUMA is not supported!\n");
        ret = false;
    }
    return ret;
}

int ndp_bind_thread_to_node(int node)
{
    int rc;
    struct bitmask *bm = numa_allocate_cpumask();
    if (!bm)
        return -1;
    numa_bitmask_clearall(bm);
    if (numa_node_to_cpus(node, bm) != 0) {
        numa_free_cpumask(bm);
        return -1;
    }
    if ((rc = numa_sched_setaffinity(0, bm)) == 0)
        goto affinity_set;
    
    goto affinity_unset;

affinity_set:
    numa_free_cpumask(bm);
    return 0;
    
affinity_unset:
    perror("Failed to set mask affinity for NUMA Node");
    numa_free_cpumask(bm);
    return -1;
}

static void warm_pages(uint8_t *base, size_t size)
{
    long page = sysconf(_SC_PAGESIZE);
    if (page < 0)
        page = 4096
    
    /* make sure compiler does not optimize this because it will be
    used for base address */
    volatile uint8_t *p = base;
    for (size_t off = 0; off < size; off++)
        p[off] = 0;

}

/**
 * params - args is a void pointer because pthread_create() requires
 *          the start_routine to have this form: *(*start_routine)(void *)
 */
static void *ndp_node_init_thread(void *args)
{
    struct thread_args *a = args;

    if (ndp_bind_thread_to_node(a->node) < 0)
        goto thread_exit1;
    
    void *addr = numa_alloc_onnode(a->size, a->node);
    if (!addr)
        goto thread_exit1;

    if (mlock(addr, a->size)) {
        numa_free(addr, a->size);
        goto thread_exit1;
    }

    warm_pages(addr, a->size);

    a->pool->base = addr;
    a->pool->size = a->size;
    a->pool->offset = 0;
    a->pool->node = a->node;

    goto thread_exit2;

thread_exit1: // pthread_exit takes void*
    pthread_exit((void *)(intptr_t)-1);

thread_exit2:
    pthread_exit((void *)(intptr_t)0);
}


int ndp_mempool_init(struct mempool_sys *sys)
{
    if (numa_available() < 0)
        return -1;

    int nfree; int nsize;
    int nnodes = numa_max_node();
    if ((nsize = numa_node_size(0, &nfree)) == -1)
        return -1;
    
    sys->num_nodes = nnodes;
    sys->pool_size = nsize;
    sys->pools = calloc(sys->num_nodes, sizeof(sys->pool_size));
    if (!sys->pools)
        return -1;

    
    pthread_t *threads = calloc(sys->num_nodes, sizeof(pthread_t));
    struct thread_args *t_args = calloc(sys->num_nodes, sizeof(struct thread_args));

    for (int node = 0; node < sys->num_nodes; node++)
    {
        t_args[node].pool = &sys->pools[node];
        t_args[node].node = node;
        t_args[node].size = &sys->pool_size;

        if(pthread_create(&threads[node], NULL, ndp_node_init_thread, 
                                                        &t_args[node]) != 0)
        {
            // sometimes enabled NUMA nodes can be sparse. If numa_max_node() returns
            // 3 this routine assumes the enabled nodes are 0, 1, 2. However, it's 
            // certainly possible that 0 2 and 5 are the enabled nodes and 1 is not.
            // there are better ways of creating threads, but I'm keeping it simple 
            // for now, so failed thread creation does break out of loop; rather,
            // the loop continues to numa_max_node() + 1.
            continue;
        }
    }

    int ok = 0;
    for (int node = 0; node < sys->num_nodes; node++)
    {
        void *retval = NULL;
        pthread_join(threads[node], &retval);
        if ((intptr_t)retval != 0)
            ok = -1;
    }

    free(threads);
    free(args);

    if (ok != 0) {
        for (int node = 0; node < sys->num_nodes; node++) {
            if (sys->pools[node].base) {
                munlock(sys->pools[node].base, sys->pool_size);
                numa_free(sys->pools[node].base, sys->pools[node].size);
            }
        }
        free(sys->pools);
        sys->pools = NULL;
        sys->num_nodes = 0;
        return -1;
    }
    return 0;
}

void *ndp_mempool_alloc_on_node(struct mempool_sys *sys, int node, size_t align)
{
    if (node < 0 || node >= sys->num_nodes)
        return NULL;
    
    int size = sys->pool_size;
    struct mempool_node *p = &sys->pools[node];

    size_t a = align ? align : 1;
    size_t off = align_up(p->offset, a);
    if (off + size > p->size)
        return NULL;

    void *ptr = p->base + off;
    p->offset = off + size;
    return ptr;
}

void mempool_system_destroy(struct mempool_sys *sys)
{
    if (!sys->pools)
        return;

    for (int node = 0; node < sys->num_nodes; node++) {
        if (!sys->pools[node]->base)
            continue;
        munlock(sys->pools[node]->base, sys->pools[node]->size);
        numa_free(sys->pools[node]->base, sys->pools[node]->size);
    }
    free(sys->pools);
    sys->pools = NULL;
    sys->num_nodes = 0;
    sys->pool_size = 0;
}

int ndp_bind_worker_node(int node)
{
    return ndp_bind_thread_to_node(node);
}


static void ndp_allocate_fixed_blocks(unsigned int a, unsigned int b, unsigned int c)
{
    //
    // variables a, b, and c allow an operator to specify the number
    // of blocks in each category desired; the size block for a, b, and c
    // are predetermined and nonnegotiable.
    //
    int tracing;
    int active_numa = SOME_FUNC_TO_ACTIVE_NUMA() // TODO
#if defined(TRACE_ALLOCATION)
    tracing = 1;
#else
    tracing = 0;
#endif

    void *a_span = ndp_malloc(A_SPAN, a, active_numa, tracing);
}

/** 
 *          IMPORTANT NOTE
 * 
 * the main function below is only for testing, the function body
 * is not representative of how the program should be run.
 * 
 */
int main(void)
{
    struct mempool_sys sys;
    if (ndp_mempool_init(&sys) != 0) {
        fprintf(stderr, "ndp_mempool_init() failed.\n");
        return 1;
    }

    int node = 0; // test with node zero
    ndp_bind_worker_node(node);
    void *buf = ndp_mempool_alloc_on_node(&sys, node, 64);
    if (!buf)
        fprintf(stderr, "Alloc failed\n");
    
    mempool_system_destroy(&sys);
    return 0;
}