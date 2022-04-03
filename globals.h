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

#define LIGHT_BEIGE 0xffe7c7
#define MEDIUM_BEIGE 0xc6b28e
#define DARK_BEIGE 0x8a7c69
// int light_beige_pixels[18][2] = {};
// int dark_beige_pixels[23][2] = {};

#define ABS(x) (((x) > 0) ? (x) : -(x))

/* Screen size. */
#define RESOLUTION_X 320
#define RESOLUTION_Y 240

/* Constants for animation */
#define BOX_LEN 2
#define NUM_BOXES 8

#define FALSE 0

volatile int pixel_buffer_start; // global variable

int score; 
int high_score = 0; 

/*
    PHYSICS VARIABLES
*/
double ball_acceleration[2] = {0,-9};
double ball_velocity[2]; 
int ball_location[2]; 
int time;

#define LAUNCH_X 170
#define LAUNCH_Y 30

/*
    GRAPHICS TEMPLATES
*/
int start_screen[RESOLUTION_X][RESOLUTION_Y]; // 2d vector with pixel colours for each coordinate
int freeplay_screen[RESOLUTION_X][RESOLUTION_Y]; //(constant) // 2d vector with pixel colours for each coordinate. template only, no flipper or balls, only bumpers and walls
int end_screen[RESOLUTION_X][RESOLUTION_Y]; // 2d vector with pixel colours for each coordinate


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
const int slanted_walls[NUM_SLANTED_WALLS][2][2] = {{{130, 215}, {111,196}}, {{210,215}, {229,196}}, {{111,137}, {93,88}},
                                          {{229,137}, {247,88}}, {{93,66}, {128,9}},  {{247,66}, {212,9}}};
const double slanted_walls_angles[6] = {1.5708, 2.3562, 0.3520, 2.7895, -0.5507,-2.5909};

#define roof 16 //y coordinate

#define NUM_VERTICAL_WALLS 4
//{x,y_small, y_big}
const int vertical_walls[NUM_VERTICAL_WALLS][3] = {{104, 137, 197}, {236, 137, 197}, {86, 65, 88},{86, 65, 88}};

#define FLIPPER_LENGTH 10 //in pixels
#define FLIPPER_L_X 129
#define FLIPPER_L_Y 219
#define FLIPPER_R_X 210
#define FLIPPER_R_Y 219
const int flipper_angles[] = {};

// const int bumper_1[][]; //(constant) (2d vector with boundary locations, normal angles, scores as rows. columns correspond to each point on the bumper )
// const int bumper_2[][]; //(constant) (2d vector with boundary locations, normal angles, scores)
// const int bumper_3[][]; //(constant) (2d vector with boundary locations, normal angles, scores)
// const int bumper_4[][]; //(constant) (2d vector with boundary locations, normal angles, scores)
// const int bumper_5[][]; //(constant) (2d vector with boundary locations, normal angles, scores)



// int prev_ball_location[][]; //(2d vector) - for buffers
// int prev_flipper_location[][]; //(2d vector)  - for buffers
// int prev_score[][]; // (2d vector)  - for buffers
