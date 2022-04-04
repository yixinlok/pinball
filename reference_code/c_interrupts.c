#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#define TIMER_BASE            0xFF202000
#define PS2_BASE              0xFF200100

#define TIMER_IRQ 29    
#define PS2_IRQ 79

void set_IRQ_stack (void); 
void config_GIC (void);
void config_private_timer (void); 
void config_ps2_keyboard (void); 
void enable_interrupts (void);
void private_timer_ISR (void); 
void ps2_keyboard_ISR (void); 

/* These global variables are written by interrupt service routines; we have to declare these as volatile * to avoid the compiler caching their values in registers */
volatile int ticks = 0;
volatile bool space_bar_pressed = false;

/* Main program ********************************************************************************/
int main(void){
    // volatile int HPS_timer_LEDG = 0x01000000; // value to turn on the HPS green light LEDG

    set_IRQ_stack (); 
    config_GIC (); 
    config_private_timer (); 
    config_ps2_keyboard (); 
    
    //the fuck does this next line do
    //*(HPS_GPIO1_ptr + 0x1) = HPS_timer_LEDG; // write to the data direction register to set // bit 24 (LEDG) to be an output

    enable_interrupts ();

    while (1){
        if(space_bar_pressed){
            printf("Space bar pressed \n");
            space_bar_pressed = false;
        }

        if(tick){
            printf("Ticks: %d \n", &ticks);
            ticks = 0;
        }
    }
}

void set_IRQ_stack(void){
    int stack, mode;
    stack = 0xFFFFFFFF − 7; // top of A9 on-chip memory, aligned to 8 bytes /* change processor to IRQ mode with interrupts disabled */
    mode = 0b11010010;
    asm("msr cpsr, %[ps]" : : [ps] "r" (mode));
    /* set banked stack pointer */
    asm("mov sp, %[ps]" : : [ps] "r" (stack));
    /* go back to SVC mode before executing subroutine return! */ 
    mode = 0b11010011;
    asm("msr cpsr, %[ps]" : : [ps] "r" (mode));
}

void enable_interrupts(void){
    int status = 0b01010011;
    asm("msr cpsr, %[ps]" : : [ps]"r"(status)); 
}

void config_GIC (){
    // complete this for private timer and

    /* configure the HPS timer interrupt */ 
    *((int *) 0xFFFED8C4) = 0x01000000; 
    *((int *) 0xFFFED118) = 0x00000080;

    /* configure the FPGA interval timer and KEYs interrupts */ 
    *((int *) 0xFFFED848) = 0x00000101;
    *((int *) 0xFFFED108) = 0x00000300;

    // Set Interrupt Priority Mask Register (ICCPMR). Enable interrupts of all priorities 
    *((int *) 0xFFFEC104) = 0xFFFF;
    // Set CPU Interface Control Register (ICCICR). Enable signaling of interrupts 
    *((int *) 0xFFFEC100) = 1; // enable = 1
    // Configure the Distributor Control Register (ICDDCR) to send pending interrupts to CPUs
    *((int *) 0xFFFED000) = 1; // enable = 1
}

void config_private_timer (){

}

void config_ps2_keyboard (){

}

void __attribute__ ((interrupt)) __cs3_isr_irq (void) {
    // Read the ICCIAR from the processor interface 
    int int_ID = *((int *) 0xFFFEC10C);
    if (int_ID == TIMER_IRQ) private_timer_ISR ();
    else if (int_ID == PS2_IRQ) ps2_keyboard_ISR ();
    else while (1);
    // Write to the End of Interrupt Register (ICCEOIR) 
    *((int *) 0xFFFEC110) = int_ID;
    return;
}

void __attribute__ ((interrupt)) __cs3_isr_undef (void) {
    while (1); 
}
void __attribute__ ((interrupt)) __cs3_isr_swi (void) {
    while (1); 
}
void __attribute__ ((interrupt)) __cs3_isr_pabort (void) {
    while (1); 
}
void __attribute__ ((interrupt)) __cs3_isr_dabort (void) {
    while (1); 
}
void __attribute__ ((interrupt)) __cs3_isr_fiq (void) {
    while (1); 
}

void private_timer_ISR (void){
    // the usual stuff
    // in our program we would update the velocity vector and position vector
}

void ps2_keyboard_ISR (void){
    // check if space bar is pressed, if yes
}