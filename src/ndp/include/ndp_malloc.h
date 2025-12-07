
#ifndef INCLUDE_NDP_NDP_MALLOC
#define INCLUDE_NDP_NDP_MALLOC

static void *ndp_malloc(const char *type, int no_blocks, int numa_node,
                                                    const bool trace_alloc);


#endif /* INCLUDE_NDP_NDP_MALLOC */