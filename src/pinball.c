#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

#include "globals.h"
#include "address_map_arm.h"
// #include "../resources/freeplay_template.h"
// #include "../resources/start_template.h"
// #include "../resources/end_template.h"

int main(void){
    volatile int * pixel_ctrl_ptr = (int *)PIXEL_BUF_CTRL_BASE;
    pixel_buffer_init(pixel_ctrl_ptr);

    while(1){
        if(state == START) 
            start(pixel_ctrl_ptr);
            
        while(state == FREEPLAY)
            freeplay(pixel_ctrl_ptr);
            
        if(state == END) 
            end(pixel_ctrl_ptr);
    }
    return 1;
}

/*
    HARDWARE STUFF
*/
void pixel_buffer_init(volatile int *pixel_ctrl_ptr){
    *(pixel_ctrl_ptr + 1) = FPGA_ONCHIP_BASE; 
    wait_for_vsync();
    
    pixel_buffer_start = *pixel_ctrl_ptr;

    *(pixel_ctrl_ptr + 1) = SDRAM_BASE;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1);
}

void wait_for_vsync(){
    
    volatile int * pixel_ctrl_ptr = (int *)PIXEL_BUF_CTRL_BASE;
    * pixel_ctrl_ptr = 1;
    while(*(pixel_ctrl_ptr + 3)&1);
    
    return;
}

bool check_key_press(){
    
    volatile int * keyboard_ptr = (int *) PS2_BASE; 
	int keyboard_data, RVALID;
    keyboard_data = * keyboard_ptr;
	RVALID = keyboard_data & 0x8000;
	
    if(RVALID) 
        return true;
        
    return false;
}

/*
    GAME STATES
*/
void start(volatile int *pixel_ctrl_ptr){
    // draw start template on both buffers
    for(int i = 0; i < 2; i++){
        draw_start_template();
        wait_for_vsync(); 
        pixel_buffer_start = *(pixel_ctrl_ptr + 1);
    }
    
    // wait to go to next screen
    while(!check_key_press()) 
        {;}

    // draw freeplay template on both buffers
    for(int i = 0; i < 2; i++){
        draw_freeplay_template();
        wait_for_vsync(); 
        pixel_buffer_start = *(pixel_ctrl_ptr + 1); 
    }
    
    initialise();
    
    state = FREEPLAY;
    return;
}

void initialise(){

	srand(time(0));

    launch_angle = rand() % 360;
    launch_angle = -90;
	launch_angle *= DEGREES_TO_RADS;
    score = 0;
    lose =0;
    ball_location[0] = LAUNCH_X;
    ball_location[1] = LAUNCH_Y;
    ball_position[0] = LAUNCH_X;
    ball_position[1] = LAUNCH_Y;
    prev_ball_location[0] = LAUNCH_X;
    prev_ball_location[1] = LAUNCH_Y;

    update_flipper_end_location(DEFAULT_FLIPPER_ANGLE);
	prev_flipper_end_location[0][0] = flipper_end_location[0][0]; 
    prev_flipper_end_location[0][1] = flipper_end_location[0][1];
	prev_flipper_end_location[1][0] = flipper_end_location[1][0]; 
    prev_flipper_end_location[1][1] = flipper_end_location[1][1];

    ball_velocity[0] = LAUNCH_SPEED * cos(launch_angle);
    ball_velocity[1] = LAUNCH_SPEED * sin(launch_angle);

    return;
}

void freeplay(volatile int *pixel_ctrl_ptr){

    collision_type = check_collision(ball_location[0], ball_location[1]);
    erase();
    update_prev();
    update();
    draw(pixel_ctrl_ptr);
    
    if(ball_location[1] >= 239)
            lose=1;
    if(lose) 
        state = END;   
}

