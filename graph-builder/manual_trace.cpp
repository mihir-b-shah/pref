#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>

#define NUM_INSTR_DESTINATIONS 2
#define NUM_INSTR_SOURCES 4

#define RDI 24
#define RAX 1
#define XMM1 33
#define RSP 21
#define RBP 22

using namespace std;

typedef struct trace_instr_format {
    long long int offset1 = -1;
    long long int offset2 = -1;
    
    unsigned long long int ip;  // instruction pointer (program counter) value
    unsigned long long int op; // op code 

    unsigned char is_branch;    // is this branch
    unsigned char branch_taken; // if so, is this taken

    unsigned char destination_registers[NUM_INSTR_DESTINATIONS]; // output registers
    unsigned char source_registers[NUM_INSTR_SOURCES];           // input registers

    unsigned long long int destination_memory[NUM_INSTR_DESTINATIONS]; // output memory
    unsigned long long int source_memory[NUM_INSTR_SOURCES];           // input memory

} trace_instr_format_t;

trace_instr_format_t create_instr(unsigned long long int ip, long long int offset, unsigned char dst_reg, 
                    unsigned char src_reg, unsigned long long int dst_mem, unsigned long long int src_mem) {
    
    trace_instr_format_t instr;
    instr.ip = ip;
    instr.offset1 = offset;
    for (int i = 0; i < NUM_INSTR_DESTINATIONS; i++) {
        instr.destination_memory[i] = 0;
        instr.destination_registers[i] = 0;
    }
    instr.destination_registers[0] = dst_reg;
    instr.destination_memory[0] = dst_mem;
    for (int i = 0; i < NUM_INSTR_SOURCES; i++) {
        instr.source_memory[i] = 0;
        instr.source_registers[i] = 0;
    }
    instr.source_memory[0] = src_mem;
    instr.source_registers[0] = src_reg;

    return instr;
}

// Manually generate a trace file
int main() {
    FILE *out = fopen("manually_created_trace", "w+");

    vector<trace_instr_format_t> instrs;

    instrs.push_back(create_instr(0x4013fe, 0x28, XMM1, RAX, 0, 0));
    instrs.push_back(create_instr(0x4013fa, -1, RAX, 0, 0, 0x12345));
    instrs.push_back(create_instr(0x4013f6, -1, 0, RDI, 0x12345, 0));
    instrs.push_back(create_instr(0x4013f3, -1, RBP, RSP, 0, 0));
    instrs.push_back(create_instr(0x4013f2, -1, RBP, 0, 0, 0));
    instrs.push_back(create_instr(0x40144e, -1, RDI, RAX, 0, 0));
    instrs.push_back(create_instr(0x40144a, -1, RAX, 0, 0, 0x12345));
    instrs.push_back(create_instr(0x40145e, -1, 0, RAX, 0x12345, 0));
    instrs.push_back(create_instr(0x40145a, 0x38, RAX, RAX, 0, 0));
    instrs.push_back(create_instr(0x401456, -1, RAX, 0, 0, 0x12345));

    for (int i = instrs.size() - 1; i >= 0; i--) {
        fwrite(&instrs[i], sizeof(trace_instr_format_t), 1, out);
    }

    fclose(out);
    
}