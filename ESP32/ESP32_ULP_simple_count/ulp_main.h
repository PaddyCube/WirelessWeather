/*
    Put your ULP globals here you want visibility
    for your sketch. Add "ulp_" to the beginning
    of the variable name and must be size 'uint32_t'
*/
#include "Arduino.h"

extern uint32_t ulp_entry;

extern uint32_t ulp_next_edge;
extern uint32_t ulp_debounce_counter;
extern uint32_t ulp_debounce_max_count;
extern uint32_t ulp_edge_count;
extern uint32_t ulp_edge_count_to_wake_up;
extern uint32_t ulp_io_number;