void end(volatile int *pixel_ctrl_ptr){
    
    for(int i = 0; i < 2; i++){
        draw_end_template();
        draw_score();
        wait_for_vsync(); 
        pixel_buffer_start = *(pixel_ctrl_ptr + 1);
    }

    while(!check_key_press()) 
        {;}

    state = START;
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

void update_prev(){
    // flippers
    prev_flipper_end_location[0][0] = flipper_end_location[0][0]; 
    prev_flipper_end_location[0][1] = flipper_end_location[0][1]; 
    prev_flipper_end_location[1][0] = flipper_end_location[1][0]; 
    prev_flipper_end_location[1][1] = flipper_end_location[1][1]; 
    // ball
    prev_ball_location[0] = ball_location[0]; 
    prev_ball_location[1] = ball_location[1]; 
}

void update_flippers(){
    if(check_key_press()) flipper_angle_counter = NUM_FLIPPER_ANGLES-1;
    if(flipper_angle_counter >= 0 ) animate_flippers();
}

// if flipper countdown >= 0, this function is called
void animate_flippers(){
    update_flipper_end_location(flipper_angles[flipper_angle_counter]);
    flipper_angle_counter -= 1;
}

void update_flipper_end_location(double angle){
    
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
    
    ball_position[0] += ball_velocity[0] / 60;
    ball_position[1] += ball_velocity[1] / 60;

    ball_location[0] = round(ball_position[0]);
    ball_location[1] = round(ball_position[1]);
    
}

void update_ball_velocity(){
    
    double v_angle, v_mag, f_angle;
    int x, y;

    switch(collision_type){

        case 0: // if no collision
            ball_velocity[1] += ball_acceleration[1] / 60;
            break;

        case ROOF_COLLIDE:
			if(ball_velocity[1]<0)
				ball_velocity[1] = -ball_velocity[1];
			ball_velocity[1] += ball_acceleration[1] / 60;
			break;

		case VARTICAL_WALL_COLLIDE:
			//checking top left wall
			if(ball_location[0]<=vertical_walls[2][0])
				if(ball_velocity[0]<0)
					ball_velocity[0] = -ball_velocity[0];
			//checking bottom left wall
			if(ball_location[0]<=vertical_walls[0][0])
				if(ball_velocity[0]<0)
					ball_velocity[0] = -ball_velocity[0];
			//checking bottom right wall
			if(ball_location[0]>=vertical_walls[1][0])
				if(ball_velocity[0]>0)
					ball_velocity[0] = -ball_velocity[0];
			//checking top right wall
			if(ball_location[0]>=vertical_walls[3][0])
				if(ball_velocity[0]>0)
					ball_velocity[0] = -ball_velocity[0];
			break;
			
        case BUMPER_1_COLLIDE: // top left bumper, goes left to right, top to bottom
        
            x = bumper_centres[0][0];
            y = bumper_centres[0][1]; 
            f_angle = atan2((ball_location[1] - y), (ball_location[0] - x));
			
            v_mag = 2*(ball_velocity[0]*cos(f_angle) + ball_velocity[1]*sin(f_angle));
            
            ball_velocity[0] = ball_velocity[0] - v_mag*cos(f_angle); 
            ball_velocity[1] = ball_velocity[1] - v_mag*sin(f_angle);
            
            break;
            
        case BUMPER_2_COLLIDE:
        
            x = bumper_centres[1][0];
            y = bumper_centres[1][1]; 
            f_angle = atan2((ball_location[1] - y), (ball_location[0] - x));

            v_mag = 2*(ball_velocity[0]*cos(f_angle) + ball_velocity[1]*sin(f_angle));
            
            ball_velocity[0] = ball_velocity[0] - v_mag*cos(f_angle); 
            ball_velocity[1] = ball_velocity[1] - v_mag*sin(f_angle); 
            
            break;
            
        case BUMPER_3_COLLIDE:
        
            x = bumper_centres[2][0];
            y = bumper_centres[2][1]; 
            f_angle = atan2((ball_location[1] - y), (ball_location[0] - x));

            v_mag = 2*(ball_velocity[0]*cos(f_angle) + ball_velocity[1]*sin(f_angle));
            
            ball_velocity[0] = ball_velocity[0] - v_mag*cos(f_angle); 
            ball_velocity[1] = ball_velocity[1] - v_mag*sin(f_angle);
            
            break;
            
        case BUMPER_4_COLLIDE:
        
            x = bumper_centres[3][0];
            y = bumper_centres[3][1]; 
            f_angle = atan2((ball_location[1] - y), (ball_location[0] - x));

            v_mag = 2*(ball_velocity[0]*cos(f_angle) + ball_velocity[1]*sin(f_angle));
            
            ball_velocity[0] = ball_velocity[0] - v_mag*cos(f_angle); 
            ball_velocity[1] = ball_velocity[1] - v_mag*sin(f_angle); 
            
            break;
            
        case BUMPER_5_COLLIDE:
        
            x = bumper_centres[4][0];
            y = bumper_centres[4][1]; 
            f_angle = atan2((ball_location[1] - y), (ball_location[0] - x));

            v_mag = 2*(ball_velocity[0]*cos(f_angle) + ball_velocity[1]*sin(f_angle));
            
            ball_velocity[0] = ball_velocity[0] - v_mag*cos(f_angle); 
            ball_velocity[1] = ball_velocity[1] - v_mag*sin(f_angle); 
            
            break;
            
        case FLIPPER_COLLIDE:
        
            if(flipper_angle_counter == -1)
                f_angle = 180*DEGREES_TO_RADS - DEFAULT_FLIPPER_ANGLE;
            else
                f_angle = flipper_angles[NUM_FLIPPER_ANGLES+flipper_angle_counter];

            v_mag = 2*(ball_velocity[0]*cos(f_angle) + ball_velocity[1]*sin(f_angle));
            ball_velocity[0] = ball_velocity[0] - v_mag*cos(f_angle); 
            ball_velocity[1] = - (ball_velocity[1] - v_mag*sin(f_angle)); 
            
            break;
            
        default:
        
            f_angle = 180*DEGREES_TO_RADS - slanted_angles[collision_type-1];
            v_mag = 2*(ball_velocity[0]*cos(f_angle) + ball_velocity[1]*sin(f_angle));
            
            ball_velocity[0] = ball_velocity[0] - v_mag*cos(f_angle); 
            ball_velocity[1] = -(ball_velocity[1] - v_mag*sin(f_angle)); 
            
            break;
    }

    return;
}

void update_score(){

    switch(collision_type){
        case BUMPER_1_COLLIDE:
            score += 50;
            break;
        case BUMPER_2_COLLIDE:
            score += 100;
            break;
        case BUMPER_3_COLLIDE:
            score += 50;
            break;
        case BUMPER_4_COLLIDE:
            score += 10;
            break;
        case BUMPER_5_COLLIDE:
            score += 10;
            break;
        default:
            break;
    }

    if(score>high_score)
        high_score = score;

    return;
}
/* 
    DRAW AND ERASE FUNCTIONS
*/
void erase(){
    erase_flippers();
    erase_ball(prev_ball_location[0], prev_ball_location[1]);
}

void draw(volatile int *pixel_ctrl_ptr){
    draw_flippers();
    draw_ball(ball_location[0], ball_location[1]);
    draw_score();
	
    wait_for_vsync(); // swap front and back buffers on VGA vertical sync
    pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer
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
            if(x+i <RESOLUTION_X && y+j <RESOLUTION_Y) plot_pixel(x+i, y+j, freeplay_template[y+j][x+i]);
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
            if(x+i <RESOLUTION_X && y+j <RESOLUTION_Y) plot_pixel(x+i, y+j, freeplay_template[y+j][x+i]);
        }
    }
}

