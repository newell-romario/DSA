# Data Structures and Algorithms Library (DSA)

A comprehensive C library implementing common data structures and algorithms with a focus on performance, flexibility, and usability.

## Features

This library provides robust implementations of the following data structures and algorithms:

### Data Structures
- **Stack** (`r2_stack.h`, `r2_arrstack.h`)
  - Both linked list and array-based implementations
  - LIFO (Last-In-First-Out) operations
  
- **Queue** (`r2_queue.h`)
  - FIFO (First-In-First-Out) implementation
  - Efficient enqueue and dequeue operations
  
- **Deque** (`r2_deque.h`)
  - Double-ended queue implementation
  - Supports insertion and deletion at both ends

- **Ring Buffer** (`r2_ring.h`)
  - Circular buffer implementation
  - Overwrites oldest elements when full
  - FIFO policy with fixed size

- **Trees**
  - AVL Tree (`r2_avltree.h`)
  - Red-Black Tree (`r2_rbtree.h`)
  - WAVL Tree (`r2_wavltree.h`)
  - Binary Tree (`r2_btree.h`)
  - All trees support standard operations (insert, delete, search)

- **Hash Tables** (`r2_hash.h`)
  - Multiple hash function implementations (WEE, KNUTH, FNV, DBJ)
  - Collision resolution strategies:
    - Separate chaining
    - Robin Hood hashing
  - Dynamic resizing capabilities

- **Graph** (`r2_graph.h`)
  - Comprehensive graph data structure
  - Supports both directed and undirected graphs
  - Graph algorithms implementation

- **Lists** (`r2_list.h`)
  - Linked list implementation
  - Basic list operations and manipulations

- **Union Find** (`r2_unionfind.h`)
  - Disjoint set data structure
  - Efficient union and find operations

### Algorithms

- **Graph Algorithms**
  - Breadth-First Search (BFS)
  - Depth-First Search (DFS)
  - Dijkstra's Shortest Path
  - Bellman-Ford Algorithm
  - Kruskal's Minimum Spanning Tree
  - Prim's Minimum Spanning Tree
  - Topological Sort
  - Cycle Detection
  - Articulation Points
  - Bridge Finding

- **Sorting Algorithms** (`r2_sort.h`)
  - Various sorting implementations
  - Performance optimized
  - In-place and stable sorting options

- **String Algorithms** (`r2_string.h`)
  - Pattern matching algorithms
  - Rabin-Karp algorithm
  - KMP algorithm
  - Naive pattern matching

## Implementation Details

- Written in pure C
- Callback function support for custom operations
- Memory management utilities
- Type-safe implementations





### Callback Functions

The library uses callback functions for customization:
```c
typedef void* (*r2_cpy)(const void *);    // For copying elements
typedef r2_int16 (*r2_cmp)(const void *, const void*);  // For comparing elements
typedef void (*r2_fd)(void *);     // For freeing data
typedef void (*r2_fk)(void *);     // For freeing keys
```

## Building the Project

This project uses CMake as its build system. To build the library:

1. Create a build directory:
```bash
mkdir build && cd build
```

2. Configure CMake:
```bash
cmake ..
```

3. Build the project:
```bash
cmake --build .
```

