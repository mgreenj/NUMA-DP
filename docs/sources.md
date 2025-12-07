
## eXpress Data Path (XDP)

[eXress Data Path](https://dl.acm.org/doi/abs/10.1145/3281411.3281443) paper.

## Reputed Data Plane Projects

* [PF_Ring](https://www.ntop.org/products/packet-capture/pf_ring/) is an amazing resource for learning data plane development; in particular, using zero-copy and kernel bypass optimizations along with DMA for fast networking.
* [DPDK](https://doc.dpdk.org/guides/prog_guide/), including a [helpful blog post on NUMA memory](https://www.dpdk.org/memory-in-dpdk-part-1-general-concepts/). DPDK is an amazing resources for learning data plane development.
* NVIDIAs [VPA for mellanox](https://docs.nvidia.com/networking/display/mlnxofedv451010/introduction).

## Highly useful reading sources
* [Linux Kernel Page Frame Reclamation](https://www.kernel.org/doc/gorman/html/understand/understand013.html). This is very helpful to understand how linux attempts to reclaim page frames, understanding active/inactive lists, LRU lists, etc. In particular, file-backed pages can remain on active lists longer than necessary, implying less physical memory available for reclaim. In Kubernetes, this can [Kubelet's view of memory](https://github.com/kubernetes/kubernetes/issues/43916).
* [Glibc Wiki MallocInternals](https://sourceware.org/glibc/wiki/MallocInternals). This was extrodenarily helpful for understanding arenas in heap memory, and home of the implementation of malloc.

## NUMA (and other memory architecture) Resources
* [NUMA Memory architecture](https://pok.acm.org/meetings/foils/Ladd16.pdf) slides from the Poughkeepsie ACM chapter. 
* The classic paper, [What Every Programmer Should Know about Memory](https://people.freebsd.org/~lstewart/articles/cpumemory.pdf) from Ulrich Drepper.
* Lenovo's [introduction to IOMMU in the Linux kernel](https://lenovopress.lenovo.com/lp1467-an-introduction-to-iommu-infrastructure-in-the-linux-kernel).
* Linux programmers [`man` page for numa, numa(3)](https://man7.org/linux/man-pages/man3/numa.3.html), that includes all of the interface functions provided by `libnuma`.


## Performance
* One of my favorites, [Performance Analysis and Tuning on Modern CPUs](https://faculty.cs.niu.edu/~winans/notes/patmc.pdf) by Denis Bakhvalov.
* If you've never cracked this book open, you're missing out: [Systems Performance: Enterprise and Cloud](https://raw.githubusercontent.com/samlee2015jp/cs_books/main/Systems.Performance.Enterprise.and.the.Cloud.2nd.Edition.2020.12.pdf) by Brendan Gregg.
* [Benchmarking and analysis of Software Data Planes](https://lfnetworking.org/wp-content/uploads/sites/7/2022/06/benchmarking_sw_data_planes_skx_bdx_mar07_2019.pdf) - Maciek Konstantynowicz et al.


## Networking
* [Scalable Data-Intensive Network Computation](https://trace.tennessee.edu/cgi/viewcontent.cgi?article=1528&context=utk_graddiss), Huadong Liu
* OG RDMA Paper: [Remote Virtual Memory Access](https://ieeexplore.ieee.org/document/9460494), Ryan Grant et al.
* [Mathematical Foundations of Computer Networking](https://www.cl.cam.ac.uk/teaching/1213/PrincComm/mfcn.pdf), S. Keshav
* Interesting NSF FIT (Future Internet Architecture) project: [Named Data Networking](https://named-data.net/).
* Somewhat [interesting powerpoint](https://opennetworking.org/wp-content/uploads/2020/12/P4-Boot-Camp-Intro-to-P4-Chang.pdf) from the P4 Language Consortium.