void draw_start_template(){
    for(int i = 0; i < 320; i++){
        for(int j = 0; j < 240; j++){
            plot_pixel(i, j, start_template[j][i]);
        }
    }
}

void draw_freeplay_template(){
    for(int i = 0; i < 320; i++){
        for(int j = 0; j < 240; j++){
            plot_pixel(i, j, freeplay_template[j][i]);
        }
    }
}

void draw_end_template(){
    for(int i = 0; i < 320; i++){
        for(int j = 0; j < 240; j++){
            plot_pixel(i, j, end_template[j][i]);
        }
    }
}

void draw_digit(int place, int number, bool high){
    // if high, offset y by certain amount
    // if place is 1 or 2, offset x by a certain amount (in negative direction)
    // smth like x0 - place * 10 and y0 + high * 30
    // void plot_pixel(int x, int y, short int line_color)
    int x, y, i, j;
    
    // lose
    if(lose){
        x=135; y=102;
        for(i=x; i < x+9; i++){
            for(j=y; j < y+12; j++){
                plot_pixel(i-place*9, j+high*23, numbers[j-y][i-x+number*9]);
            }
        }
        return;
    }
    
    // freeplay
    x=72; y=119;
    for(i=x; i < x+9; i++){
        for(j=y; j < y+12; j++){
            plot_pixel(i-place*9, j+high*41, numbers[j-y][i-x+number*9]);
        }
    }
}


