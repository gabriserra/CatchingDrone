#include "userpanel.h"
#include <allegro.h>
#include <float.h>
#include <string.h>
#include <math.h>

//-----------------------------------
// PRIVATE: KEYBOARD RELATED FUNCTIONS
//-----------------------------------

// ---
// Waits for a key pressed and extracts the corresponding scan code
// return: char - scancode of pressed key
// ---
static char get_scancode() {
	int 	key;		// key code
	
	key = readkey(); 	// block until a key is pressed
	return key >> 8;	// get the scancode
}

// ---
// Returns the scancode of a pressed key or 0 if no key was pressed
// return: char - scancode of pressed key or 0 if no key was pressed
// ---
static char nb_get_scancode() {
	if (keypressed())
		return readkey() >> 8; //get the scancode
	else 
		return 0;
}

// ---
// Blocking: Return only after BACKSPACE key press
// return: void
// ---
static void wait_back_key() {
	char 	scan; // scancode of key pressed

	while(scan != KEY_BACKSPACE)
		scan = get_scancode();
}

// ---
// Non blocking: Return 1 if BACKSPACE key is pressed, 0 otherwise
// return: int - 1 if BACKSPACE, 0 otherwise
// ---
static int nb_get_back_key() {
	return key[KEY_BACKSPACE] != 0;
}

// ---
// Blocking: Return only after ENTER key press
// return: void
// ---
static void wait_enter_key() {
	char 	scan; // scancode of key pressed
	
	while(scan != KEY_ENTER)
		scan = get_scancode();
}

// ---
// Non blocking: Return 1 if ENTER key is pressed, 0 otherwise
// return: int - 1 if ENTER, 0 otherwise
// ---
static int nb_get_enter_key() {
	return key[KEY_ENTER] != 0;
}

// ---
// Non blocking: Return 1 if R key is pressed, 0 otherwise
// return: int - 1 if ENTER, 0 otherwise
// ---
static int nb_get_r_key() {
	return key[KEY_R] != 0;
}

//--------------------------------
// PRIVATE: MOUSE RELATED FUNCTIONS
//--------------------------------

// ---
// Return 1 if mouse cursor is placed inside coord (x1, y1, x1, y2) provided
// float x1: top left angle x coordinate
// float y1: top left angle y coordinate
// float x2: bottom right angle x coordinate
// float y2: bottom right angle y coordinate
// return: int - 1 if mouse is placed inside the rectangle, 0 otherwise
// ---
static int is_mouse_in(float x1, float y1, float x2, float y2) {
	if(mouse_x > x1 && mouse_x < x2)
		if(mouse_y > y1 && mouse_y < y2)
			return 1;	
	return 0;
}

// ---
// Return 1 if mouse cursor is placed inside ball with radius r
// float* pos: pointer to Vector[3] that contains ball position
// float r: radius of the ball
// return: int - 1 if mouse is placed inside the ball, 0 otherwise
// ---
static int is_mouse_in_ball(float* pos, float r) {
	return is_mouse_in(pos[X]-r, pos[Y]-r, pos[X]+r, pos[Y]+r);
}

// ---
// Return 1 if mouse cursor is placed inside map square box
// return: int - 1 if mouse is placed inside map square box, 0 otherwise
// ---
static int is_mouse_in_map() {
	return is_mouse_in(MAPSQX1, MAPSQY2, MAPSQX2, MAPSQY1);
}

// ---
// Return 1 if mouse cursor is placed inside throw square box
// return: int - 1 if mouse is placed inside throw square box, 0 otherwise
// ---
static int is_mouse_in_thbox() {
	return 
		is_mouse_in(DIRLINEX, PWRLINEY, DIRLINEX+DIRLINEW, PWRLINEY+PWRLINEH);
}

// ---
// Return 1 if the left button of mouse is pressed
// return: int - 1 if mouse left button is pressed, 0 otherwise
// ---
static int get_mouse_left_click() {
	return mouse_b & 1;
}

// ---
// Return 1 if the right button of mouse is pressed
// return: int - 1 if mouse right button is pressed, 0 otherwise
// ---
static int get_mouse_right_click() {
	return mouse_b & 2;
}

