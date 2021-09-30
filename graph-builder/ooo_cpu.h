#ifndef OOO_CPU_H
#define OOO_CPU_H

#include "cache.h"
#include <fstream>>

#ifdef CRC2_COMPILE
#define STAT_PRINTING_PERIOD 1000000
#else
#define STAT_PRINTING_PERIOD 10000000
#endif
#define DEADLOCK_CYCLE 1000000
#define NUM_CPU 1

using namespace std;

// CORE PROCESSOR
#define FETCH_WIDTH 6
#define DECODE_WIDTH 6
#define EXEC_WIDTH 6
#define LQ_WIDTH 2
#define SQ_WIDTH 2
#define RETIRE_WIDTH 4
#define SCHEDULER_SIZE 128
#define BRANCH_MISPREDICT_PENALTY 1
//#define SCHEDULING_LATENCY 0
//#define EXEC_LATENCY 0
//#define DECODE_LATENCY 2

#define STA_SIZE (ROB_SIZE*NUM_INSTR_DESTINATIONS_SPARC)

extern uint32_t SCHEDULING_LATENCY, EXEC_LATENCY, DECODE_LATENCY;

// cpu
class O3_CPU {
  public:
    uint32_t cpu;

    // trace
    FILE *trace_file;
    char trace_string[1024];
    char gunzip_command[1024];

    // instruction
    uint64_t instr_unique_id, completed_executions, 
             begin_sim_cycle, begin_sim_instr, 
             last_sim_cycle, last_sim_instr,
             finish_sim_cycle, finish_sim_instr,
             warmup_instructions, simulation_instructions, instrs_to_read_this_cycle, instrs_to_fetch_this_cycle,
             next_print_instruction, num_retired;
    uint32_t inflight_reg_executions, inflight_mem_executions, num_searched;
    uint32_t next_ITLB_fetch;

    // branch
    int branch_mispredict_stall_fetch; // flag that says that we should stall because a branch prediction was wrong
    int mispredicted_branch_iw_index; // index in the instruction window of the mispredicted branch.  fetch resumes after the instruction at this index executes
    uint8_t  fetch_stall;
    uint64_t fetch_resume_cycle;
    uint64_t num_branch, branch_mispredictions;
    uint64_t total_rob_occupancy_at_branch_mispredict;
  uint64_t total_branch_types[8];

  // trace cache for previously decoded instructions
  
    // constructor
    O3_CPU() {
        cpu = 0;

        // trace
        trace_file = NULL;

        // instruction
        instr_unique_id = 0;
        completed_executions = 0;
        begin_sim_cycle = 0;
        begin_sim_instr = 0;
        last_sim_cycle = 0;
        last_sim_instr = 0;
        finish_sim_cycle = 0;
        finish_sim_instr = 0;
        warmup_instructions = 0;
        simulation_instructions = 0;
        instrs_to_read_this_cycle = 0;
        instrs_to_fetch_this_cycle = 0;

        next_print_instruction = STAT_PRINTING_PERIOD;
        num_retired = 0;

        inflight_reg_executions = 0;
        inflight_mem_executions = 0;
        num_searched = 0;

        next_ITLB_fetch = 0;

        // branch
        branch_mispredict_stall_fetch = 0;
        mispredicted_branch_iw_index = 0;
        fetch_stall = 0;
	fetch_resume_cycle = 0;
        num_branch = 0;
        branch_mispredictions = 0;
	for(uint32_t i=0; i<8; i++)
	  {
	    total_branch_types[i] = 0;
	  }
    }

    // functions
    void read_from_trace(),
         fetch_instruction(),
         decode_and_dispatch(),
         schedule_instruction(),
         execute_instruction(),
         schedule_memory_instruction(),
         execute_memory_instruction(),
         do_scheduling(uint32_t rob_index),  
         reg_dependency(uint32_t rob_index),
         do_execution(uint32_t rob_index),
         do_memory_scheduling(uint32_t rob_index),
         operate_lsq(),
         complete_execution(uint32_t rob_index),
         reg_RAW_dependency(uint32_t prior, uint32_t current, uint32_t source_index),
         reg_RAW_release(uint32_t rob_index),
         mem_RAW_dependency(uint32_t prior, uint32_t current, uint32_t data_index, uint32_t lq_index);

    void initialize_core();
    void add_load_queue(uint32_t rob_index, uint32_t data_index),
         add_store_queue(uint32_t rob_index, uint32_t data_index),
         execute_store(uint32_t rob_index, uint32_t sq_index, uint32_t data_index);
    int  execute_load(uint32_t rob_index, uint32_t sq_index, uint32_t data_index);
    void check_dependency(int prior, int current);
    void operate_cache();
    void update_rob();
    void retire_rob();

    uint32_t check_and_add_lsq(uint32_t rob_index);

    // branch predictor
    uint8_t predict_branch(uint64_t ip);
    void    initialize_branch_predictor(),
            last_branch_result(uint64_t ip, uint8_t taken);

  // code prefetching
  void l1i_prefetcher_initialize();
  void l1i_prefetcher_branch_operate(uint64_t ip, uint8_t branch_type, uint64_t branch_target);
  void l1i_prefetcher_cache_operate(uint64_t v_addr, uint8_t cache_hit, uint8_t prefetch_hit);
  void l1i_prefetcher_cycle_operate();
  void l1i_prefetcher_cache_fill(uint64_t v_addr, uint32_t set, uint32_t way, uint8_t prefetch, uint64_t evicted_v_addr);
  void l1i_prefetcher_final_stats();
  int prefetch_code_line(uint64_t pf_v_addr); 
};

extern O3_CPU ooo_cpu[1];

#endif
