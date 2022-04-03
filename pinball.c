#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "globals.h"
#include "address_map_arm.h"


int main(void){
    volatile int * pixel_ctrl_ptr = (int *)PIXEL_BUF_CTRL_BASE;
    pixel_buffer_init(pixel_ctrl_ptr);

    
    clear_screen(); // pixel_buffer_start points to the pixel buffer
    return 1;
}

void pixel_buffer_init(int *pixel_ctrl_ptr){
    *(pixel_ctrl_ptr + 1) = FPGA_ONCHIP_BASE; 
    wait_for_vsync();
    pixel_buffer_start = *pixel_ctrl_ptr;
    clear_screen(); 
    *(pixel_ctrl_ptr + 1) = SDRAM_BASE;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1);
}

/*
    PHYSICS FUNCTIONS
*/

/* 
    DRAWING FUNCTIONS
*/
void plot_pixel(int x, int y, short int line_color)
{
    *(short int *)(pixel_buffer_start + (y << 10) + (x << 1)) = line_color;
}

void draw_ball(int x, int y){
    // MEDIUM BEIGE
    for(int i = -6 ; i <= 6 ; i++){
        int limit = 6;
        if( i == -6 || i == 6) limit = 2;
        if( i == -5 || i == 5) limit = 3;
        if( i == -4 || i == 4) limit = 4;
        if( i == -3 || i == 3) limit = 5;
        for(int j = -limit ; j <= limit ; i++){
            plot_pixel(x+i,y+j,MEDIUM_BEIGE);
        }
    }
    // LIGHT BEIGE

    // DARK BEIGE
}

void draw_flippers(int angle){
    int x1 = FLIPPER_L_X + FLIPPER_LENGTH * cos(angle);
    int y1 = FLIPPER_L_Y + FLIPPER_LENGTH * sin(angle);
    draw_thick_line(FLIPPER_L_X, FLIPPER_L_Y, x1, y1);

    x1 = FLIPPER_R_X - FLIPPER_LENGTH * cos(angle);
    y1 = FLIPPER_R_Y + FLIPPER_LENGTH * sin(angle);
    draw_thick_line(FLIPPER_R_X, FLIPPER_R_Y, x1, y1);
}

void draw_thick_line(int x0, int y0, int x1, int y1){

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
			plot_pixel(y,x,WHITE);
		}
        else {
			plot_pixel(x,y,WHITE);
		}
        error += dy;
        if(error > 0){
            y += y_step;
            error -= dx;
        }
    }
}

void draw_box(int x, int y, short int colour){
    for(int i = -2; i <= 3 ; i++){
        plot_pixel(x+3,y+i,colour);
        plot_pixel(x+2,y+i,colour);
        plot_pixel(x+1,y+i,colour);
        plot_pixel(x,y+i,colour);
        plot_pixel(x-1,y+i,colour);
        plot_pixel(x-2,y+i,colour);
    }    
}

void swap(int* p1, int* p2){
    int temp;
    temp = *p1;
    *p1 = *p2;
    *p2 = temp;
}

/*
    COLLISION FUNCTIONS
*/
int check_collision(int ball_x, int ball_y){
    int collide_id = 0;
    //check in the following order for optimal speed:
    
    if(collide_id!=0) return collide_id;

    //check straight walls
    if(ball_y<= roof) return WALL_COLLIDE;
    for(int i = 0; i<NUM_VERTICAL_WALLS; i++){
        if(ball_x== vertical_walls[i][0]){
            if(ball_y > vertical_walls[i][1] && ball_y < vertical_walls[i][2]) return WALL_COLLIDE;
        }
    }

    //check flippers
    

    //check slanted walls
    for(int i = 0; i<NUM_SLANTED_WALLS; i++){
        if(check_slanted_wall_collide(i,ball_x,ball_y)) return WALL_COLLIDE;
    }
    //check bumpers

    return collide_id;
}

//tested
bool check_slanted_wall_collide(int wall_id, int ball_x, int ball_y){
    int x0 = slanted_walls[wall_id][0][0];
    int y0 = slanted_walls[wall_id][0][1];
    int x1 = slanted_walls[wall_id][1][0];
    int y1 = slanted_walls[wall_id][1][1];

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
            if(ball_x ==y && ball_y ==x) printf("works\n");
			return true;
		}
        else {
            if(ball_x ==x && ball_y ==y) printf("works\n");
			return true;
		}
        error += dy;
        if(error > 0){
            y += y_step;
            error -= dx;
        }
    }
    return false;
}