//----------------------------
// PUBLIC: KEYBOARD RELATED FUNCTIONS
//----------------------------

// ---
// Blocking: Return only after ESC key press
// return: void
// ---
void wait_esc_key() {
	char 	scan; // scancode of key pressed

	while(scan != KEY_ESC)
		scan = get_scancode();
}

// ---
// Non blocking: Return 1 if ESC key is pressed, 0 otherwise
// return: int - 1 if ESC, 0 otherwise
// ---
int nb_get_esc_key() {
	return key[KEY_ESC] != 0;
}

//----------------------------------------------------
// PRIVATE: UTILITIES AND COORDINATES RELATED FUNCTIONS
//----------------------------------------------------

// ---
// Map a value (in) from a range (in_i - in_f) to another (out_i - out_f)
// float in: value in input range to be mapped in output range
// float in_i: input range start
// float in_f: input range end
// float out_i: output range start
// float out_f: output range end
// return: float - value mapped in output range
// ---
static float r_map(
		float in, float in_i, float in_f, float out_i, float out_f) {
	
	float 	slope;	// slope of range to be mapped
	float 	out;	// value mapped in output range

	// avoid error caused by erroneous input
	if(in < in_i || in > in_f)
		in = in_i + ((in_f - in_i) / 2);

	// avoid divide by 0 exception
	if(in_f - in_i == 0)
		slope = FLT_MAX;
	else
		slope = (out_f - out_i) / (in_f - in_i);
	
	out = out_i + slope * (in - in_i);
	return out;
}

// ---
// Convert user panel map coordinates to real world coordinates
// float* old_coord: pointer to Vector[2] that contains map coord
// float* new_coord: pointer to Vector[2] will contains real world coord
// return: void
// ---
static void coord_map_to_real(float* old_coord, float* new_coord) {
	new_coord[X] = r_map(old_coord[X], MAPSQX1, MAPSQX2, WRL_I, WRL_F);
	new_coord[Y] = r_map(old_coord[Y], MAPSQY2, MAPSQY1, WRL_I, WRL_F);
}

// ---
// Convert real world coordinates to user panel map coordinates
// float* old_coord: pointer to Vector[2] that contains real world coord
// float* new_coord: pointer to Vector[2] will contains map coord
// return: void
// ---
static void coord_real_to_map(float* old_coord, float* new_coord) {
	new_coord[X] = r_map(old_coord[X], WRL_I, WRL_F, MAPSQX1, MAPSQX2);
	new_coord[Y] = r_map(old_coord[Y], WRL_I, WRL_F, MAPSQY2, MAPSQY1);
}

// ---
// Calculate power of ball throw starting from bound and current value
// float lw_bound: lower possible power
// float up_bound: maximum possible power
// float current: current power value
// return: float - power value mapped from 0 to NUMBARBLOCK
// ---
static float calc_power(float lw_bound, float up_bound, float current) {
	if(current > up_bound || current < lw_bound)
		return 0;

	return r_map(current, lw_bound, up_bound, 0, NUMBARBLOCK);
}

// ---
// Calculate dir of ball launch starting from bound and current value
// float lw_bound: lower possible direction (left)
// float up_bound: maximum possible direction (right)
// float current: current direction value
// return: float - direction value mapped from 0 to NUMBARBLOCK
// ---
static float calc_dir(float lw_bound, float up_bound, float current) {
	float 	bound = NUMBARBLOCK / 2;	// absolute min/max value output

	if(current > up_bound || current < lw_bound)
		return 0;

	return r_map(current, lw_bound, up_bound, -bound, bound);
}

// ---
// Position the object (ball or drone) in map box at mouse current coord
// float* pos: pointer to Vector[2] that will contains mouse act coordinate
// float* positioned: pointer to boolean flag that indicates obj has positioned
// return: void
// ---
static void set_obj_position(float* pos, int* positioned) {
	pos[X] = mouse_x;
	pos[Y] = mouse_y;
	*positioned = 1;
}

