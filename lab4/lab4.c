// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>

#include <stdint.h>
#include <stdio.h>

#include "i8042.h"
#include "mouse.h"

extern struct packet packet0;
extern uint32_t total_packets;
extern bool error_reading;
extern uint8_t packet;
extern int counter;
extern int counter_t;
int counter_exit = 0;
int x_displacement = 0;
int y_displacement = 0;
// Any header files included below this line should have been created by you

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need/ it]
  lcf_trace_calls("/home/lcom/labs/lab4/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab4/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

//test mouse packet
int (mouse_test_packet)(uint32_t cnt) {
    //enable mouse
    if(enable_mouse() != 0) {
      return 1;
    }

    uint8_t bit_no = IRQ1;
    uint32_t irq_set = BIT(bit_no);

    //subscribe to mouse
    if(mouse_subscribe_int() != 0) {
      return 1;
    }

    int ipc_status;
    message msg;
    double r = 0;

    //while we haven't reached the total number of packets
    while(total_packets < cnt) {
        if ((r = driver_receive(ANY,&msg, &ipc_status)) != 0){
      printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status)){
      switch (_ENDPOINT_P(msg.m_source)){
        case HARDWARE:
        if (msg.m_notify.interrupts & irq_set) {
          mouse_ih();
          //if error and counter is not 2 (haven't read everything yet)
          if((error_reading == true) && counter != 2) {
              //no problem for now
              continue;
          }
          //else
          else if((error_reading == true) && counter == 2) {
              //reset error reader
              error_reading = false;
          }
          else {
            //print mouse packet
            print_mouse_packet();
          }
         
        }
        break;
          
        default: break;

      }
    }
    }

  if(mouse_unsubscribe_int() != 0) {
    return 1;
  }

  if(disable_mouse() != 0) {
    return 1;
  }

    return 0;
}

int (mouse_test_async)(uint8_t idle_time) {
    uint8_t bit_no_m = IRQ1;
    uint8_t bit_no_t = IRQ0;
    uint32_t irq_set_m = BIT(bit_no_m);
    uint32_t irq_set_t = BIT(bit_no_t);

    if(enable_mouse() != 0) {
      return 1;
    }

    if(mouse_subscribe_int() != 0) {
      return 1;
    }

    if(timer_subscribe_int(&bit_no_t) != 0) {
      return 1;
    }

    int ipc_status;
    message msg;
    double r = 0;

    while(counter_t < idle_time * 60) {
        if ((r = driver_receive(ANY,&msg, &ipc_status)) != 0){
      printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status)){
      switch (_ENDPOINT_P(msg.m_source)){
        case HARDWARE:
        if (msg.m_notify.interrupts & irq_set_m) {
          counter_t = 0;
          mouse_ih();
          if((error_reading == true) && counter != 2) {
              continue;
          }
          else if((error_reading == true) && counter == 2) {
              error_reading = false;
          }
          else {
            print_mouse_packet();
          }
         
        }
        if (msg.m_notify.interrupts & irq_set_t) {
            timer_int_handler();
          }
        break;
          
        default: break;

      }
    }
    }

  if(mouse_unsubscribe_int() != 0) {
    return 1;
  }

  if(disable_mouse() != 0) {
    return 1;
  }

  if(timer_unsubscribe_int() != 0) {
    return 1;
  }

    return 0;
}

//displays the packets received from the mouse with a logical exit condition
int (mouse_test_gesture)(uint8_t x_len, uint8_t tolerance) {
    //enable mouse
    if(enable_mouse() != 0) {
      return 1;
    }

    uint8_t bit_no = IRQ1;
    uint32_t irq_set = BIT(bit_no);
    //create a counter to see how many times an exit has been called?
    counter_exit = 0;

    //subscribe mouse
    if(mouse_subscribe_int() != 0) {
      return 1;
    }

    int ipc_status;
    message msg;
    double r = 0;

    //while counter_exit hasn't reached 6 yet
    while(counter_exit != 6) {
        if ((r = driver_receive(ANY,&msg, &ipc_status)) != 0){
      printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status)){
      switch (_ENDPOINT_P(msg.m_source)){
        case HARDWARE:
        if (msg.m_notify.interrupts & irq_set) {
          mouse_ih();
          if((error_reading == true) && counter != 2) {
              continue;
          }
          else if((error_reading == true) && counter == 2) {
              error_reading = false;
          }
          else {
            //print mouse packet
            print_mouse_packet();
            //if new code
            if(counter == 0) {
              //if left mouse button selected and counter_exit is 0, counter_exit becomes 1
              if(packet0.lb && counter_exit == 0) {
                counter_exit = 1;
              }
              //else if left mouse button selected and packet position x + tolerance is more or equals to 0 and packet position y + tolarence is also more or equal to 0 and counter_exit is 1
              else if(packet0.lb && (packet0.delta_x + tolerance) >= 0 && (packet0.delta_y + tolerance) >= 0 && counter_exit == 1) {
                //displacement of x is incremented by the current position of x
                x_displacement += packet0.delta_x;
                //if the displacemnt of x is more or equals to the minimum value specified
                if (x_displacement >= x_len) {
                  //counter_exit becomes 2
                  counter_exit = 2;
                }
              }
              //if only counter_exit == 1 then reset 
              else if(counter_exit == 1) {
                counter_exit = 0;
                x_displacement = 0;
              }
              //if release left mouse button and the counter_exit is 2
              else if(!packet0.lb && counter_exit == 2) {
                //counter_exit becomes 3 and the displacemente resets
                counter_exit = 3;
                x_displacement = 0;
              }
              //if press right mouse button and counter exit is 3
              else if(packet0.rb && counter_exit == 3) {
                //counter exit becomes 4
                counter_exit = 4;
              }
              //else if right mouse button selected and packet position x + tolerance is more or equals to 0 and packet position y + tolarence is also less or equal to 0 and counter_exit is 4
              else if(packet0.rb && (packet0.delta_x + tolerance) >= 0 && (packet0.delta_y - tolerance) <= 0 && counter_exit == 4) {
                //diplacement of x gets incremented with position of x
                x_displacement += packet0.delta_x;
                //if displacamente of x is more or equals to the minimum value specified
                if(x_displacement >= x_len) {
                  //increment counter exit
                  counter_exit = 5;
                }
              }
              //else if counter exit is 4 but nothing else
              else if (counter_exit == 4) {
                // reset x displacemente and counter exit
                x_displacement = 0;
                counter_exit = 0;
              }
              //if right mouse button released and counter exit is 5
              else if(!packet0.rb && counter_exit == 5) {
                //if left mouse button is pressed
                if(packet0.lb) {
                  // reset exit counter
                  counter_exit = 0;
                }
                //else increment exit counter to 6
                counter_exit = 6;
              }
              //if counter_exit == 5 and nothing else
              else if(counter_exit == 5) {
                //reset exit counter
                counter_exit = 0;
              }

            }

        }
    }
    break;
    default: break;
          }
    }
  }

   //unsubscribe mouse
   if(mouse_unsubscribe_int() != 0) {
    return 1;
   }

   //disable mouse
   if(disable_mouse() != 0) {
    return 1;
   }

   //return 0 if success
    return 0;
}


int (mouse_test_remote)(uint16_t period, uint8_t cnt) {
    /* This year you need not implement this. */
    printf("%s(%u, %u): under construction\n", __func__, period, cnt);
    return 1;
}
