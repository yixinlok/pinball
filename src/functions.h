/*
    HARDWARE STUFF
*/
void pixel_buffer_init(int *pixel_ctrl_ptr);

/*
    GAME STATES
*/


/*
    KINEMATICS FUNCTIONS
*/
void update_flipper_end_location(double angle); 

/* 
    DRAWING FUNCTIONS
*/
void plot_pixel(int x, int y, short int line_color);
void draw_start_template( );
void draw_freeplay_template( );
void draw_ball(int x, int y);
void erase_ball(int x, int y);
void draw_flippers(int angle);
void erase_flippers(int angle);
void draw_thick_line(int x0, int y0, int x1, int y1);
void draw_box(int x, int y, short int colour);

/*
    COLLISION CHECKING FUNCTIONS
*/
int check_collision(int ball_x, int ball_y);
bool check_slanted_wall_collide(int wall_id, int ball_x, int ball_y);
bool check_flipper_collide(int LR, int ball_x, int ball_y);
/*
    MISCELLANEOUS + HELPER FUNCTIONS
*/
void swap(int* p1, int* p2);
