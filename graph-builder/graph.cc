#include "graph.h"
#include <string>
using namespace std;
#define NO_NONTERM -100

// Problem Aug 3, the reordering of vertices after removing

Graph graph_create() {
    
    Graph graph;
    return graph;

}

vertex_descriptor_t add_vertex(Graph * g, uint64_t source, term_type t) {
    
    VertexProperty vp; 

    vp.ty = t;
    vp.source = source;
    
    vertex_descriptor_t vd = boost::add_vertex(vp, *g);

    return vd;
}

pair<edge_descriptor_t, bool> add_edge(Graph * g, vertex_descriptor_t src, vertex_descriptor_t dst){

    EdgeProperty ep;

    edge_descriptor_t ed;
    bool inserted;

    tie(ed, inserted) = boost::add_edge(src, dst, *g);
    pair<edge_descriptor_t, bool> ret (ed, inserted);

    return ret;

}

vector<vertex_descriptor_t> find_source_vertices(Graph * g, vertex_descriptor_t v) {
    
    // find in edges to target
    boost::graph_traits<Graph>::in_edge_iterator ei, ei_end;
    boost::tie(ei, ei_end) = boost::in_edges(v, *g);

    vector<vertex_descriptor_t> adjs;
    vertex_descriptor_t src;

    for (; ei != ei_end; ++ei) {
        src = boost::source(*ei, *g);
        adjs.push_back(src);
    }

    // cout << "the number of source here is " << adjs.size() << endl;
    return adjs;

}

VertexProperty get_source_property(Graph g, vertex_descriptor_t target) {

    // find in edges to target
    boost::graph_traits<Graph>::in_edge_iterator ei, ei_end;
    boost::tie(ei, ei_end) = boost::in_edges(target, g);
    // find source
    vertex_descriptor_t src = boost::source(*ei, g);
    // get property
    boost::property_map<Graph, boost::vertex_bundle_t>::type pmap = boost::get(boost::vertex_bundle, g);
    VertexProperty vp = boost::get(pmap, src);

    return vp;

}

// Find the source vertex of the input target in g
vertex_descriptor_t get_nonterm_source(Graph * g, vertex_descriptor_t target) {
    
    // find in edges to target
    boost::graph_traits<Graph>::in_edge_iterator ei, ei_end;
    boost::tie(ei, ei_end) = boost::in_edges(target, *g);
    // find source
    vertex_descriptor_t src, nonterm_src;
    VertexProperty vp;
    boost::property_map<Graph, boost::vertex_bundle_t>::type pmap = boost::get(boost::vertex_bundle, *g);
    
    for (; ei != ei_end; ++ei) {
        src = boost::source(*ei, *g);
        vp = boost::get(pmap, src);
        if (find_source_vertices(g, src).size() != 0)
            return src;
    }

    return NULL;
    
}

vertex_descriptor_t get_first_source(Graph * g, vertex_descriptor_t target) {
    
    // find in edges to target
    boost::graph_traits<Graph>::in_edge_iterator ei, ei_end;
    boost::tie(ei, ei_end) = boost::in_edges(target, *g);
    // find source
    vertex_descriptor_t src;

    src = boost::source(*ei, *g);

    return src;
    
}

vertex_descriptor_t get_target(Graph * g, vertex_descriptor_t v) {

    // find in edges to target
    // cout << "got here -1" << endl;
    boost::graph_traits<Graph>::out_edge_iterator ei, ei_end;
    // cout << "got here 0" << endl;
    boost::tie(ei, ei_end) = boost::out_edges(v, *g);
    // find source
    vertex_descriptor_t target;
    
    // cout << "got here 1" << endl;
    target = boost::target(*ei, *g);
    // cout << "got here 2" << endl;
    return target;
    
}

int numlen(uint64_t n)
{
    // count number of characters needed for number, +1 if negative
    int length = 0;
    if (n == 0)
        return 1;
    if (n < 0) {
        n = -n;
        length++;
    }
    while (n > 0) {
        length++;
        n = n / 10;
    }
    
    return length;
}

