#include <lcom/lcf.h>

#include <stdint.h>

int cnt = 0;


int(util_get_LSB)(uint16_t val, uint8_t *lsb) {
  uint16_t temp = 0;

  temp = (val & 0x00FF);

  *lsb = (uint8_t) temp;

  return 0;
}

int(util_get_MSB)(uint16_t val, uint8_t *msb) {
  uint16_t temp = 0;

  temp = (val & 0xFF00);

  temp = temp >> 8;

  *msb = (uint8_t) temp;

  return 0;
}

int (util_sys_inb)(int port, uint8_t *value) {

  uint32_t variable = 0;
  
  if(sys_inb(port, &variable) == 1) {
    return 1;
  }
  #ifdef LAB3
  cnt++;
  #endif


  *value = (uint8_t) variable;


  return 0;
}
