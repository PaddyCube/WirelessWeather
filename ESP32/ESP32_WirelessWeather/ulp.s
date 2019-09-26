/* ULP Example: multi pulse counting, counting two GPIOs while ESP sleeps
 *  
   This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.

   ULP wakes up to run this code at a certain period, determined by the values
   in SENS_ULP_CP_SLEEP_CYCx_REG registers. On each wake up, the program checks
   the input on GPIO_A and GPIO_B. If the values are different from the previous one, the
   program "debounces" the input: on the next debounce_max_count wake ups,
   it expects to see the same value of input.
   If this condition holds true, the program increments edge_count for each GPIO and starts
   waiting for input signal polarity to change again.

   While ULP counts, ESP main core sleeps for a given time (10s). After each timer wake-up, 
   ESP core reads ULP counting values for both pins and print the value to console

   (commented out:
   When the edge counter reaches certain value (set by the main program),
   this program running triggers a wake up from deep sleep. )

   This example is based on the ULP count example provided by Espressif. All credits to them
   https://github.com/espressif/esp-idf/blob/master/examples/system/ulp

   I used the ulptool provided by duff2013 https://github.com/duff2013/ulptool so this example
   was created using Arduino IDE instead of ESP-IDF. Thanks to duff2013 to share this great tool
   with the community
*/

/* ULP assembly files are passed through C preprocessor first, so include directives
   and C macros may be used in these files 
 */
#include "soc/rtc_cntl_reg.h"
#include "soc/rtc_io_reg.h"
#include "soc/soc_ulp.h"

  /* Define variables, which go into .bss section (zero-initialized data) */
  .bss
  /* Next input signal edge expected: 0 (negative) or 1 (positive) */
  .global next_edge_wind
next_edge_wind:
  .long 0

  .global next_edge_rain
next_edge_rain:
  .long 0
  /* Counter started when signal value changes.
     Edge is "debounced" when the counter reaches zero. */
  .global debounce_counter_wind
debounce_counter_wind:
  .long 0
  
  .global debounce_counter_rain
debounce_counter_rain:
  .long 0

  /* Value to which debounce_counter gets reset.
     Set by the main program. */
  .global debounce_max_count
debounce_max_count:
  .long 0

  /* Total number of signal edges acquired */
  .global edge_count_wind
edge_count_wind:
  .long 0

  .global edge_count_rain
edge_count_rain:
  .long 0
  
  /* Number of edges to acquire before waking up the SoC.
     Set by the main program. */
  .global edge_count_to_wake_up
edge_count_to_wake_up:
  .long 0

  /* RTC IO number used to sample the input signal.
     Set by main program. */
  .global io_number_wind
io_number_wind:
  .long 0

  .global io_number_rain
io_number_rain:
  .long 0
  /* Code goes into .text section */
  .text
  .global entry
entry:

/* START OF CHECK FOR RTC_GPIO A */
read_io_wind:
  /* Load io_number */
  move r3, io_number_wind
  ld r3, r3, 0

  /* Lower 16 IOs and higher need to be handled separately,
   * because r0-r3 registers are 16 bit wide.
   * Check which IO this is.
   */
  move r0, r3
  jumpr read_io_high_wind, 16, ge

  /* Read the value of lower 16 RTC IOs into R0 */
  READ_RTC_REG(RTC_GPIO_IN_REG, RTC_GPIO_IN_NEXT_S, 16)
  rsh r0, r0, r3
  jump read_done_wind

  /* Read the value of RTC IOs 16-17, into R0 */
read_io_high_wind:
  READ_RTC_REG(RTC_GPIO_IN_REG, RTC_GPIO_IN_NEXT_S + 16, 2)
  sub r3, r3, 16
  rsh r0, r0, r3

read_done_wind:
  and r0, r0, 1
  /* State of input changed? */
  move r3, next_edge_wind
  ld r3, r3, 0
  add r3, r0, r3
  and r3, r3, 1
  jump changed_wind, eq
  /* Not changed */
  /* Reset debounce_counter to debounce_max_count */
  move r3, debounce_max_count
  move r2, debounce_counter_wind
  ld r3, r3, 0
  st r3, r2, 0
  /* Start check for RTC_GPIO B */
  jump read_io_rain

  .global changed_wind
