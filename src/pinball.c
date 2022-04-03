#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "globals.h"
#include "address_map_arm.h"
#include "resources/freeplay_template.h"
#include "resources/start_template.h"
#include "resources/end_template.h"



int main(void){
    volatile int * pixel_ctrl_ptr = (int *)PIXEL_BUF_CTRL_BASE;
    pixel_buffer_init(pixel_ctrl_ptr);

    
    clear_screen(); // pixel_buffer_start points to the pixel buffer
    return 1;
}

/*
    HARDWARE STUFF
*/

void pixel_buffer_init(int *pixel_ctrl_ptr){
    *(pixel_ctrl_ptr + 1) = FPGA_ONCHIP_BASE; 
    wait_for_vsync();
    pixel_buffer_start = *pixel_ctrl_ptr;
    clear_screen(); 
    *(pixel_ctrl_ptr + 1) = SDRAM_BASE;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1);
}

/*
    GAME STATES
*/
void freeplay(){
    collide_id = check_collision();
    erase();
    // update();
    draw();
}

/*
    KINEMATICS FUNCTIONS
*/
// if flipper countdown != 0, this function is called
void animate_flipper(){
    update_flipper_end_location(flipper_angles[flipper_angle_counter-1]);
    flipper_angle_counter -= 1;
}

void update_flipper_end_location(double angle){
    //update prev
    for(int i = 0; i < 2; i++){ //left or right flipper
        //update x and y
        prev_flipper_end_location[i][0] = flipper_end_location[i][0]; 
        prev_flipper_end_location[i][1] = flipper_end_location[i][1]; 
    }
    //update current
    int xleft = FLIPPER_L_X + FLIPPER_LENGTH * cos(angle);
    int yleft = FLIPPER_L_Y + FLIPPER_LENGTH * sin(angle);
    flipper_end_location[0][0] = xleft;
    flipper_end_location[0][1] = yleft;

    int xright = FLIPPER_R_X - FLIPPER_LENGTH * cos(angle);
    int yright = FLIPPER_R_Y + FLIPPER_LENGTH * sin(angle);
    flipper_end_location[1][0] = xright;
    flipper_end_location[1][1] = yright;
}

/* 
    DRAW AND ERASE FUNCTIONS
*/
void erase(){
    erase_flippers();
    erase_ball(prev_ball_location[0], prev_ball_location[1]);
}

void draw(){
    draw_flippers();
    draw_ball(ball_location[0], ball_location[1]);
    //draw_score();
}

void plot_pixel(int x, int y, short int line_color)
{
    *(short int *)(pixel_buffer_start + (y << 10) + (x << 1)) = line_color;
}

//tested
void draw_ball(int x, int y){
    for(int j = -6; j <= 6; j++){
        int limit = 6;
        if(j==-6 || j == 6) limit =2;
        if(j==-5 || j == 5) limit =3;
        if(j==-4 || j == 4) limit =4;
        if(j==-3 || j == 3) limit =5;
        for(int i = -limit; i <= limit; i++){
            plot_pixel(x+i, y+j, ball_colours[j+6][i+6]);
        }
    }
}

void erase_ball(int x, int y){
    for(int j = -6; j <= 6; j++){
        int limit = 6;
        if(j==-6 || j == 6) limit =2;
        if(j==-5 || j == 5) limit =3;
        if(j==-4 || j == 4) limit =4;
        if(j==-3 || j == 3) limit =5;
        for(int i = -limit; i <= limit; i++){
            plot_pixel(x+i, y+j, freeplay[j+6][i+6]);
        }
    }
}

void draw_start_template( ){
    for(int i = 0; i < 320; i++){
        for(int j = 0; j < 240; j++){
            plot_pixel(i, j, start[j][i]);
        }
    }
}

void draw_freeplay_template( ){
    for(int i = 0; i < 320; i++){
        for(int j = 0; j < 240; j++){
            plot_pixel(i, j, freeplay[j][i]);
        }
    }
}

void draw_flippers(){
    int xleft = flipper_end_location[0][0];
    int yleft = flipper_end_location[0][1];
    int xright = flipper_end_location[1][0];
    int yright = flipper_end_location[1][1];

    draw_thick_line(FLIPPER_L_X, FLIPPER_L_Y, xleft, yleft, WHITE);
    draw_thick_line(FLIPPER_R_X, FLIPPER_R_Y, xright, yright, WHITE);
}

void erase_flippers(){
    int xleft = prev_flipper_end_location[0][0];
    int yleft = prev_flipper_end_location[0][1];
    int xright = prev_flipper_end_location[1][0];
    int yright = prev_flipper_end_location[1][1];

    draw_thick_line(FLIPPER_L_X, FLIPPER_L_Y, xleft, yleft, BLACK);
    draw_thick_line(FLIPPER_R_X, FLIPPER_R_Y, xright, yright, BLACK);
}

void draw_thick_line(int x0, int y0, int x1, int y1, short int colour){
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
    COLLISION CHECKING FUNCTIONS
*/
int check_collision(int ball_x, int ball_y){
    int collide_id = 0;
    //check in the following order for optimal speed:
    
    //check straight walls
    if(ball_y<= roof) return WALL_COLLIDE;
    for(int i = 0; i<NUM_VERTICAL_WALLS; i++){
        if(ball_x== vertical_walls[i][0]){
            if(ball_y > vertical_walls[i][1] && ball_y < vertical_walls[i][2]) return WALL_COLLIDE;
        }
    }

    //check flippers
    if(check_flipper_collide(0, ball_x,ball_y)) return FLIPPER_COLLIDE; //left
    if(check_flipper_collide(1, ball_x,ball_y)) return FLIPPER_COLLIDE; //right

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
            if(ball_x == y && ball_y == x) return true;
		}
        else {
            if(ball_x == x && ball_y == y) return true;
		}
        error += dy;
        if(error > 0){
            y += y_step;
            error -= dx;
        }
    }
    return false;
}

bool check_flipper_collide(int LR, int ball_x, int ball_y){
    if(LR==0){
        int x0 = FLIPPER_L_X;
        int y0 = FLIPPER_L_Y;
        int x1 = flipper_end_location[0][0];
        int y1 = flipper_end_location[0][1];
    }
    else{
        int x0 = FLIPPER_R_X;
        int y0 = FLIPPER_R_Y;
        int x1 = slanted_walls[wall_id][1][0];
        int y1 = slanted_walls[wall_id][1][1];
    }

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
            //if distance between point on the flipper is smaller than flipper radius + ball radius
            if(sqrt((ball_x - y)^2 + (ball_y - x)^2) <  10) return true;
		}
        else {
            if(sqrt((ball_x - x)^2 + (ball_y - y)^2) <  10) return true;
		}
        error += dy;
        if(error > 0){
            y += y_step;
            error -= dx;
        }
    }
    return false;
}
