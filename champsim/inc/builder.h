
#ifndef BUILDER_H
#define BUILDER_H

#include <deque>
#include <queue>
#include "graph.h"
#include "instruction.h"

class O3_CPU;
struct DFGraph {
  std::deque<ooo_model_instr> window;
  Graph g;
  vertex_descriptor_t root;
  std::queue<uint64_t> misses;
  O3_CPU* cpu;

  DFGraph(O3_CPU* cpu){ this->cpu = cpu; }

  void build_graph();
};

// callback- for DFGraph=void*
class CACHE;
void update_graph(CACHE*, void*);

#endif
