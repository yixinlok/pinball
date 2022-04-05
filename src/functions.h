/*
    HARDWARE STUFF
*/
void pixel_buffer_init(int *pixel_ctrl_ptr);
bool check_key_press();
void wait_for_vsync();

/*
    GAME STATES
*/
void start(int *pixel_ctrl_ptr);
void freeplay(int *pixel_ctrl_ptr);
void end(int *pixel_ctrl_ptr);
void initialise();
/*
    KINEMATICS FUNCTIONS
*/
void update_prev();
void update();
void update_ball_position();
void update_ball_velocity();
void update_score();
void update_flippers();
void animate_flippers();
void update_flipper_end_location(double angle); 

/* 
    DRAWING FUNCTIONS
*/
void draw(int *pixel_ctrl_ptr);
void erase();
void plot_pixel(int x, int y, short int line_color);
void draw_start_template( );
void draw_freeplay_template( );
void draw_end_template( );
void draw_ball(int x, int y);
void erase_ball(int x, int y);
void draw_flippers(int angle);
void erase_flippers(int angle);
void draw_thick_line(int x0, int y0, int x1, int y1);
void draw_box(int x, int y, short int colour);
void draw_digit(int place, int number, bool high);
void draw_score();


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