void draw_score(){


    bool high = 0;
    int number, digit, place;
    number = score;
    place = 0;

    while (number > 0) {
        digit = number % 10;
        draw_digit(place, digit, 0);
        number /= 10;
        place++;
    }
    
    number = high_score;
    place = 0;
    while (number > 0) {
        digit = number % 10;
        draw_digit(place, digit, 1);
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
			draw_box(y,x,colour);
		}
        else {
			draw_box(x,y,colour);
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
    if(ball_y<= roof) return ROOF_COLLIDE;
	
    for(int i = 0; i<NUM_VERTICAL_WALLS; i++){
        if(ball_x <= vertical_walls[i][0]+3 && ball_x >= vertical_walls[i][0]-3){
            if(ball_y > vertical_walls[i][1] && ball_y < vertical_walls[i][2]) 
				return VARTICAL_WALL_COLLIDE;
        }
    }

    //check flippers
    if(check_flipper_collide(0, ball_x,ball_y)) return FLIPPER_COLLIDE; //left
    if(check_flipper_collide(1, ball_x,ball_y)) return FLIPPER_COLLIDE; //right

		
	for(int i = 0; i < 5; i++){
    	if(check_bumper_collide(i, ball_x,ball_y)) 
			return (-i-1);
	}
    //check slanted walls
    for(int i = 0; i<NUM_SLANTED_WALLS; i++){
        if(check_slanted_wall_collide(i,ball_x,ball_y)){
			switch(i){
				case 0:
					return WALL_0_COLLIDE;
					break;
				case 1: 
					return WALL_1_COLLIDE;
					break;
				case 2:  
					return WALL_2_COLLIDE;
					break;
				case 3:  
					return WALL_3_COLLIDE;
					break;
				case 4:  
					return WALL_4_COLLIDE;
					break;
				case 5:  
					return WALL_5_COLLIDE;
					break;
				default:
					break;
			}
		}
    }

    return collision_type;
}

bool check_bumper_collide(int bumper_id, int ball_x, int ball_y){
    
    int x = bumper_centres[bumper_id][0];
    int y = bumper_centres[bumper_id][1];
    
    double radius = sqrt(pow((x - ball_x),2) + pow((y - ball_y), 2));

    if(radius <= (BUMPER_DIAMETER + BALL_DIAMETER + 4) /2) 
        return true;
    
    return false;
}

//tested
bool check_slanted_wall_collide(int wall_id, int ball_x, int ball_y){
    int x0 = slanted_walls[wall_id][0][0];
    int y0 = slanted_walls[wall_id][0][1];
    int x1 = slanted_walls[wall_id][1][0];
    int y1 = slanted_walls[wall_id][1][1];

    bool is_steep = abs(y1-y0) > abs(x1-x0);
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
            //if(ball_x == y && ball_y == x) return true;
			double a = sqrt(pow((ball_x - y),2) + pow((ball_y - x),2));
			if(a <=  2) {return true;}
		}
        else {
            //if(ball_x == x && ball_y == y) return true;
			double a = sqrt(pow((ball_x - x),2) + pow((ball_y - y),2));
			if(a <=  2) {return true;}
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
            double a = sqrt(pow((ball_x - y),2) + pow((ball_y - x),2));
			if(a <=  10) {return true;}
		}
        else {
			double a = sqrt(pow((ball_x - x),2) + pow((ball_y - y),2));
            if(a <=  10) {return true;}
		}
        error += dy;
        if(error > 0){
            y += y_step;
            error -= dx;
        }
    }
    return false;
}
