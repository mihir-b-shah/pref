#include "ooo_cpu.h"
#include "set.h"
#include <iostream>

// out-of-order core
O3_CPU ooo_cpu[NUM_CPUS]; 
uint64_t current_core_cycle[NUM_CPUS], stall_cycle[NUM_CPUS];
uint32_t SCHEDULING_LATENCY = 0, EXEC_LATENCY = 0, DECODE_LATENCY = 0;

void O3_CPU::read_from_trace()
{
    // actual processors do not work like this but for easier implementation,
    // we read instruction traces and virtually add them in the ROB
    // note that these traces are not yet translated and fetched 

    uint8_t continue_reading = 1;
    uint32_t num_reads = 0;
    instrs_to_read_this_cycle = FETCH_WIDTH;

    // first, read PIN trace
    while (continue_reading) {

	    input_instr trace_read_instr;

        size_t instr_size = 1;
        
        if (!fread(&trace_read_instr, instr_size, 1, trace_file))
        {
            // reached end of file for this trace
            cout << "*** Reached end of trace for Core: " << cpu << " Repeating trace: " << trace_string << endl; 
    
            // close the trace file and re-open it
            pclose(trace_file);
            trace_file = popen(gunzip_command, "r");
            if (trace_file == NULL) {
                cerr << endl << "*** CANNOT REOPEN TRACE FILE: " << trace_string << " ***" << endl;
                assert(0);
            }
        }
	    else
	      { // successfully read the trace

		if(instr_unique_id == 0)
		  {
		    current_instr = next_instr = trace_read_instr;
		  }
		else
		  {
		    current_instr = next_instr;
		    next_instr = trace_read_instr;
		  }

                // copy the instruction into the performance model's instruction format
                ooo_model_instr arch_instr;
                int num_reg_ops = 0, num_mem_ops = 0;

                arch_instr.instr_id = instr_unique_id;
                arch_instr.ip = current_instr.ip;
                arch_instr.is_branch = current_instr.is_branch;
                arch_instr.branch_taken = current_instr.branch_taken;

                arch_instr.asid[0] = cpu;
                arch_instr.asid[1] = cpu;

		bool reads_sp = false;
		bool writes_sp = false;
		bool reads_flags = false;
		bool reads_ip = false;
		bool writes_ip = false;
		bool reads_other = false;

                for (uint32_t i=0; i<MAX_INSTR_DESTINATIONS; i++) {
                    arch_instr.destination_registers[i] = current_instr.destination_registers[i];
                    arch_instr.destination_memory[i] = current_instr.destination_memory[i];
                    arch_instr.destination_virtual_address[i] = current_instr.destination_memory[i];

		    switch(arch_instr.destination_registers[i])
		      {
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
                    if (arch_instr.destination_memory[i]) {
                        num_mem_ops++;

                        // update STA, this structure is required to execute store instructions properly without deadlock
                        if (num_mem_ops > 0) {			  
#ifdef SANITY_CHECK
                            if (STA[STA_tail] < UINT64_MAX) {
                                if (STA_head != STA_tail)
                                    assert(0);
                            }
#endif
                            STA[STA_tail] = instr_unique_id;
                            STA_tail++;

                            if (STA_tail == STA_SIZE)
                                STA_tail = 0;
                        }
                    }
                }

                for (int i=0; i<NUM_INSTR_SOURCES; i++) {
                    arch_instr.source_registers[i] = current_instr.source_registers[i];
                    arch_instr.source_memory[i] = current_instr.source_memory[i];
                    arch_instr.source_virtual_address[i] = current_instr.source_memory[i];

		    switch(arch_instr.source_registers[i])
                      {
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

		total_branch_types[arch_instr.branch_type]++;
		
		if((arch_instr.is_branch == 1) && (arch_instr.branch_taken == 1))
		  {
		    arch_instr.branch_target = next_instr.ip;
		  }

                // add this instruction to the IFETCH_BUFFER
                if (IFETCH_BUFFER.occupancy < IFETCH_BUFFER.SIZE) {
		  uint32_t ifetch_buffer_index = add_to_ifetch_buffer(&arch_instr);
		  num_reads++;

                    // handle branch prediction
                    if (IFETCH_BUFFER.entry[ifetch_buffer_index].is_branch) {

                        DP( if (warmup_complete[cpu]) {
                        cout << "[BRANCH] instr_id: " << instr_unique_id << " ip: " << hex << arch_instr.ip << dec << " taken: " << +arch_instr.branch_taken << endl; });

                        num_branch++;

			// handle branch prediction & branch predictor update
			uint8_t branch_prediction = predict_branch(IFETCH_BUFFER.entry[ifetch_buffer_index].ip);
			uint64_t predicted_branch_target = IFETCH_BUFFER.entry[ifetch_buffer_index].branch_target;
			if(branch_prediction == 0)
			  {
			    predicted_branch_target = 0;
			  }
			// call code prefetcher every time the branch predictor is used
			l1i_prefetcher_branch_operate(IFETCH_BUFFER.entry[ifetch_buffer_index].ip,
						      IFETCH_BUFFER.entry[ifetch_buffer_index].branch_type,
						      predicted_branch_target);
			
			if(IFETCH_BUFFER.entry[ifetch_buffer_index].branch_taken != branch_prediction)
			  {
			    branch_mispredictions++;
			    total_rob_occupancy_at_branch_mispredict += ROB.occupancy;
			    if(warmup_complete[cpu])
			      {
				fetch_stall = 1;
				instrs_to_read_this_cycle = 0;
				IFETCH_BUFFER.entry[ifetch_buffer_index].branch_mispredicted = 1;
			      }
			  }
			else
			  {
			    // correct prediction
			    if(branch_prediction == 1)
			      {
				// if correctly predicted taken, then we can't fetch anymore instructions this cycle
				instrs_to_read_this_cycle = 0;
			      }
			  }
			
			last_branch_result(IFETCH_BUFFER.entry[ifetch_buffer_index].ip, IFETCH_BUFFER.entry[ifetch_buffer_index].branch_taken);
                    }

                    if ((num_reads >= instrs_to_read_this_cycle) || (IFETCH_BUFFER.occupancy == IFETCH_BUFFER.SIZE))
                        continue_reading = 0;
                }
                instr_unique_id++;
            }
        
    }

    //instrs_to_fetch_this_cycle = num_reads;
}
