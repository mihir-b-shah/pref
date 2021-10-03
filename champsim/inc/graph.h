#ifndef GRAPH_H
#define GRAPH_H

#include <utility> // for std::pair
#include <string>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
 
enum term_type {CONST, REG, ADDR};

// The properties of the vertex
// value stands for the op code when ty == NONTERM
// value stands for a constant/addr/reg otherwise
struct VertexProperty{

    term_type ty;
    uint64_t source;

};

struct EdgeProperty {
};

// The type for our graph.
// It uses an adjacency list implementation
typedef boost::adjacency_list<boost::vecS, boost::listS,       
            boost::bidirectionalS, VertexProperty, EdgeProperty> Graph;

// Types for vertex descriptor and edge descriptor
typedef boost::graph_traits<Graph>::vertex_descriptor vertex_descriptor_t;
typedef boost::graph_traits<Graph>::edge_descriptor edge_descriptor_t;

// A trivial predicate about edges that always returns true 
template <class edge_descriptor_t>
struct edge_predicate {
    bool operator() (const edge_descriptor_t& e) const {
      return true;
    }
};

// Creates a graph
Graph graph_create();

// Add vertex with source [source] and type [t] to graph [g]
// returns the vertex descriptor of the newly added vertex
vertex_descriptor_t add_vertex(Graph * g, uint64_t source, term_type t);

// Add a directed edge to the graph [g]
// Source of the edge is [src] and destination of the edge is [dst]
// Returns a pair of edge descriptor and boolean. The Boolean 
// describes whether the edge is successfully added
std::pair<edge_descriptor_t, bool> add_edge(Graph * g, vertex_descriptor_t src, vertex_descriptor_t dst);

// Find the sources of [v] in [g]
// Returns the vertices in a vector
std::vector<vertex_descriptor_t> find_source_vertices(Graph * g, vertex_descriptor_t v);

VertexProperty get_source_property(Graph g, vertex_descriptor_t target);

// Find the nonterminal source vertex of the input [target] in [g]
// As a pre-condition, there is at most one such source
vertex_descriptor_t get_nonterm_source(Graph * g, vertex_descriptor_t target);

// Find the first source vertex of the input [target] in [g]
vertex_descriptor_t get_first_source(Graph * g, vertex_descriptor_t target);

// Find the target of [v] in [g]
vertex_descriptor_t get_target(Graph * g, vertex_descriptor_t v);

// Count number of characters needed for number, +1 if negative
int numlen(uint64_t n);

// print out the graph in matrix from
void print_vertices(Graph *g);

// Remove edges in [g] whose source and target are the same vertex
void remove_self_edge(Graph * g);

// Transform type to string
std::string ty_to_string(term_type t);

// Print out of vertex properties
void print_vertex_property(VertexProperty p);

void store_load_bypassing(Graph *g, vertex_descriptor_t root);

void remove_vertex_in_func(Graph *g, vertex_descriptor_t v, vertex_descriptor_t root);

#endif