// print out the graph in matrix from
void print_vertices(Graph *g) {
    boost::graph_traits<Graph>::vertex_iterator vi, vi_end;
    boost::tie(vi, vi_end) = boost::vertices(*g);

    boost::graph_traits<Graph>::vertex_iterator ui, ui_end;

    edge_descriptor_t e;
    bool to, from;
    int i = 0;

    boost::property_map<Graph, boost::vertex_bundle_t>::type pmap = boost::get(boost::vertex_bundle, *g);
    VertexProperty vp;

    for (; vi != vi_end; vi++) {
        vp = boost::get(pmap, *vi);
        if (i == 0)
            cout << std::setw(7) << vp.source;
        else
            cout << std::setw(6) << vp.source;
        i++;
    }
    i = 0;
    cout << endl;

    boost::tie(vi, vi_end) = boost::vertices(*g);
    for (; vi != vi_end; vi++) {
        vp = boost::get(pmap, *vi);
        cout << vp.source;
        boost::tie(ui, ui_end) = boost::vertices(*g);

        for (; ui != ui_end; ui++) {
            boost::tie(e, to) = boost::edge(*ui, *vi, *g);
            boost::tie(e, from) = boost::edge(*vi, *ui, *g);
            if (i == 0) {
                if (to || from)
                    cout << std::setw(7 - numlen(vp.source)) << 1;
                else 
                    cout << std::setw(7 - numlen(vp.source)) << 0;  
            }
            else {
                if (to || from)
                    cout << std::setw(6) << 1;
                else 
                    cout << std::setw(6) << 0; 
            }
            i++;
        }
        i = 0;
        
        cout << endl;
    }

}

void remove_self_edge(Graph * g) {
    boost::graph_traits<Graph>::vertex_iterator vi, vi_end;
    boost::tie(vi, vi_end) = boost::vertices(*g);

    // self-edge. Don't understand why it exists yet
    edge_descriptor_t start_self;
    bool start_self_exists;

    for (; vi != vi_end; vi++) {
        boost::tie(start_self, start_self_exists) = boost::edge(*vi, *vi, *g);
        if (start_self_exists) {
            // cout << "removed self edge for " << *vi << endl;
            boost::remove_edge(start_self, *g);
        }
    }
}

string ty_to_string(term_type t) {
    switch (t) {
    case REG:
        return "REG";
    
    case ADDR:
        return "ADDR";

    case CONST:
        return "CONST";

    default:
        return "ERROR";
    }
}

void print_vertex_property(VertexProperty p) {
    cout << "source: " << p.source << " type: " << ty_to_string(p.ty) << endl;
}

