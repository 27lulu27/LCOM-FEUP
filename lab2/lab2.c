#include <lcom/lcf.h>
#include <lcom/lab2.h>

#include <stdbool.h>
#include <stdint.h>

extern int counter;
extern int hook_id;


int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/lab2/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab2/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

/*Read and print the configuration of the timer
(timer -> 8 bit number that contains what timer we're working with)
(field -> it's a selector for what information of the status we want displayed)
*/
int(timer_test_read_config)(uint8_t timer, enum timer_status_field field) {
  uint8_t st = 0; //8 bit number to store the timer status
  //get the configuration of the timer
  if(timer_get_conf(timer, &st) != 0) {
    return 1;
  }

  //display the configuration of the timer2
  if(timer_display_conf(timer, st, field) != 0) {
    return 1;
  }

  //return 0 if success
  return 0;
}

/*Set the frequency of the timer
(timer -> 8 bit number that contains what timer we're working with)
(freq -> 32 bit number of the frequency that we want to set)
*/
int(timer_test_time_base)(uint8_t timer, uint32_t freq) {
  //set frequency of the timer
  if(timer_set_frequency(timer, freq) == 1) {
    return 1;
  }

  //return 0 if success
  return 0;
}

/*Use interrupts to count the seconds
(time -> amount of seconds)*/
int(timer_test_int)(uint8_t time) {


  uint8_t bit_no = 0;
  //interrupt request is set to bit_no
  uint32_t irq_set = BIT(bit_no);

  //subscribe irq line
  if(timer_subscribe_int(&bit_no) != 0){
    return 1;
  }

  //set frequency of the timer
  if(timer_set_frequency(time ,60) == 1) {
    return 1;
  }
  
  

  //int counter = 0;
  //create a message
  message msg;
  //create an int that will store the ipc status
  int ipc_status;

 //while there's still time
  while(time > 0) { 
    // read message from kernel if failure return 1
     if ( (driver_receive(ANY, &msg, &ipc_status)) == 1 ) { 
         return 1;
     }
     // check if message is notification
     if (is_ipc_notify(ipc_status)) { 
          //check if message is from hardware
          if(_ENDPOINT_P(msg.m_source) == HARDWARE){ 
                //checks if the notification is from the correct device
                if (msg.m_notify.interrupts & irq_set) {
                   //handler of the interrupt (in this case, increments the counter)
                   timer_int_handler();
                   //check if a second has passed
                   if(counter % 60 == 0){
                      // print elapsed time
                      timer_print_elapsed_time();
                      //reduce time
                      time--;
                      }
                }   
          }
     }
  }

 // unsubscribe irq line
 if(timer_unsubscribe_int() == 1) {
    return 1;
  }

  //return 0 if success
  return 0;
}
