# NUMA Data Plane
NUMA Data Plane is my attempt to create a NUMA aware data plane to better understand performance optimization on NUMA architectures. I've compiled a [list of helpful resources](docs/sources.md) that I used to complete this project; I would encourage anyone interested to explore these resources, especially if you're planning a similar project. Prior to beginning this project, I had a decent amount of experience with DPDK; as a grad student, I worked on an interesting research project for high-throughput data transmission in Cloud and HPC contexts, while maximizing higher level application usage of resources using interoperable convergence instead of overlay.

I'm very passionate about Operating Systems, CPU/GPU architecture, and performance engineering. I love to learn and that is why I started this project.

## Avoiding malloc()

## Abstraction Layer (Work In Progress)

### Memory Pooling

#### Memory Alignment
#### Fragmentation Avoidance
#### Group Sizing Algorithm

### Service APIs

#### Allocation
#### Shared Resources
#### Kernel Bypass & Zero Copy

## Optimizations

### Huge Pages
### Memory Pinning
### Direct Memory Access (DMA)