void store_load_bypassing(Graph *g, vertex_descriptor_t root) {

    int num_sources;
    // The vector to keep track of the vertices along the path
    deque<vertex_descriptor_t> circle;
    // find the first non-terminal source
    vertex_descriptor_t start;
    // the current place on the path
    vertex_descriptor_t cur = root;
    // the next place in the path
    vertex_descriptor_t next;
    // properties of the vertices
    VertexProperty cur_property;
    VertexProperty start_property;
    VertexProperty next_property;
    // properties for testing
    VertexProperty p;
    // self-edge. Don't understand why it exists yet
    edge_descriptor_t start_self;
    bool start_self_exists;
    // edge iterators for the current vertex
    boost::graph_traits<Graph>::edge_iterator ei, ei_end;
    // property map to easily access the properties
    boost::property_map<Graph, boost::vertex_bundle_t>::type pmap = boost::get(boost::vertex_bundle, *g);
    // Predicate for removing edge
    edge_predicate<edge_descriptor_t> pred;

    // search up along the path
    while (circle.empty()) {
        // cout << "TO find another circle\n";
        while (1) {
            // cout << "Loop!\n";
            num_sources = find_source_vertices(g, cur).size();
            // when we are at an ADD node
            if (num_sources == 2) {
                // cout << "two source!" << endl;
                // check if the ADD node has a child who is a nonterm
                cur = get_nonterm_source(g, cur);

                // print the current vertex
                // cout << "current: ";
                cur_property = boost::get(pmap, cur);
                // print_vertex_property(cur_property);

                if (cur == NULL) 
                    break;
                else if (find_source_vertices(g, cur).size() == 1) {
                    start = cur;               
                    circle.push_back(start);
                }
            }
            // when we reach a leaf node
            else if (num_sources == 0) {
                // cout << "no more source!" << endl;
                // cout << "BEFORE EXITING, the graph contains " << boost::num_vertices(*g) << " vertices" << endl;
                break;
            }
            // when we reach a LOAD node
            else if (num_sources == 1) {

                // cout << "only one source!" << endl;
                next = get_first_source(g, cur);
                // p = boost::get(pmap, next);
                // print_vertex_property(p);

                if (next == NULL) {
                    // cout << "no more next vertex\n";
                    break;
                }
                
                // check whether cur complete the circle
                cur_property = boost::get(pmap, cur);
                next_property = boost::get(pmap, next);
                start_property = boost::get(pmap, start);

                // if a circle is completed, remove all vertices in the circle
                if (next_property.source == start_property.source) {
                    
                    circle.push_back(next);

                    if (find_source_vertices(g, next).size()) {
                        next = get_first_source(g, next);

                        // cout << "before removal, the graph contains " << boost::num_edges(*g) << " edges" << endl;
                        boost::remove_in_edge_if(circle[0], pred, *g);
                        for (int i = 1; i < circle.size(); i++) {
                            p = boost::get(pmap, circle[i]);
                            // cout << i << ": ";
                            // print_vertex_property(p);
                            boost::remove_in_edge_if(circle[i], pred, *g);
                            boost::remove_vertex(circle[i], *g);
                        }
                        // cout << endl;
                        // cout << "after removal, the graph contains " << boost::num_edges(*g) << " edges" << endl;
                        
                        // recalculate the pmap after vertex removal
                        pmap = boost::get(boost::vertex_bundle, *g);

                        // reconnect the graph
                        if (next != NULL) 
                            add_edge(g, next, start);
                        else {
                            // cout << "Next is NULL, exit\n";
                            break;
                        }
                            
                        // cout << "THE SOURCES OF START: ";
                        // vector<vertex_descriptor_t> ss = find_source_vertices(g, start);
                        
                        // for (int i = 0; i < ss.size(); i++)
                        // {
                        //     p = boost::get(pmap, ss[i]);
                        //     print_vertex_property(p);
                        // }
                        
                        // cout << "AFTER reconnect\n";
                        // print_vertices(g);
                        // clear the circle
                        circle.clear();
                        // the new start of the circle is the next vertex
                        // cout << "DONE removing" << endl;
                        cur = root;
                        break;
                    }
                    else {
                        // cout << "before removal, the graph contains " << boost::num_edges(*g) << " edges" << endl;
                        boost::remove_in_edge_if(circle[0], pred, *g);
                        for (int i = 1; i < circle.size(); i++) {
                            p = boost::get(pmap, circle[i]);
                            // cout << i << ": ";
                            // print_vertex_property(p);
                            boost::remove_in_edge_if(circle[i], pred, *g);
                            boost::remove_vertex(circle[i], *g);
                        }
                        // cout << endl;
                        // cout << "after removal, the graph contains " << boost::num_edges(*g) << " edges" << endl;
                        // the new start of the circle is the next vertex
                        // cout << "DONE removing" << endl;
                        cur = root;
                        break;
                    }
                }
                else {
                    // cout << "havent reached the end of circle yet\n";
                    circle.push_back(next);
                }

                cur = next;
            }
        }
    } 
}

void remove_vertex_in_func(Graph *g, vertex_descriptor_t v, vertex_descriptor_t root) {
    boost::remove_vertex(v, *g);
    get_nonterm_source(g, root);
}