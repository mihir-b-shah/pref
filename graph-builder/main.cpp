#include <iostream>
#include <fstream>
#include <assert.h>
#include <deque>
#include "graph.h"
#include "instruction.h"

// store the trace name here
char trace_string[1024];
char gunzip_command[1024];

uint64_t champsim_seed;

// the trace file
FILE *trace_file;

// the cache miss profile, contains the ip of instructions
// where cache miss happened
FILE *profile;

// ./main -traces ../ChampSim/dpc3_traces/600.perlbench_s-210B.champsimtrace.xz examp.txt
// g++ -g -std=c++11 -o main main.cpp

using namespace std;

// Read in an instruction in input_instr structure and 
// transform it to ooo_model_instr type
ooo_model_instr copy_into_format (input_instr current_instr) {
    
    // copy the instruction into the performance model's instruction format
    ooo_model_instr arch_instr;
    int num_reg_ops = 0, num_mem_ops = 0;

    // arch_instr.instr_id = instr_unique_id;
    arch_instr.ip = current_instr.ip;
    arch_instr.op = current_instr.op;
    arch_instr.is_branch = current_instr.is_branch;
    arch_instr.branch_taken = current_instr.branch_taken;

    arch_instr.offset1 = current_instr.offset1;
    arch_instr.offset2 = current_instr.offset2;

    // arch_instr.asid[0] = cpu;
    // arch_instr.asid[1] = cpu;

    bool reads_sp = false;
    bool writes_sp = false;
    bool reads_flags = false;
    bool reads_ip = false;
    bool writes_ip = false;
    bool reads_other = false;

    for (uint32_t i=0; i<NUM_INSTR_DESTINATIONS; i++) {
        arch_instr.destination_registers[i] = current_instr.destination_registers[i];
        arch_instr.destination_memory[i] = current_instr.destination_memory[i];
        arch_instr.destination_virtual_address[i] = current_instr.destination_memory[i];

        switch(arch_instr.destination_registers[i]) {
            case 0:
                break;
            case REG_STACK_POINTER:
                writes_sp = true;
                break;
            case REG_INSTRUCTION_POINTER:
                writes_ip = true;
                break;
            default:
                break;
        }

        /*
        if((arch_instr.is_branch) && (arch_instr.destination_registers[i] > 24) && (arch_instr.destination_registers[i] < 28))
            {
        arch_instr.destination_registers[i] = 0;
            }
        */
        
        if (arch_instr.destination_registers[i])
            num_reg_ops++;
        // if (arch_instr.destination_memory[i]) {
        //     num_mem_ops++;

        // }
    }
    for (int i=0; i<NUM_INSTR_SOURCES; i++) {
        arch_instr.source_registers[i] = current_instr.source_registers[i];
        arch_instr.source_memory[i] = current_instr.source_memory[i];
        arch_instr.source_virtual_address[i] = current_instr.source_memory[i];

        switch(arch_instr.source_registers[i]) {
            case 0:
                break;
            case REG_STACK_POINTER:
                reads_sp = true;
                break;
            case REG_FLAGS:
                reads_flags = true;
                break;
            case REG_INSTRUCTION_POINTER:
                reads_ip = true;
                break;
            default:
                reads_other = true;
                break;
        }
        
        /*
        if((!arch_instr.is_branch) && (arch_instr.source_registers[i] > 25) && (arch_instr.source_registers[i] < 28))
            {
        arch_instr.source_registers[i] = 0;
            }
        */
        
        if (arch_instr.source_registers[i])
            num_reg_ops++;
        if (arch_instr.source_memory[i])
            num_mem_ops++;
    }
    
    arch_instr.num_reg_ops = num_reg_ops;
    arch_instr.num_mem_ops = num_mem_ops;
    if (num_mem_ops > 0) 
        arch_instr.is_memory = 1;

    // determine what kind of branch this is, if any
    if(!reads_sp && !reads_flags && writes_ip && !reads_other)
    {
        // direct jump
        arch_instr.is_branch = 1;
        arch_instr.branch_taken = 1;
        arch_instr.branch_type = BRANCH_DIRECT_JUMP;
    }
    else if(!reads_sp && !reads_flags && writes_ip && reads_other)
    {
        // indirect branch
        arch_instr.is_branch = 1;
        arch_instr.branch_taken = 1;
        arch_instr.branch_type = BRANCH_INDIRECT;
    }
    else if(!reads_sp && reads_ip && !writes_sp && writes_ip && reads_flags && !reads_other)
    {
        // conditional branch
        arch_instr.is_branch = 1;
        arch_instr.branch_taken = arch_instr.branch_taken; // don't change this
        arch_instr.branch_type = BRANCH_CONDITIONAL;
    }
    else if(reads_sp && reads_ip && writes_sp && writes_ip && !reads_flags && !reads_other)
    {
        // direct call
        arch_instr.is_branch = 1;
        arch_instr.branch_taken = 1;
        arch_instr.branch_type = BRANCH_DIRECT_CALL;
    }
    else if(reads_sp && reads_ip && writes_sp && writes_ip && !reads_flags && reads_other)
    {
        // indirect call
        arch_instr.is_branch = 1;
        arch_instr.branch_taken = 1;
        arch_instr.branch_type = BRANCH_INDIRECT_CALL;
    }
    else if(reads_sp && !reads_ip && writes_sp && writes_ip)
    {
        // return
        arch_instr.is_branch = 1;
        arch_instr.branch_taken = 1;
        arch_instr.branch_type = BRANCH_RETURN;
    }
    else if(writes_ip)
    {
        // some other branch type that doesn't fit the above categories
        arch_instr.is_branch = 1;
        arch_instr.branch_taken = arch_instr.branch_taken; // don't change this
        arch_instr.branch_type = BRANCH_OTHER;
    }

    // total_branch_types[arch_instr.branch_type]++;
    
    // if((arch_instr.is_branch == 1) && (arch_instr.branch_taken == 1))
    // {
    //     arch_instr.branch_target = next_instr.ip;
    // }

    return arch_instr;

}

