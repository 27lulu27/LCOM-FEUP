// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>

#include <lcom/lab5.h>

#include <stdint.h>
#include <stdio.h>

// Any header files included below this line should have been created by you
#include "i8042.h"
#include "keyboards.h"
#include "video.h"

extern int scancode;
extern vbe_mode_info_t vmi_p;

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/lab5/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab5/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}


//test if video is being initialized 
int(video_test_init)(uint16_t mode, uint8_t delay) {

  //initialize video
  if (vg_initi(mode) != 0) {
    printf("video initi failed\n");    return 1;
  }

  // wait x seconds
  sleep(delay);

  //exit video
  if (vg_exit() != 0) {
    printf("exit failed\n");
    return 1;
  }

  //if success return 0
  return 0;
}

// test if a rectangle is drawn 
int(video_test_rectangle)(uint16_t mode, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color) {

  uint8_t bit_no = IRQ1;

  // set irq to the bit of bit_no
  uint8_t irq_set = BIT(bit_no);

  // subscribe to keyboard controller
  if (kbc_subscribe_int() != 0) {
    return 1;
  }

  //initialize video
  if (vg_initi(mode) != 0) {
    printf("video initi failed\n");
    return 1;
  }

  //draw rectangle
  if (vg_draw_rectangle(x, y, width, height, color) != 0) {
    return 1;
  }

  // int where ipc status will be placed
  int ipc_status;
  // message
  message msg;
  // just to print driver's error message
  double r = 0;

  write_to_video_mem();
  // while the ESC key isn't pressed
  while (scancode != ESC) {
    // read driver message from kernel
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
      printf("driver_receive failed with: %d", r);
      // in case of an error, this keeps trying to read (could use return 1 instead)
      continue;
    }
    // check if ipc_status is a notification
    if (is_ipc_notify(ipc_status)) {
      // check if message is from hardware
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:
          // if message and irq_set are the same
          if (msg.m_notify.interrupts & irq_set) {
            // reads the scancodes
            kbc_ih();
          }
          break;
        default:
          break;
      }
    }
  }

  // unsubscribe keyboard controller
  if (kbc_unsubscribe_int() != 0) {
    return 1;
  }

  //exit video
  if (vg_exit() != 0) {
    printf("exit failed\n");
    return 1;
  }

  //return 0 if successful
  return 0;
}

