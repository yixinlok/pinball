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
#define BLACK 0x0

#define ABS(x) (((x) > 0) ? (x) : -(x))

/* Screen size. */
#define RESOLUTION_X 320
#define RESOLUTION_Y 240

/* Constants for animation */
#define BOX_LEN 2
#define NUM_BOXES 8

#define FALSE 0

#define START 1 
#define FREEPLAY 2 
#define END 3 
int state = START; 
volatile int pixel_buffer_start; // global variable 

bool lose;
int score; 
int high_score = 0 ;

/*
    PHYSICS VARIABLES
*/
// #define LAUNCH_X 170
#define LAUNCH_X 193
#define LAUNCH_Y 30
#define LAUNCH_SPEED 35 // launch speed has to be less than 38/s
int launch_angle; 

double ball_acceleration[2] = {0,5}; // constant acceleration
double ball_velocity[2]; // magnitude must always be less than 60/s  
int ball_location[2] = {LAUNCH_X, LAUNCH_Y}; 

/*
    COLLISION VARIABLES: WALLS FLIPPERS BUMPERS
*/
int collision_type = 0; 
#define WALL_COLLIDE 1
#define FLIPPER_COLLIDE 2
#define BUMPER_1_COLLIDE -1
#define BUMPER_2_COLLIDE -2
#define BUMPER_3_COLLIDE -3
#define BUMPER_4_COLLIDE -4
#define BUMPER_5_COLLIDE -5


#define NUM_SLANTED_WALLS 6
//{x0,y0}, {x1,y1}
// const int slanted_walls[NUM_SLANTED_WALLS][2][2] = {{{130, 215}, {111,196}}, {{210,215}, {229,196}}, {{111,137}, {93,88}},
//                                           {{229,137}, {247,88}}, {{93,66}, {128,9}},  {{247,66}, {212,9}}};
const int slanted_walls[NUM_SLANTED_WALLS][2][2] = {{{153, 215}, {134,196}}, {{233,215}, {252,196}}, {{134,137}, {116,88}},
                                          {{252,137}, {270,88}}, {{116,66}, {151,9}},  {{270,66}, {236,9}}};
//these angles are not relative to the pixel coordinates, they are relative to user
const double slanted_walls_angles[6] = {1.5708, 2.3562, 0.3520, 2.7895, -0.5507,-2.5909}; //in radians

#define roof 16 //y coordinate

#define NUM_VERTICAL_WALLS 4
//{x,y_small, y_big}
// const int vertical_walls[NUM_VERTICAL_WALLS][3] = {{104, 137, 197}, {236, 137, 197}, {86, 65, 88},{86, 65, 88}};
const int vertical_walls[NUM_VERTICAL_WALLS][3] = {{127, 137, 197}, {259, 137, 197}, {109, 65, 88},{277, 65, 88}};

#define FLIPPER_LENGTH 10 //in pixels
// #define FLIPPER_L_X 129
#define FLIPPER_L_X 152
#define FLIPPER_L_Y 219
// #define FLIPPER_R_X 210
#define FLIPPER_R_X 233
#define FLIPPER_R_Y 219
#define DEFAULT_FLIPPER_ANGLE 0.5350 //in radians, equal to 30 degrees
// {x0,y0}, {x1,y1} [left(0)/right(1)][x(0)/y(1)]
#define NUM_FLIPPER_ANGLES 30
int flipper_angle_counter = 0;
double flipper_angles[NUM_FLIPPER_ANGLES] = {0.49933333334,0.46366666668,0.42800000002,0.39233333336,0.3566666667,0.32100000004,0.28533333338,0.24966666672,0.21400000006,0.1783333334,0.14266666674,0.10700000008,0.0713333334199999,0.0356666667599999,9.99998972517347E-11,0.0356666667599999,0.0713333334199999,0.10700000008,0.14266666674,0.1783333334,0.21400000006,0.24966666672,0.28533333338,0.32100000004,0.3566666667,0.39233333336,0.42800000002,0.46366666668,0.49933333334,0.535,};//[L/R][x/y]
//[L/R][x/y]
int flipper_end_location[2][2] = {{0, 0},{0, 0}};
int prev_flipper_end_location[2][2] = {{0, 0},{0, 0}};


#define BUMPER_DIAMETER 26
// const int bumper_1[][]; //(constant) (2d vector with boundary locations, normal angles, scores as rows. columns correspond to each point on the bumper )
// const int bumper_2[][]; //(constant) (2d vector with boundary locations, normal angles, scores)
// const int bumper_3[][]; //(constant) (2d vector with boundary locations, normal angles, scores)
// const int bumper_4[][]; //(constant) (2d vector with boundary locations, normal angles, scores)
// const int bumper_5[][]; //(constant) (2d vector with boundary locations, normal angles, scores)

#define BALL_DIAMETER 13
const uint16_t ball_colours[13][13] = { {65535,65535,65535,65535,50577,50577,50577,50577,50577,65535,65535,65535,65535}, {65535,65535,65535,50577,50577,50577,50577,50577,50577,50577,65535,65535,65535}, {65535,65535,50577,50577,65336,65336,65336,65336,50577,50577,50577,65535,65535}, {65535,50577,50577,65336,65535,65535,65336,50577,50577,50577,50577,35821,65535}, {50577,50577,65336,65535,65535,65336,50577,50577,50577,50577,50577,50577,35821}, {50577,50577,65336,65535,65336,50577,50577,50577,50577,50577,50577,50577,35821}, {50577,50577,65336,65336,50577,50577,50577,50577,50577,50577,50577,35821,35821}, {50577,50577,65336,50577,50577,50577,50577,50577,50577,50577,50577,35821,35821}, {50577,50577,50577,50577,50577,50577,50577,50577,50577,50577,50577,35821,35821}, {65535,50577,50577,50577,50577,50577,50577,50577,50577,50577,35821,35821,65535}, {65535,65535,50577,50577,50577,50577,50577,50577,50577,35821,35821,65535,65535}, {65535,65535,65535,35821,50577,50577,35821,35821,35821,35821,65535,65535,65535}, {65535,65535,65535,65535,35821,35821,35821,35821,35821,65535,65535,65535,65535}, };
//{x,y}
int prev_ball_location[2]; //(2d vector) - for buffers

