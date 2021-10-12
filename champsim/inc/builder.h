
#ifndef BUILDER_H
#define BUILDER_H

#include <deque>
#include <unordered_set>
#include "graph.h"
#include "instruction.h"

class O3_CPU;
struct DFGraph {
  std::deque<ooo_model_instr> window;
  Graph g;
  vertex_descriptor_t root;
  std::unordered_set<uint64_t> misses;
  O3_CPU* cpu;

  DFGraph(O3_CPU* cpu){ this->cpu = cpu; }

  void build_graph(uint64_t);
};

// callback- for DFGraph=void*
class CACHE;
void update_graph(CACHE*, void*);

#endif