//test if a pattern is drawn 
int(video_test_pattern)(uint16_t mode, uint8_t no_rectangles, uint32_t first, uint8_t step) {

  uint8_t bit_no = IRQ1;

  // set irq to the bit of bit_no
  uint8_t irq_set = BIT(bit_no);

  int index = 0;
  int first_red = 0;
  int first_green = 0;
  int first_blue = 0;
  int width = 0;
  int heigth = 0;
  int color = 0;

  // subscribe to keyboard controller
  if (kbc_subscribe_int() != 0) {
    return 1;
  }

  //initialize video
  if (vg_initi(mode) != 0) {
    printf("video initi failed\n");
    return 1;
  }

  //calculate the width by diving the max width with the amount of rectangles
  width = vmi_p.XResolution / no_rectangles;

  //calculate the height by diving the max height with the amount of rectangles
  heigth = vmi_p.YResolution / no_rectangles;

  //color are calculate differently depending on the mode
  switch (mode) {
    case 0x105:
      //loop through all the rectangle in the y axis
      for (int i = 0; i < no_rectangles; i++) {
        //loop through all the rectangle in the x axis
        for (int j = 0; j < no_rectangles; j++) {
          //if the rectangle surpasses the max width, stop
          if((j * width) > vmi_p.XResolution) {
            break;
          }

          //generate a color depending on the place 
          //changes color depending on postiong x and y and multiplies by step to see how diferent the color will be
          index = (first + (i * no_rectangles + j) * step) % (1 << vmi_p.BitsPerPixel);
      
          //draw rectangle on the right place with the right color
          vg_draw_rectangle(j * width, i * heigth, width, heigth, index);
        }
         //if the rectangle surpasses the max height, stop
         if((i * heigth) > vmi_p.YResolution) {
            break;
          }
      }
      break;
    case 0x110:
      //loop through all the rectangle in the y axis
      for (int i = 0; i < no_rectangles; i++) {
        //loop through all the rectangle in the x axis
        for (int j = 0; j < no_rectangles; j++) {
          //if the rectangle surpasses the max width, stop
          if((j * width) > vmi_p.XResolution) {
            break;
          }
          //calculate red for each position (mod gets rid of unwanted bits)
          first_red = (((0x007c00 & first) >> vmi_p.RedFieldPosition) + j * step) % (1 << vmi_p.RedMaskSize);
          //calculate green for each position 
          first_green = (((0x0003e0 & first) >> vmi_p.GreenFieldPosition) + i * step) % (1 << vmi_p.GreenMaskSize);
          //calculate blue for each position 
          first_blue = (((0x00001F & first) >> vmi_p.BlueFieldPosition) + (i + j) * step) % (1 << vmi_p.BlueMaskSize);

          //mix of red, green, blue
          color = (first_red << vmi_p.RedFieldPosition) | (first_green << vmi_p.GreenFieldPosition) | (first_blue << vmi_p.BlueFieldPosition);

          //draw rectangle on the right place with the right color 
          vg_draw_rectangle(j * width, i * heigth, width, heigth, color);
        }
         //if the rectangle surpasses the max height, stop
         if((i * heigth) > vmi_p.YResolution) {
            break;
          }
      }
      break;
    case 0x115:
      //loop through all the rectangle in the y axis
      for (int i = 0; i < no_rectangles; i++) {
        //loop through all the rectangle in the x axis
        for (int j = 0; j < no_rectangles; j++) {
          //if the rectangle surpasses the max width, stop
          if((j * width) > vmi_p.XResolution) {
            break;
          }
          first_red = (((0xFF0000 & first) >> vmi_p.RedFieldPosition) + j * step) % (1 << vmi_p.RedMaskSize);
          first_green = (((0x00FF00 & first) >> vmi_p.GreenFieldPosition) + i * step) % (1 << vmi_p.GreenMaskSize);
          first_blue = (((0x0000FF & first) >> vmi_p.BlueFieldPosition) + (i + j) * step) % (1 << vmi_p.BlueMaskSize);
          color = 0 | (first_red << vmi_p.RedFieldPosition) | (first_green << vmi_p.GreenFieldPosition) | (first_blue << vmi_p.BlueFieldPosition);
          //draw rectangle on the right place with the right color 
          vg_draw_rectangle(j * width, i * heigth, width, heigth, color);
        }
         //if the rectangle surpasses the max height, stop
         if((i * heigth) > vmi_p.YResolution) {
            break;
          }
      }
      break;
    case 0x11a:
      //loop through all the rectangle in the y axis
      for (int i = 0; i < no_rectangles; i++) {
        //loop through all the rectangle in the x axis
        for (int j = 0; j < no_rectangles; j++) {
          //if the rectangle surpasses the max width, stop
          if((j * width) > vmi_p.XResolution) {
            break;
          }
          first_red = (((0x00f800 & first) >> vmi_p.RedFieldPosition) + j * step) % (1 << vmi_p.RedMaskSize);
          first_green = (((0x0007e0 & first) >> vmi_p.GreenFieldPosition) + i * step) % (1 << vmi_p.GreenMaskSize);
          first_blue = (((0x00001F & first) >> vmi_p.BlueFieldPosition) + (i + j) * step) % (1 << vmi_p.BlueMaskSize);
          color = (first_red << vmi_p.RedFieldPosition) | (first_green << vmi_p.GreenFieldPosition) | (first_blue << vmi_p.BlueFieldPosition);
          //draw rectangle on the right place with the right color 
          vg_draw_rectangle(j * width, i * heigth, width, heigth, color);
        }
         //if the rectangle surpasses the max height, stop
         if((i * heigth) > vmi_p.YResolution) {
            break;
          }
      }
      break;
    case 0x14c:
      //loop through all the rectangle in the y axis
      for (int i = 0; i < no_rectangles; i++) {
        //loop through all the rectangle in the x axis
        for (int j = 0; j < no_rectangles; j++) {
          //if the rectangle surpasses the max width, stop
          if((j * width) > vmi_p.XResolution) {
            break;
          }
          first_red = (((0xFF0000 & first) >> vmi_p.RedFieldPosition) + j * step) % (1 << vmi_p.RedMaskSize);
          first_green = (((0x00FF00 & first) >> vmi_p.GreenFieldPosition) + i * step) % (1 << vmi_p.GreenMaskSize);
          first_blue = (((0x0000FF & first) >> vmi_p.BlueFieldPosition) + (i + j) * step) % (1 << vmi_p.BlueMaskSize);
          color = (first_red << vmi_p.RedFieldPosition) | (first_green << vmi_p.GreenFieldPosition) | (first_blue << vmi_p.BlueFieldPosition);
          //draw rectangle on the right place with the right color 
          vg_draw_rectangle(j * width, i * heigth, width, heigth, color);
          //if the rectangle surpasses the max height, stop
          if(i * heigth > vmi_p.YResolution) {
            break;
          }
        }
      }
      break;
      default:
      break;
  }

  // int where ipc status will be placed
  int ipc_status;
  // message
  message msg;
  // just to print driver's error message
  double r = 0;

  write_to_video_mem();
  // while the ESC key isn't pressed
  while (scancode != ESC) {
    // read driver message from kernel
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
      printf("driver_receive failed with: %d", r);
      // in case of an error, this keeps trying to read (could use return 1 instead)
      continue;
    }
    // check if ipc_status is a notification
    if (is_ipc_notify(ipc_status)) {
      // check if message is from hardware
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:
          // if message and irq_set are the same
          if (msg.m_notify.interrupts & irq_set) {
            // reads the scancodes
            kbc_ih();
          }
          break;
        default:
          break;
      }
    }
  }

  // unsubscribe keyboard controller
  if (kbc_unsubscribe_int() != 0) {
    return 1;
  }

  //exit video
  if (vg_exit() != 0) {
    printf("exit failed\n");
    return 1;
  }

  //return 0 if successful
  return 0;

}