// Search the last occurence of the miss pc in the trace_file
deque<ooo_model_instr> search_last_occurence(uint64_t miss_pc) {

    // to read in from the trace file
    input_instr current_instr_read;
    input_instr last_occur;
    // structure better for program use
    ooo_model_instr current_instr;
    ooo_model_instr res;
    size_t instr_size = sizeof(input_instr);
    // signaling whether ip is found
    bool found = 0;

    deque<ooo_model_instr> window;
    int max_window_size = 1000;

    while (fread(&current_instr_read, instr_size, 1, trace_file))
    {
        current_instr = copy_into_format(current_instr_read);
        if (window.size() < max_window_size) {
            window.push_front(current_instr);
        }
        else {
            window.pop_back();
            window.push_front(current_instr);
        }
        // cout << "the target: " << hex << miss_pc << dec << endl;
        // cout << "the current ip is: " << hex << current_instr_read.ip << dec << endl;
        if (miss_pc == current_instr_read.ip) {
            found = 1;
            break;
        }

    }

    pclose(trace_file);
        
    // exit
    if (!found) {
        cout << "NOT FOUND\n";
    }

    return window;
        
}

// Search the last occurence of register reg
// if not found, return -1
int traceback_reg(uint8_t reg, deque<ooo_model_instr> trace_window, int index) {
    
    int size = trace_window.size();
    ooo_model_instr cur_instr;
    bool found = -1;

    for (int i = index; i < size; i++)
    {
        cur_instr = trace_window[i];
        if(cur_instr.destination_registers[0] == reg) {
            found = 1;
            return i;
        }
    }

    // cout << "previous reg not found, returning -1\n";
    return -1;
    
}