//-----------------------------------------------------
// PRIVATE: DRAW ON SCREEN AND BITMAPS RELATED FUNCTIONS
//-----------------------------------------------------

// ---
// Draw the top box container on buffer buff
// BITMAP* buff: pointer to BITMAP buffer to be drew
// return: void
// ---
static void draw_top_box(BITMAP* buff) {
	rect(buff, TPBOXX1, TPBOXY1, TPBOXX2, TPBOXY2, MCOL);
}

// ---
// Draw the map box container on buffer buff
// BITMAP* buff: pointer to BITMAP buffer to be drew
// return: void
// ---
static void draw_map_box(BITMAP* buff) {
	rect(buff, MAPSQX1, MAPSQY1, MAPSQX2, MAPSQY2, MCOL);
}

// ---
// Draw the power box container on buffer buff
// BITMAP* buff: pointer to BITMAP buffer to be drew
// return: void
// ---
static void draw_pwr_box(BITMAP* buff) {
	rect(buff, PWRSQX1, PWRSQY1, PWRSQX2, PWRSQY2, MCOL);
}

// ---
// Draw top box static string on buffer buff
// BITMAP* buff: pointer to BITMAP buffer to be drew
// return: void
// ---
static void draw_text(BITMAP* buff) {
	textout_ex(buff, font, WNDCAPT, RXMARG, VARMARG(1), MCOL, -1);
	textout_ex(buff, font, MCLKCMD(LEFT, DRONE), RXMARG, VARMARG(2), MCOL, -1);
	textout_ex(buff, font, MCLKCMD(RIGHT, BALL), RXMARG, VARMARG(3), MCOL, -1);
	textout_ex(buff, font, LNCCMD, RXMARG, VARMARG(4), MCOL, -1);
	textout_ex(buff, font, STARTCMD, RXMARG, VARMARG(5), MCOL, -1);
	textout_ex(buff, font, PAUSECMD, RXMARG, VARMARG(6), MCOL, -1);
	textout_ex(buff, font, ENDCMD, RXMARG, VARMARG(7), MCOL, -1);
	textout_ex(buff, font, RESETCMD, RXMARG, VARMARG(8), MCOL, -1);
}

// ---
// Draw the power bar container and text on buffer buff
// BITMAP* buff: pointer to BITMAP buffer to be drew
// return: void
// ---
static void draw_bar(BITMAP* buff) {
	int 	mid_x;	// half LNCBAR x
	mid_x = LNCBARX1 + ((LNCBARX2 - LNCBARX1) / 2);

	rect(buff, LNCBARX1, LNCBARY1, LNCBARX2, LNCBARY2, MCOL);
	textout_centre_ex(buff, font, BARCAPT, mid_x, LNCBARY2-STM, MCOL, -1);
}

// ---
// Draw the power box boll on buffer buff at coord x, y
// BITMAP* buff: pointer to BITMAP buffer to be drew
// float x: x-position of centre of the ball
// float y: y-position of centre of the ball
// return: void
// ---
static void draw_ball(BITMAP* buff, float x, float y) {
	circlefill(buff, x, y, BALLR, MCOL);
}

// ---
// Draw power lines legend and caption on buffer buff
// BITMAP* buff: pointer to BITMAP buffer to be drew
// return: void
// ---
static void draw_legend(BITMAP* buff) {
	int 	mid_x;	// half DIRLINE x
	mid_x = DIRLINEX + (DIRLINEW / 2);

	line(buff, PWRLINEX, PWRLINEY+PWRLINEH, PWRLINEX, PWRLINEY, MCOL);
	textout_centre_ex(buff, font, PWRLINCPT, PWRLINEX, PWRLINEY-STM, MCOL, -1);
	line(buff, DIRLINEX, DIRLINEY, DIRLINEX+DIRLINEW, DIRLINEY, MCOL);
	textout_centre_ex(buff, font, DIRLINCPT, mid_x, DIRLINEY+PAD, MCOL, -1);
}