int(video_test_xpm)(xpm_map_t xpm, uint16_t x, uint16_t y) {
  uint8_t bit_no = IRQ1;

  // set irq to the bit of bit_no
  uint8_t irq_set = BIT(bit_no);

  // subscribe to keyboard controller
  if (kbc_subscribe_int() != 0) {
    return 1;
  }

  if (vg_initi(0x105) != 0) {
    printf("video initi failed\n");
    return 1;
  }

  if(draw_xpm(xpm, x, y) != 0) {
    printf("xpm failed to load\n");
    return 1;
  }

  // int where ipc status will be placed
  int ipc_status;
  // message
  message msg;
  // just to print driver's error message
  double r = 0;

  write_to_video_mem();
  // while the ESC key isn't pressed
  while (scancode != ESC) {
    // read driver message from kernel
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
      printf("driver_receive failed with: %d", r);
      // in case of an error, this keeps trying to read (could use return 1 instead)
      continue;
    }
    // check if ipc_status is a notification
    if (is_ipc_notify(ipc_status)) {
      // check if message is from hardware
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:
          // if message and irq_set are the same
          if (msg.m_notify.interrupts & irq_set) {
            // reads the scancodes
            kbc_ih();
          }
          break;
        default:
          break;
      }
    }
  }

  // unsubscribe keyboard controller
  if (kbc_unsubscribe_int() != 0) {
    return 1;
  }

  if (vg_exit() != 0) {
    printf("exit failed\n");
    return 1;
  }

  return 0;
}