// Search the last occurence of the effective address
// if not found, return -1
int traceback_ea(uint64_t ea, deque<ooo_model_instr> trace_window, int index) {
    
    int size = trace_window.size();
    ooo_model_instr cur_instr;
    bool found = -1;

    for (int i = index; i < size; i++)
    {
        cur_instr = trace_window.at(i);
        if(cur_instr.destination_memory[0] == ea) {
            found = 1;
            return i;
        }
    }

    // cout << "previous ea not found, returning -1\n";
    return -1;
    
}

// Check if the intruction has constant offset
// If it does, return the offset
// Otherwise, return -1
long long int find_const_offset(ooo_model_instr cur_instr) {
    if (cur_instr.offset1 != -1 && cur_instr.offset1 != 0) 
        return cur_instr.offset1;
    else if (cur_instr.offset2 != -1 && cur_instr.offset2 != 0) 
        return cur_instr.offset2;
    else
        return -1;
}

// Transform an instruction to a vertex in the graph
int instr_to_vertex(vertex_descriptor_t parent, ooo_model_instr instr, deque<ooo_model_instr> trace_window, Graph *g) {
    
    vertex_descriptor_t son;
    vertex_descriptor_t offset;

    int next_index;

    // When there is no offset
    if (instr.offset1 == -1) {
        if (instr.is_memory) {
            son = add_vertex(g, instr.source_memory[0], ADDR);
            next_index = traceback_ea(instr.source_memory[0], trace_window, 0);
        }
        else {
            son = add_vertex(g, instr.source_registers[0], ADDR);
            next_index = traceback_ea(instr.source_registers[0], trace_window, 0);
        }
        add_edge(g, son, parent);
    }
    else {
        offset = add_vertex(g, instr.offset1, CONST);
        if (instr.is_memory) {
            son = add_vertex(g, instr.source_memory[0], ADDR);
            next_index = traceback_ea(instr.source_memory[0], trace_window, 0);
        }
        else {
            son = add_vertex(g, instr.source_registers[0], ADDR);
            next_index = traceback_ea(instr.source_registers[0], trace_window, 0);
        }
        add_edge(g, offset, parent);
        add_edge(g, son, parent);
    }

    return next_index;
}

// Builds the graph for the instructions in the trace window
vertex_descriptor_t build_graph(deque<ooo_model_instr> trace_window, Graph *g, uint64_t miss_pc) {

    // the possible constant source of the root
    vertex_descriptor_t offset;
    // the root of our graph
    vertex_descriptor_t root;
    // the index for the current instruction
    int cur_index = 0;
    int next_index;
    // the vertex correponding to the current instruction
    vertex_descriptor_t cur_vertex;
    // the parent for the current instruction
    vertex_descriptor_t cur_parent;

    root = add_vertex(g, trace_window[0].destination_registers[0], ADDR);

    // cout << "current index before entering is loop is: " << cur_index << endl;
    cur_parent = root;

    // Find the first dependency of the miss-causing pc
    // Scan up the trace window until we see the miss pc again
    while(1) {
        // If we see the miss pc again, the graph is completed
        
        if (trace_window[cur_index].offset1 == -1) {
            // cout << "NO offset at instr " << cur_index << endl;
            if (trace_window[cur_index].is_memory) {
                // cout << "instr " << cur_index << " uses memory " << trace_window[cur_index].source_memory[0] << endl;
                cur_vertex = add_vertex(g, trace_window[cur_index].source_memory[0], ADDR);
                next_index = traceback_ea(trace_window[cur_index].source_memory[0], trace_window, cur_index + 1);
            }
            else {
                // cout << "instr " << cur_index << " uses reg " << trace_window[cur_index].source_registers[0] << endl;
                cur_vertex = add_vertex(g, trace_window[cur_index].source_registers[0], ADDR);
                next_index = traceback_reg(trace_window[cur_index].source_registers[0], trace_window, cur_index + 1);
            }
            add_edge(g, cur_vertex, cur_parent);
        }
        else {
            // cout << "YES offset at instr " << cur_index << endl;
            offset = add_vertex(g, trace_window[cur_index].offset1, CONST);
            if (trace_window[cur_index].is_memory) {
                // cout << "instr " << cur_index << " uses memory " << trace_window[cur_index].source_memory[0] << endl;
                cur_vertex = add_vertex(g, trace_window[cur_index].source_memory[0], ADDR);
                next_index = traceback_ea(trace_window[cur_index].source_memory[0], trace_window, cur_index + 1);
            }
            else {
                // cout << "instr " << cur_index << " uses reg " << trace_window[cur_index].source_registers[0] << endl;
                cur_vertex = add_vertex(g, trace_window[cur_index].source_registers[0], ADDR);
                next_index = traceback_reg(trace_window[cur_index].source_registers[0], trace_window, cur_index + 1);
            }
            add_edge(g, offset, cur_parent);
            add_edge(g, cur_vertex, cur_parent);
        }
        // if we cannot trace back anymore, break
        if (next_index == -1)
            break;
        cur_parent = cur_vertex;
        cur_index = next_index;
            
        if (trace_window[cur_index].ip == miss_pc) {
            // cout << "reached miss pc again, exit\n";
            break;
        }
    }
    
    return root;
}

