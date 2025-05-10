#include <lcom/lcf.h>
#include <lcom/timer.h>

#include <stdint.h>

#include "i8254.h"

int counter_t = 0;
int hook_id = 0;

int (timer_set_frequency)(uint8_t timer, uint32_t freq) {
  if(freq < TIMER_FREQ/USHRT_MAX || freq > TIMER_FREQ  || timer < 0 || timer > 2) {
    return 1;
  }

  uint8_t st;

  if(timer_get_conf(timer, &st) == 1) {
    return 1;
  }

  uint8_t timer_cw = TIMER_SEL0;

  if (timer == 1) {
    timer_cw = TIMER_SEL1;
  }

  else if(timer == 2) {
    timer_cw = TIMER_SEL2;
  }

  st = st << 4;

  st = st >> 4;
  
  uint8_t control_word  = timer_cw | TIMER_LSB_MSB | st;

  uint16_t curr_freq = TIMER_FREQ/freq;

  if(sys_outb(TIMER_CTRL, control_word) != 0) {
    return 1;
  }

  uint8_t lsb = 0x00;

  util_get_LSB(curr_freq, &lsb);

  uint8_t msb = 0x00;

  util_get_MSB(curr_freq, &msb);

  uint8_t timer_register = TIMER_0;

  switch (timer)
  {
  case 1:
  timer_register = TIMER_1;
    break;
  case 2:
  timer_register = TIMER_2;
    break;
  default:
    break;
  }

  if(sys_outb(timer_register, (uint32_t )lsb) != 0) {
    return 1;
  }

  if(sys_outb(timer_register, (uint32_t )msb) != 0) {
    return 1;
  }

  return 0;
}

int (timer_subscribe_int)(uint8_t *bit_no) {

  *bit_no = BIT(hook_id); 
  
  if(bit_no == NULL) {
    return 1;
  }

  if(sys_irqsetpolicy(TIMER0_IRQ, IRQ_REENABLE, &hook_id) == 1) {
    return 1;
  }
  
  return 0;
}

int (timer_unsubscribe_int)() {

  if(sys_irqrmpolicy(&hook_id) == 1) {
    return 1;
  }
  
  return 0;
}

void (timer_int_handler)() {
  counter_t++;
}

int (timer_get_conf)(uint8_t timer, uint8_t *st) {
  if(st == NULL || timer < 0 || timer > 2) {
    return 1;
  }

  uint8_t read_back_command = (TIMER_RB_CMD | TIMER_RB_COUNT_ | TIMER_RB_SEL(timer));

  
  if(sys_outb(TIMER_CTRL, read_back_command) != 0) {
    return 1;
  }

  if(util_sys_inb(TIMER_0 + timer, st) == 1){
    return 1;
  }


  return 0;

}

int (timer_display_conf)(uint8_t timer, uint8_t st,
                        enum timer_status_field field) {
  union timer_status_field_val val;
  

  switch(field) {
    case tsf_all:
    val.byte = st;
    break;
    case tsf_base:
    val.bcd = (st & 0x01);
    break;
    case tsf_initial:
    val.in_mode = (st & 0x30) >> 4;
    break;
    case tsf_mode:
    val.count_mode = (st & 0x0D) >> 1;

    break;
    default:
    break;
  }

  if(timer_print_config(timer, field, val) == 1) {
    return 1;
  }

  return 0;

}
