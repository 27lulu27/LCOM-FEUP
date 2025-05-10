#include <lcom/lcf.h>

#include <stdint.h>

/*getting the least significant byte of a value
(val -> 16 bit value)
(lsb -> 8 bit number to store the least significant bit)*/
int(util_get_LSB)(uint16_t val, uint8_t *lsb) {
  uint16_t temp = 0;

  //getting the least significant byte
  temp = (val & 0x00FF);

  //convert from uint16_t to uint8_t
  *lsb = (uint8_t) temp;

  //return 0 if success
  return 0;
}

/*getting the most significant byte of a value
(val -> 16 bit value)
(lsb -> 8 bit number to store the least significant bit)*/
int(util_get_MSB)(uint16_t val, uint8_t *msb) {
  uint16_t temp = 0;

  //getting the most significant byte
  temp = (val & 0xFF00);

  //shift byte to the right (8 bits)
  temp = temp >> 8;

  //convert from uint16_t to uint8_t
  *msb = (uint8_t) temp;

  //return 0 if success
  return 0;
}

int (util_sys_inb)(int port, uint8_t *value) {

  uint32_t variable = 0;
  
  if(sys_inb(port, &variable) == 1) {
    return 1;
  }


  *value = (uint8_t) variable;


  return 0;
}