int main(int argc, char** argv)
{

    char miss_instr[255];
    deque<ooo_model_instr> last_occur_window;

    sprintf(trace_string, "%s", argv[2]);

    std::string full_name(argv[2]);
    std::string last_dot = full_name.substr(full_name.find_last_of("."));

    std::string fmtstr;
    std::string decomp_program;

    // Assert the type of trace the program accepts
    if (full_name.substr(0,4) == "http")
    {
        // Check file exists
        char testfile_command[4096];
        sprintf(testfile_command, "wget -q --spider %s", argv[2]);
        FILE *testfile = popen(testfile_command, "r");
        if (pclose(testfile))
        {
            std::cerr << "TRACE FILE NOT FOUND" << std::endl;
            assert(0);
        }
        fmtstr = "wget -qO- %2$s | %1$s -dc";
    }
    else
    {
        std::ifstream testfile(argv[2]);
        if (!testfile.good())
        {
            std::cerr << "TRACE FILE NOT FOUND" << std::endl;
            assert(0);
        }
        fmtstr = "%1$s -dc %2$s";
    }

    if (last_dot[1] == 'g') // gzip format
        decomp_program = "gzip";
    else if (last_dot[1] == 'x') // xz
        decomp_program = "xz";
    else {
        std::cout << "ChampSim does not support traces other than gz or xz compression!" << std::endl;
        assert(0);
    }

    // Build the command for opening the trace file
    sprintf(gunzip_command, fmtstr.c_str(), decomp_program.c_str(), argv[2]);

    // Open the trace file
    trace_file = popen(gunzip_command, "r");
    if (trace_file == NULL) {
        printf("\n*** Trace file not found: %s ***\n\n", argv[2]);
        assert(0);
    }

    // ------------------------------------------------------------------------ //
    /////////////////////// trace reading complete ///////////////////////////////
    // ------------------------------------------------------------------------ //

    // Open the profile file
    profile = fopen(argv[3], "r");
    uint64_t miss_pc;

    // Build a trace window
    while (fgets(miss_instr, sizeof(miss_instr), profile)) {
        printf("%s", miss_instr); 
        miss_pc = (int)strtol(miss_instr, NULL, 0);
        last_occur_window = search_last_occurence(miss_pc);
    }

    fclose(profile);
    
    cout << "creating the graph" << endl;
    Graph g = graph_create();
    cout << "building the graph" << endl;
    vertex_descriptor_t root = build_graph(last_occur_window, &g, miss_pc);
    print_vertices(&g);
    boost::property_map<Graph, boost::vertex_bundle_t>::type pmap = boost::get(boost::vertex_bundle, g);
    VertexProperty root_p = boost::get(pmap, root);
    print_vertex_property(root_p);

    cout << "do compaction to the graph" << endl;
    store_load_bypassing(&g, root);
    print_vertices(&g);
    
    return 0;
}
