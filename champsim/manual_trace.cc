
#include <cstdio>
#include <vector>
#include <cstdint>

#define RDI 24
#define RAX 1
#define XMM1 33
#define RSP 21
#define RBP 22

struct trace_instr_format_t {
  uint64_t ip;

  uint8_t is_br;
  uint8_t br_tkn;

  uint8_t dst_regs[2];
  uint8_t src_regs[4];

  uint64_t dst_mem[2];
  uint64_t src_mem[4];

  trace_instr_format_t(uint64_t ip, uint8_t is_br, uint8_t tkn, uint8_t dst_reg, uint8_t src_reg, uint64_t dst_mem, uint64_t src_mem) : ip(ip), is_branch(is_br), br_tkn(tkn), dst_regs{dst_reg, 0}, src_regs{src_reg, 0, 0, 0}, dst_mem{dst_mem,0}, src_mem{src_mem, 0, 0, 0} {}
};

using namespace std;

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