// ---
// Fill and draw the power bar respect to current power
// BITMAP* buff: pointer to BITMAP buffer to be drew
// float power: power from 1 to NUMBARBLOCK
// return: void
// ---
static void draw_filled_bar(BITMAP* buff, float power) {
	int 	i;
	float 	color;									// color of block of bar
	float	block_x1, block_y1, block_x2, block_y2; // coord of block of bar
	
	// avoid power out of range
	if(power <= 0 || power > NUMBARBLOCK)
		return;

	// cal fixed x1, x2 coordinate of each block
	block_x1 = LNCBARX1 + PAD;
	block_x2 = LNCBARX2 - PAD;

	// draw one block every integer unit of power
	for(i = 0; i <= (int)power; i++) {
		if(i < PWRLOWBOUND)
			color = LPBARCOL;
		else if(i < PWRMIDBOUND)
			color = MPBARCOL;
		else
			color = HPBARCOL;
			
		block_y1 = LNCBARY1 - PAD - (i * BARHBK);
		block_y2 = LNCBARY1 - PAD - ((i+1) * BARHBK);

		rectfill(buff, block_x1, block_y1, block_x2, block_y2, color);
	}	
}

// ---
// Draw the simulation state text on buffer buff
// BITMAP* buff: pointer to BITMAP buffer to be drew
// int state: state of simulation to be displayed
// return: void
// ---
static void draw_simulat_text(BITMAP* buff, int state) {
	if(state == RUNNING)
		textout_ex(buff, font, STCAPT(RUNNING!), RXMARG, VARMARG(9), MCOL, -1);
	else if(state == STOPPED)
		textout_ex(buff, font, STCAPT(STOPPED!), RXMARG, VARMARG(9), MCOL, -1);
	else if(state == PAUSED)
		textout_ex(buff, font, STCAPT(PAUSED!), RXMARG, VARMARG(9), MCOL, -1);
}

// ---
// Draw object of map box on buff at obj_pos with radius r and color col
// BITMAP* buff: pointer to BITMAP buffer to be drew
// float* obj_pos: centre of circle that indicates obj (map coord)
// int r: radius of the cirle
// int col: color of the cirle
// return: void
// ---
static void draw_obj_on_map(BITMAP* buff, float* obj_pos, int r, int col) {
	circlefill(buff, obj_pos[X], obj_pos[Y], r, col);
}

// ---
// Create and initialize a bitmap buffer
// int dim_x: width of the bitmap buffer to be created
// int dim_y: height of the bitmap buffer to be created
// int color: color of the bitmap buffer to be created
// return: BITMAP* - pointer to bitmap buffer created
// ---
static BITMAP* create_bitmap_buff(int dim_x, int dim_y, int color) {
	BITMAP* buff = create_bitmap(dim_x, dim_y);
	clear_bitmap(buff);
	clear_to_color(buff, color);
	return buff;
}

// ---
// Transfer a bitmap on screen safely and destroy the buffer
// BITMAP* buff: pointer to BITMAP buffer that has to be copied
// int xs: top left x-coord of rect in buffer
// int xy: top left y-coord of rect in buffer
// int xd: top left x-coord of rect in screen
// int yd: top left y-coord of rect in screen
// int w: width of the rect that has to be copied
// int h: height of the rect that has to be copied
// return: void
// ---
static void bmp_2_screen(
		BITMAP* buff, int xs, int ys, int xd, int yd, int w, int h) {
	
	scare_mouse();
	blit(buff, screen, xs, ys, xd, yd, w, h);	
	unscare_mouse();
	destroy_bitmap(buff);
}

//-------------------------------------
// PRIVATE: UPDATE BOX RELATED FUNCTIONS
//-------------------------------------

