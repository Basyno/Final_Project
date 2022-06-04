#ifndef HASH_H_
#define HASH_H_
#include <systemc>
#include <cmath>
#include <iomanip>
using namespace sc_core;

#include <tlm>
#include <tlm_utils/simple_target_socket.h>

#include "filter_def.h"

struct HASH : public sc_module {
  tlm_utils::simple_target_socket<HASH> tsock;

  sc_fifo<unsigned char> hash_in;
  sc_fifo<int> hash_out;

  SC_HAS_PROCESS(HASH);

  HASH(sc_module_name n): 
    sc_module(n), 
    tsock("t_skt"), 
    base_offset(0) 
  {
    tsock.register_b_transport(this, &HASH::blocking_transport);
    SC_THREAD(do_hash);
  }

  ~HASH() {
	}
  int i,j,t,k,m;
  int val[HASH_NUM];
  long long int product_sum[HASH_NUM]={0};
	long long int hash_value[HASH_NUM]={0};
	int value_p = 31;
	int value_m = 1000000009;
  unsigned int base_offset;

  void do_hash(){
    { wait(CLOCK_PERIOD, SC_NS); }

     for (unsigned int i = 0; i < HASH_NUM; i++) {
        val[i] = 0;
        wait(CLOCK_PERIOD, SC_NS);
      }

    while (true) {
     
      for (unsigned int j = 0; j < HASH_NUM; j++) {
          val[j] = hash_in.read(); 
      }

      for (unsigned int t = 0; t < HASH_NUM/6; t++) {
          product_sum[t] = (val[6*t]-96)*1 + (val[6*t+1]-96)*value_p + (val[6*t+2]-96)*value_p*value_p + (val[6*t+3]-96)*value_p*value_p*value_p + (val[6*t+4]-96)*value_p*value_p*value_p*value_p + + (val[6*t+5]-96)*value_p*value_p*value_p*value_p*value_p;
      }

      for (unsigned int k = 0; k < HASH_NUM/6; k++) {
          hash_value[k] = product_sum[k] % value_m; 
      }

      // cout << (int)result << endl;
    for (unsigned int m = 0; m < HASH_NUM/6; m++) {
      hash_out.write(hash_value[m]);
    }
    }
  }

  void blocking_transport(tlm::tlm_generic_payload &payload, sc_core::sc_time &delay){
    wait(delay);
    // unsigned char *mask_ptr = payload.get_byte_enable_ptr();
    // auto len = payload.get_data_length();
    tlm::tlm_command cmd = payload.get_command();
    sc_dt::uint64 addr = payload.get_address();
    unsigned char *data_ptr = payload.get_data_ptr();

    addr -= base_offset;


    // cout << (int)data_ptr[0] << endl;
    // cout << (int)data_ptr[1] << endl;
    // cout << (int)data_ptr[2] << endl;
    word buffer;

    switch (cmd) {
      case tlm::TLM_READ_COMMAND:
        // cout << "READ" << endl;
        switch (addr) {
          case HASH_RESULT_ADDR:
            buffer.sint = hash_out.read();
            break;
          default:
            std::cerr << "READ Error! HASH::blocking_transport: address 0x"
                      << std::setfill('0') << std::setw(8) << std::hex << addr
                      << std::dec << " is not valid" << std::endl;
          }
        data_ptr[0] = buffer.uc[0];
        data_ptr[1] = buffer.uc[1];
        data_ptr[2] = buffer.uc[2];
        data_ptr[3] = buffer.uc[3];
        break;
      case tlm::TLM_WRITE_COMMAND:
        // cout << "WRITE" << endl;
        switch (addr) {
          case HASH_R_ADDR:
            hash_in.write(data_ptr[0]);
            break;
          default:
            std::cerr << "WRITE Error! HASH::blocking_transport: address 0x"
                      << std::setfill('0') << std::setw(8) << std::hex << addr
                      << std::dec << " is not valid" << std::endl;
        }
        break;
      case tlm::TLM_IGNORE_COMMAND:
        payload.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
        return;
      default:
        payload.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
        return;
      }
      payload.set_response_status(tlm::TLM_OK_RESPONSE); // Always OK
  }
};
#endif
