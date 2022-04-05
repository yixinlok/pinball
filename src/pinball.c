#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "globals.h"
#include "address_map_arm.h"
#include "../resources/freeplay_template.h"
#include "../resources/start_template.h"
#include "../resources/end_template.h"
//asians
int main(void){
    volatile int * pixel_ctrl_ptr = (int *)PIXEL_BUF_CTRL_BASE;
    pixel_buffer_init(pixel_ctrl_ptr);

    // while(1){
    //     while(state == START) start();
    //     while(state == FREEPLAY) freeplay();
    //     while(state == END) end();
    // }
    
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
void wait_for_vsync(){
    volatile int * pixel_ctrl_ptr = (int *)0xFF203020;
    int status;
    * pixel_ctrl_ptr = 1;
    while(*(pixel_ctrl_ptr + 3)&1);
    return;
}

bool check_key_press(){
    volatile int * keyboard_ptr = (int *) PS2_BASE; 
	int keyboard_data, RVALID;
    keyboard_data = * keyboard_ptr;
	RVALID = keyboard_data & 0x8000;
    if(RVALID) return true;
    else return false;
}

/*
    GAME STATES
*/
void start(){
    draw_start_template();
    //wait for space key to be pressed
    draw_freeplay_template();
    //display countdown?
    initialise();
    //initialise ball velocity, score, etc
    return;
}

void initialise(){
    launch_angle = rand() % 361;
    score = 0;
    high_score = 0;
    ball_velocity[0] = LAUNCH_SPEED * cos(launch_angle);
    ball_velocity[1] = LAUNCH_SPEED * sin(launch_angle);
    return;
}

void freeplay(){
    collision_type = check_collision(ball_location[0], ball_location[1]);
    erase();
    update();
    draw();
    //wait for vsync stuff
}

void end(){
    draw_end_template();
    return;
}

/*
    PHYSICS FUNCTIONS
*/
void update(){
    update_flippers();
    update_score(); // update score first
    update_ball_velocity();
    update_ball_position();
    return;
}

void update_flippers(){
    if(check_key_press()) flipper_angle_counter = NUM_FLIPPER_ANGLES-1;
    if(flipper_angle_counter >= 0 ) animate_flipper();
}

// if flipper countdown >= 0, this function is called
void animate_flipper(){
    update_flipper_end_location(flipper_angles[flipper_angle_counter]);
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

void update_ball_position(){
    ball_location[0] += round (ball_velocity[0] / 60);
    ball_location[1] += round (ball_velocity[1] / 60);
    return;
}

void update_ball_velocity(){

    switch(collision_type){
        case 0: // if no collision
            ball_velocity[0] += ball_acceleration[0] / 60;
            ball_velocity[1] += ball_acceleration[1] / 60;
            break;
        case WALL_COLLIDE:
        // if vertical wall, multiply x component by negative one
        // if horizontal wall, multiply y component by negative one
            break;
        case FLIPPER_COLLIDE: 
        // ? how poop
            break; 
        default: // default case is for bumpers
        // once again, if top or bottom of bumper, flip y 
        // if left of right, flip x
            ball_velocity[0] = ball_velocity[0];
            break;
    }
    return;
}

void update_score(){


    return;
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
    draw_score();
    wait_for_vsync(); // swap front and back buffers on VGA vertical sync
    //pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer
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
            if(j<320 && i<240) plot_pixel(x+i, y+j, ball_colours[y+j][x+i]);
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

void draw_start_template(){
    for(int i = 0; i < 320; i++){
        for(int j = 0; j < 240; j++){
            plot_pixel(i, j, start[j][i]);
        }
    }
}

void draw_freeplay_template(){
    for(int i = 0; i < 320; i++){
        for(int j = 0; j < 240; j++){
            plot_pixel(i, j, freeplay[j][i]);
        }
    }
}

void draw_end_template(){
    for(int i = 0; i < 320; i++){
        for(int j = 0; j < 240; j++){
            plot_pixel(i, j, freeplay[j][i]);
        }
    }
}

void draw_digit(int place, int number, bool high){
    // if high, offset y by certain amount, and recursively call function
    // if place is 1 or 2, offset x by a certain amount (in negative direction)
    // smth like x0 - place * 10 and y0 + high * 30
    if(high)
        draw_digit(place, number, false);

	switch (number){
		case 0:
			//draw_thick_line(67, 127, 67, 117, WHITE);
			draw_line(67, 127, 67, 117, WHITE);
			break;
		case 1:
			break;
		case 2:
			break;
		case 3:
			break;
		case 4:
			break;
		case 5:
			break;
		case 6:
			break;
		case 7:
			break;
		case 8:
			break;
		case 9:
			break;
		default:
			break;
	}
}

void draw_score(){

	int number, digit, place;
    bool high = 0;

    if(score > high_score)
        high = 1;

	while (number > 0) {
		digit = number % 10;
    	draw_digit(place, digit, high);
		number /= 10;
		place++;
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
    int collision_type = 0;
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

    return collision_type;
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
    int x0, x1, y0, y1;
    if(LR==0){
        x0 = FLIPPER_L_X;
        y0 = FLIPPER_L_Y;
        x1 = flipper_end_location[0][0];
        y1 = flipper_end_location[0][1];
    }
    else{
        x0 = FLIPPER_R_X;
        y0 = FLIPPER_R_Y;
        x1 = flipper_end_location[1][0];
        y1 = flipper_end_location[1][1];
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
            if(sqrt((ball_x - y)^2 + (ball_y - x)^2) <=  10) return true;
		}
        else {
            if(sqrt((ball_x - x)^2 + (ball_y - y)^2) <=  10) return true;
		}
        error += dy;
        if(error > 0){
            y += y_step;
            error -= dx;
        }
    }
    return false;
}