// ---
// Update the map box with new drone and ball coord
// pstate* panel: pointer to panel state structure
// float* d_pos: pointer to Vector[3] that contains real world drone position
// float* b_pos: pointer to Vector[3] that contains real world ball position
// return: void
// ---
static void update_map_box(struct pstate* panel, float* d_pos, float* b_pos) {
	BITMAP* buff; // buffer to be printed
	buff = create_bitmap_buff(XWIN, YWIN, MAPBKG);

	// if simul is running we can accept new pos from extern
	if(panel->simul_state == RUNNING)
		set_map_pos(panel, d_pos, b_pos);

	// if drone is positioned we draw on map, else we can choose its pos
	if(panel->drone_positioned)
		draw_obj_on_map(buff, panel->drone_pos, MAPDRONER, MAPDRONECOL);
	else if(is_mouse_in_map() && get_mouse_left_click())
		set_obj_position(panel->drone_pos, &panel->drone_positioned);

	// if ball is positioned we draw on map, else we can choose its pos
	if(panel->ball_positioned)
		draw_obj_on_map(buff, panel->ball_pos, MAPBALLR, MAPBALLCOL);
	else if(is_mouse_in_map() && get_mouse_right_click())
		set_obj_position(panel->ball_pos, &panel->ball_positioned);

	draw_map_box(buff);
	bmp_2_screen(buff, MAPSQX1, MAPSQY2, MAPSQX1, MAPSQY2, SQLBOX+1, SQHBOX+1);	
}

// ---
// Update the launch bar box
// pstate* panel: pointer to panel state structure
// return: void
// ---
static void update_launch_box(struct pstate* panel) {
	BITMAP* buff;	// buffer to be printed	
	int 	drag;	// indicates if ball is draggable

	buff = create_bitmap_buff(XWIN, YWIN, BKG);
	drag = is_mouse_in_thbox(); //&& is_mouse_in_ball(panel->launch_ball, BALLR);

	// if ball is draggable and user click with left mouse button
	if(drag && get_mouse_left_click()) {
		// ball pos, power and dir are updated
		panel->launch_ball[X] = mouse_x;
		panel->launch_ball[Y] = mouse_y;
		panel->power = calc_power(LNCBARY2, LNCBARY1, mouse_y);
		panel->dir = calc_dir(DIRLINEX, DIRLINEX+DIRLINEW, mouse_x);
	}

	// draw everything
	draw_ball(buff, panel->launch_ball[X], panel->launch_ball[Y]);
	draw_bar(buff);
	draw_legend(buff);
	draw_filled_bar(buff, panel->power);
	draw_pwr_box(buff);
	bmp_2_screen(buff, PWRSQX1, PWRSQY2, PWRSQX1, PWRSQY2, SQLBOX+1, SQHBOX+1);	
}

// ---
// Update the panel text box
// pstate* panel: pointer to panel state structure
// return: void
// ---
static void update_panel_box(struct pstate* panel) {
	BITMAP*	buff; // buffer to be printed
	
	buff = create_bitmap_buff(XWIN, YWIN, BKG);	
	draw_text(buff);
	draw_simulat_text(buff, panel->simul_state);
	draw_top_box(buff);		
	bmp_2_screen(buff, TPBOXX1, TPBOXY2, TPBOXX1, TPBOXY2, PNLBOX+1, PNHBOX+1);
}

//---------------------------------
// PRIVATE: EVENT RELEATED FUNCTIONS
//---------------------------------

// ---
// Check for change of state simulation
// pstate* panel: pointer to panel state structure
// return: void
// ---
static void change_state(struct pstate* panel) {
	if(nb_get_r_key())
		p_reset(panel);
	else if(nb_get_enter_key() && are_obj_posit(panel))
		panel->simul_state = RUNNING;
	else if(nb_get_back_key() && panel->simul_state == RUNNING)
		panel->simul_state = PAUSED;
}

//----------------------------
// PUBLIC: INIT AND EXIT ALLEGRO LIB
//----------------------------

// ---
// Start to draw user panel on screen
// return: void
// ---
void init_panel() {
	allegro_init(); 
	install_keyboard();
	install_mouse();
	enable_hardware_cursor();
	set_color_depth(VGA);
	set_gfx_mode(GFX_AUTODETECT_WINDOWED, XWIN, YWIN, 0, 0); 
	clear_to_color(screen, BKG);
	set_window_title(WNDTITLE);
	show_mouse(screen);
}

// ---
// Exit and close the user panel
// return: void
// ---
void exit_panel() {
	allegro_exit();
}