changed_wind:
  /* Input state changed */
  /* Has debounce_counter reached zero? */
  move r3, debounce_counter_wind
  ld r2, r3, 0
  add r2, r2, 0 /* dummy ADD to use "jump if ALU result is zero" */
  jump edge_detected_wind, eq
  /* Not yet. Decrement debounce_counter */
  sub r2, r2, 1
  st r2, r3, 0
  /* Start check for RTC_GPIO B */
  jump read_io_rain

  .global edge_detected_wind
edge_detected_wind:
  /* Reset debounce_counter to debounce_max_count */
  move r3, debounce_max_count
  move r2, debounce_counter_wind
  ld r3, r3, 0
  st r3, r2, 0
  /* Flip next_edge */
  move r3, next_edge_wind
  ld r2, r3, 0
  add r2, r2, 1
  and r2, r2, 1
  st r2, r3, 0
  /* Increment edge_count */
  move r3, edge_count_wind
  ld r2, r3, 0
  add r2, r2, 1
  st r2, r3, 0
  /* Compare edge_count to edge_count_to_wake_up */
  move r3, edge_count_to_wake_up
  ld r3, r3, 0
  sub r3, r3, r2
 /* jump wake_up, eq */
  /* Start check for RTC_GPIO B */
  jump read_io_rain


/* START OF CHECK FOR RTC_GPIO B */
read_io_rain:
  /* Load io_number */
  move r3, io_number_rain
  ld r3, r3, 0

  /* Lower 16 IOs and higher need to be handled separately,
   * because r0-r3 registers are 16 bit wide.
   * Check which IO this is.
   */
  move r0, r3
  jumpr read_io_high_rain, 16, ge

  /* Read the value of lower 16 RTC IOs into R0 */
  READ_RTC_REG(RTC_GPIO_IN_REG, RTC_GPIO_IN_NEXT_S, 16)
  rsh r0, r0, r3
  jump read_done_rain

  /* Read the value of RTC IOs 16-17, into R0 */
read_io_high_rain:
  READ_RTC_REG(RTC_GPIO_IN_REG, RTC_GPIO_IN_NEXT_S + 16, 2)
  sub r3, r3, 16
  rsh r0, r0, r3

read_done_rain:
  and r0, r0, 1
  /* State of input changed? */
  move r3, next_edge_rain
  ld r3, r3, 0
  add r3, r0, r3
  and r3, r3, 1
  jump changed_rain, eq
  /* Not changed */
  /* Reset debounce_counter to debounce_max_count */
  move r3, debounce_max_count
  move r2, debounce_counter_rain
  ld r3, r3, 0
  st r3, r2, 0
  /* End program */
  halt

  .global changed_rain
changed_rain:
  /* Input state changed */
  /* Has debounce_counter reached zero? */
  move r3, debounce_counter_rain
  ld r2, r3, 0
  add r2, r2, 0 /* dummy ADD to use "jump if ALU result is zero" */
  jump edge_detected_rain, eq
  /* Not yet. Decrement debounce_counter */
  sub r2, r2, 1
  st r2, r3, 0
  /* End program */
  halt

  .global edge_detected_rain
edge_detected_rain:
  /* Reset debounce_counter to debounce_max_count */
  move r3, debounce_max_count
  move r2, debounce_counter_rain
  ld r3, r3, 0
  st r3, r2, 0
  /* Flip next_edge */
  move r3, next_edge_rain
  ld r2, r3, 0
  add r2, r2, 1
  and r2, r2, 1
  st r2, r3, 0
  /* Increment edge_count */
  move r3, edge_count_rain
  ld r2, r3, 0
  add r2, r2, 1
  st r2, r3, 0
  /* Compare edge_count to edge_count_to_wake_up */
  move r3, edge_count_to_wake_up
  ld r3, r3, 0
  sub r3, r3, r2
 /* jump wake_up, eq */
  /* Not yet. End program */
  halt


  
  .global wake_up
wake_up:
  /* Check if the system can be woken up */
  READ_RTC_FIELD(RTC_CNTL_LOW_POWER_ST_REG, RTC_CNTL_RDY_FOR_WAKEUP)
  and r0, r0, 1
  jump wake_up, eq

  /* Wake up the SoC, end program */
  wake
  halt