int(video_test_move)(xpm_map_t xpm, uint16_t xi, uint16_t yi, uint16_t xf, uint16_t yf,
                     int16_t speed, uint8_t fr_rate) {
  xpm_image_t img;
  
  timer_set_frequency(0,fr_rate);
  uint8_t bit_no = IRQ1;
  uint8_t bit_no_timer = IRQ0;

  // set irq to the bit of bit_no
  uint8_t irq_set = BIT(bit_no);
  uint8_t irq_set_timer = BIT(bit_no_timer);

  // subscribe to keyboard controller
  if (kbc_subscribe_int() != 0) {
    return 1;
  }

  if(timer_subscribe_int(&bit_no_timer) != 0) {
    return 1;
  }

  if (vg_initi(0x105) != 0) {
    printf("video init failed\n");
    return 1;
  }

  uint8_t *sprite = xpm_load(xpm, XPM_INDEXED, &img);

  if(draw_sprite(img, sprite, xi, yi) != 0) {
    return 1;
  }

  // int where ipc status will be placed
  int ipc_status;
  // message
  message msg;
  // just to print driver's error message
  double r = 0;

  bool done = false;

  int counter = 0;

  write_to_video_mem();
  // while the ESC key isn't pressed
  while (scancode != ESC) {
    // read driver message from kernel
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
      printf("driver_receive failed with: %d", r);
      // in case of an error, this keeps trying to read (could use return 1 instead)
      continue;
    }
    // check if ipc_status is a notification
    if (is_ipc_notify(ipc_status)) {
      // check if message is from hardware
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:
          // if message and irq_set are the same
          if (msg.m_notify.interrupts & irq_set) {
            // reads the scancodes
            kbc_ih();
          }
          if(msg.m_notify.interrupts & irq_set_timer) {
            timer_int_handler();
            if (!done) {
                if(speed < 0 && counter % (-speed) == 0){
                    if (xi == xf) {
                    if(vg_draw_rectangle(xi, yi, img.width, img.height, 0) != 0) {
                      return 1;
                    }
                    yi = yi + 1;
                    if (yi > yf) {
                      yi = yf;
                    }
                    if(draw_sprite(img, sprite, xi, yi) != 0) {
                      return 1;
                    }
                    if (yi == yf)
                      done = true;
                  }
                  else if (yi == yf) {
                    if(vg_draw_rectangle(xi, yi, img.width, img.height, 0) != 0) {
                      return 1;
                    }
                    xi = xi + 1;
                    if (xi > xf) {
                      xi = xf;
                    }
                    if(draw_sprite(img, sprite, xi, yi) != 0) {
                      return 1;
                    }
                    if (xi == xf)
                      done = true;
                  }
                }
                else if(speed > 0){
                  if (xi == xf) {
                  if(vg_draw_rectangle(xi, yi, img.width, img.height, 0) != 0) {
                    return 1;
                  }
                  yi = yi + speed;
                  if (yi > yf) {
                    yi = yf;
                  }
                  if(draw_sprite(img, sprite, xi, yi) != 0) {
                    return 1;
                  }
                  if (yi == yf)
                    done = true;
                }
                else if (yi == yf) {
                  if(vg_draw_rectangle(xi, yi, img.width, img.height, 0) != 0) {
                    return 1;
                  }
                  xi = xi + speed;
                  if (xi > xf) {
                    xi = xf;
                  }
                  if(draw_sprite(img, sprite, xi, yi) != 0) {
                    return 1;
                  }
                  if (xi == xf)
                    done = true;
                }
              }
                write_to_video_mem();
              }
          }
          break;
        default:
          break;
      }
    }
  }

  // unsubscribe keyboard controller
  if (kbc_unsubscribe_int() != 0) {
    return 1;
  }

  if(timer_unsubscribe_int() != 0) {
    return 1;
  }

  if (vg_exit() != 0) {
    printf("exit failed\n");
    return 1;
  }

  return 0;
}

int(video_test_controller)() {
  /* To be completed */
  printf("%s(): under construction\n", __func__);

  return 1;
}