// ---
// Main graphic loop
// return: void
// ---
void graphic_loop(struct pstate* panel, float* d_new_pos, float* b_new_pos) {
	// check for event that change internal state
	change_state(panel);

	// update the three boxes
	update_map_box(panel, d_new_pos, b_new_pos);
	update_launch_box(panel);
	update_panel_box(panel);
}

//----------------------------
// PUBLIC: GETTER AND SETTER
//----------------------------

// ---
// Re-set panel struct to initial state
// pstate* panel: pointer to panel state structure
// return: void
// ---
void p_reset(struct pstate* panel) {
	memset(panel, 0, sizeof(struct pstate));
	launch_ball_init(panel);
	map_drone_init(panel);
}

// ---
// Set the launch box ball at initial defaul coordinates
// pstate* panel: pointer to panel state structure
// return: void
// ---
void launch_ball_init(struct pstate* panel) {
	panel->launch_ball[X] = BALLX;
	panel->launch_ball[Y] = BALLY;
}

// ---
// Set the drone initial Z-position in map
// pstate* panel: pointer to panel state structure
// return: void
// ---
void map_drone_init(struct pstate* panel) {
	panel->drone_pos[Z] = DINITH;
}

// ---
// Set simulation state
// pstate* panel: pointer to panel state structure
// int state: possible panel state (STOPPED RUNNING CATCHED PAUSED)
// return: void
// ---
void set_simul_state(struct pstate* panel, int state) {
	panel->simul_state = state;
}

// ---
// Get simulation state
// pstate* panel: pointer to panel state structure
// return: void
// ---
int get_simul_state(struct pstate* panel) {
	return panel->simul_state;
}

// ---
// Set drone and ball map position from real coord
// pstate* panel: pointer to panel state structure
// float* d_real_pos: pointer to Vector[3] that contains real world drone pos
// float* b_real_pos: pointer to Vector[3] that contains real world ball pos
// return: void
// ---
void set_map_pos(struct pstate* panel, float* d_real_pos, float* b_real_pos) {
	// Get and convert real world coordinate to map coordinates
	coord_real_to_map(d_real_pos, panel->drone_pos);
	coord_real_to_map(b_real_pos, panel->ball_pos);
	panel->drone_pos[Z] = d_real_pos[Z];
	panel->ball_pos[Z] = b_real_pos[Z];	
}

// ---
// Return 1 if drone OR ball are positioned, 0 otherwise
// pstate* panel: pointer to panel state structure
// return: int - 1 if drone OR ball are positioned, 0 in other cases
// ---
int is_obj_posit(struct pstate* panel) {
	if(panel->drone_positioned || panel->ball_positioned)
		return 1;
	return 0;
}

// ---
// Return 1 if drone AND ball are positioned, 0 otherwise
// pstate* panel: pointer to panel state structure
// return: int - 1 if drone OR ball are positioned, 0 in other cases
// ---
int are_obj_posit(struct pstate* panel) {
	if(panel->drone_positioned && panel->ball_positioned)
		return 1;
	return 0;
}

// ---
// Get real world coordinates and put in provided float*
// pstate* panel: pointer to panel state structure
// float* d_pos: pointer to Vector[3] that will contain real world drone pos
// float* b_pos: pointer to Vector[3] that will contain real world ball pos
// return: void
// ---
void get_real_coord(struct pstate* panel, float* d_pos, float* b_pos) {
	// get and convert map coord to real world coordinates
	coord_map_to_real(panel->drone_pos, d_pos);
	coord_map_to_real(panel->ball_pos, b_pos);
	d_pos[Z] = panel->drone_pos[Z];
	b_pos[Z] = panel->ball_pos[Z];
}

// ---
// Get user panel power setted by user
// pstate* panel: pointer to panel state structure
// return: float - power between 0 and NUMBARBLOCK
// ---
float get_power(struct pstate* panel) {
	return panel->power;
}

// ---
// Get direction of ball setted by user
// pstate* panel: pointer to panel state structure
// return: float - direction between 0 and NUMBARBLOCK
// ---
float get_dir(struct pstate* panel) {
	return panel->dir;
}