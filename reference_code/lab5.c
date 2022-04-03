/* This files provides address values that exist in the system */

#define SDRAM_BASE            0xC0000000
#define FPGA_ONCHIP_BASE      0xC8000000
#define FPGA_CHAR_BASE        0xC9000000

/* Cyclone V FPGA devices */
#define LEDR_BASE             0xFF200000
#define HEX3_HEX0_BASE        0xFF200020
#define HEX5_HEX4_BASE        0xFF200030
#define SW_BASE               0xFF200040
#define KEY_BASE              0xFF200050
#define TIMER_BASE            0xFF202000
#define PIXEL_BUF_CTRL_BASE   0xFF203020
#define CHAR_BUF_CTRL_BASE    0xFF203030

/* VGA colors */
#define WHITE 0xFFFF
#define YELLOW 0xFFE0
#define RED 0xF800
#define GREEN 0x07E0
#define BLUE 0x001F
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define GREY 0xC618
#define PINK 0xFC18
#define ORANGE 0xFC00

#define ABS(x) (((x) > 0) ? (x) : -(x))

/* Screen size. */
#define RESOLUTION_X 320
#define RESOLUTION_Y 240

/* Constants for animation */
#define BOX_LEN 2
#define NUM_BOXES 8

#define FALSE 0
#define TRUE 1

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>


// Begin part3.c code for Lab 7


volatile int pixel_buffer_start; // global variable

int main(void)
{
    volatile int * pixel_ctrl_ptr = (int *)0xFF203020;
    // declare other variables(not shown)
    // initialize location and direction of rectangles(not shown)

    /* set front pixel buffer to start of FPGA On-chip memory */
    *(pixel_ctrl_ptr + 1) = 0xC8000000; // first store the address in the 
                                        // back buffer
    /* now, swap the front/back buffers, to set the front buffer location */
    wait_for_vsync();
    /* initialize a pointer to the pixel buffer, used by drawing functions */
    pixel_buffer_start = *pixel_ctrl_ptr;
    clear_screen(); // pixel_buffer_start points to the pixel buffer
    /* set back pixel buffer to start of SDRAM memory */
    *(pixel_ctrl_ptr + 1) = 0xC0000000;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1); // we draw on the back buffer
    clear_screen(); // pixel_buffer_start points to the pixel buffer


    int x[8];
    int y[8];

    int dx[8];
    int dy[8];

    int prev_x[8];
    int prev_y[8];

    for(int i = 0; i<8 ; i++){
        x[i] = rand()%320; 
        y[i] = rand()%240;
        dx[i] = (rand()%2)*2-1; 
        dy[i] = (rand()%2)*2-1;
        prev_x[i] = 0;
        prev_y[i] = 0;
    }


    while (1)
    {
        //clear_screen();
        for(int i = 0; i<8 ; i++){
            /* Erase any boxes and lines that were drawn in the last iteration */
            draw_box(prev_x[i], prev_y[i], 0);
            draw_line(prev_x[i], prev_y[i], prev_x[(i+1)%8], prev_y[(i+1)%8], 0); 
        }

        for(int i = 0; i<8 ; i++){  
            // code for updating the locations of boxes (not shown)
            if(x[i] <= 0) dx[i] = 1;
            if(x[i] >= 319) dx[i] = -1;

            if(y[i] <= 0) dy[i] = 1;
            if(y[i] >= 239) dy[i] = -1;
            
            prev_x[i] = x[i];
            prev_y[i] = y[i];
            
            x[i] += dx[i];
            y[i] += dy[i]; 
        }

        // code for drawing the boxes and lines (not shown)
        draw_box(x[0], y[0], WHITE);
        draw_line(x[0], y[0], x[1], y[1], WHITE);
        
        draw_box(x[1], y[1], YELLOW);
        draw_line(x[1], y[1], x[2], y[2], YELLOW);

        draw_box(x[2], y[2], RED);
        draw_line(x[2], y[2], x[3], y[3], RED);

        draw_box(x[3], y[3], GREEN);
        draw_line(x[3], y[3], x[4], y[4], GREEN);

        draw_box(x[4], y[4], BLUE);
        draw_line(x[4], y[4], x[5], y[5], BLUE);

        draw_box(x[5], y[5], CYAN);
        draw_line(x[5], y[5], x[6], y[6], CYAN);

        draw_box(x[6], y[6], ORANGE);
        draw_line(x[6], y[6], x[7], y[7], ORANGE);

        draw_box(x[7], y[7], PINK);
        draw_line(x[7], y[7], x[0], y[0], PINK);
        
        wait_for_vsync(); // swap front and back buffers on VGA vertical sync
        pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer
    }
}

void wait_for_vsync(){
    volatile int * pixel_ctrl_ptr = (int *)0xFF203020;
    int status;
    * pixel_ctrl_ptr = 1;
    while(*(pixel_ctrl_ptr + 3)&1);
    return;
}

// code for subroutines (not shown)
void clear_screen(){
        for (int x = 0; x < 320; x++)
                for (int y = 0; y < 240; y++)
                        plot_pixel (x, y, 0);
}

void draw_box(int x, int y, short int colour){
    plot_pixel(x,y,colour);
    plot_pixel(x+1,y,colour);
    plot_pixel(x,y+1,colour);
    plot_pixel(x+1,y+1,colour);
}

void plot_pixel(int x, int y, short int line_color)
{
    *(short int *)(pixel_buffer_start + (y << 10) + (x << 1)) = line_color;
}

void draw_line(int x0, int y0, int x1, int y1, short int colour){

    bool is_steep = abs(y1-y0) > abs(x1-x0);
    int temp;
    if(is_steep){
        swap(&x0, &y0);
        swap(&x1, &y1);
    }
    if(x0 > x1){
        swap(&x0, &x1);
        swap(&y0, &y1);
    }

    int dx = x1 - x0;
    int dy = abs(y1 - y0);
    int error = -(dx/2);
    int y = y0;
    int y_step;
    if(y0 < y1) y_step = 1;
    else y_step = -1;

    for(int x = x0; x <= x1; x++){
        if(is_steep){
			plot_pixel(y,x,colour);
		}
        else {
			plot_pixel(x,y,colour);
		}
        error += dy;
        if(error > 0){
            y += y_step;
            error -= dx;
        }
    }

}

void swap(int* p1, int* p2){
    int temp;
    temp = *p1;
    *p1 = *p2;
    *p2 = temp;
}