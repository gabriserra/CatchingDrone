//-------------------------------------------------------------------------
// USERPANEL_H: CONTAINS SOME THINGS TO DRAW USER PANEL AND SET SIMUL PARAM
//-------------------------------------------------------------------------

#ifndef USERPANEL_H
#define USERPANEL_H

//-----------------------------------------------------
// GRAPHICS CONSTANTS (DIMENSION)
//-----------------------------------------------------
#define XWIN 			800				// window X resolution
#define YWIN 			600				// window Y resolution
#define PNLBOX 			770				// X length of the top menu box
#define PNHBOX 			140				// Y length of the top menu box
#define SQLBOX 			370				// X length of the square boxex
#define SQHBOX 			410				// Y length of the square boxes
#define PWRLINEH 		310				// Y lenght of power line
#define DIRLINEW 		150				// X lenght of direction line
#define BARHBK			30				// height of each power bar block	
#define PAD				5				// padding
#define STM		 		10				// margin from string to another
#define RXMARG 			25				// right margin from window in top box
#define VARMARG(NUM) 	10 + NUM * 15	// variable margin
#define NUMBARBLOCK 	10				// number of block of power bar
#define PWRLOWBOUND		4				// bound from low to mid power
#define PWRMIDBOUND		8				// bound from mid to high power

//------------------------------------------------------
// GRAPHICS CONSTANTS (COORDINATES)
//-----------------------------------------------------
#define TPBOXX1 	15					// top box X1 coord
#define TPBOXY1		155					// top box Y1 coord
#define TPBOXX2		785					// top box X2 coord
#define TPBOXY2		15					// top box Y2 coord
#define MAPSQX1 	15					// map box X1 coord
#define MAPSQY1 	585					// map box Y1 coord
#define MAPSQX2 	385					// map box X2 coord
#define MAPSQY2 	175					// map box Y2 coord
#define PWRSQX1		415					// power box X1 coord
#define PWRSQY1		585					// power box Y1 coord
#define PWRSQX2		785					// power box X2 coord
#define PWRSQY2		175					// power box Y1 coord
#define LNCBARX1 	475					// launch bar X1 coord
#define LNCBARY1 	540					// launch bar Y1 coord
#define LNCBARX2 	525					// launch bar X2 coord
#define LNCBARY2 	230					// launch bar Y2 coord
#define PWRLINEX 	750					// power line X1 coord
#define PWRLINEY 	230					// power line Y1 coord
#define DIRLINEX	580 				// direction line X1 coord
#define DIRLINEY 	550					// direction line Y1 coord	
#define BALLX 		650					// power ball initial x position
#define BALLY		230					// power ball initial y position
#define BALLR		10					// power ball radius
#define MAPDRONER 	5					// on map drone radius
#define MAPBALLR 	5					// on map ball radius

//-------------------------------------------------------
// COLOR CONSTANTS
//-------------------------------------------------------
#define VGA 		8					// color depth
#define BKG 		15					// background color
#define MCOL 		0					// menu color
#define LPBARCOL 	10					// low power bar color
#define MPBARCOL 	14					// medium power bar color
#define HPBARCOL 	12					// high power bar color
#define MAPBKG 		2					// map background color
#define MAPDRONECOL 4					// on map drone color
#define MAPBALLCOL	1					// on map ball color

//-----------------------------------
// USER PANEL STRING
//-----------------------------------
#define WNDTITLE 			"Catching drone 3D Simulator"								
#define WNDCAPT 			"USER PANEL"												
#define MCLKCMD(BTN, OBJ) 	"Use mouse " #BTN " click to position " #OBJ "!"		
#define LNCCMD 				"Drag the ball down to choose throw power!"					
#define STARTCMD 			"Press ENTER to start simulation!"							
#define PAUSECMD			"Press BACKSPACE to pause simulation!"						
#define ENDCMD				"Press ESC to quit the simulatator"							
#define RESETCMD 			"Press R to reset the simulation"							
#define STCAPT(STATE)	 	"Simulation is " #STATE									
#define BARCAPT 			"Power Bar"													
#define PWRLINCPT 			"PWR"														
#define DIRLINCPT 			"DIR"														

//------------------------------------
// SPACES CONSTANT
//-----------------------------------
#define GR_DIM		2					// graphic space dimension (2D)
#define SP_DIM	 	3					// world space dimension (3D)
#define X 			0					// first axis coord
#define Y 			1					// second axis coord
#define Z			2					// third axis coord
#define WRL_I		-100				// simulation world begin (meter)
#define WRL_F		100					// simulation world end (meter)

//-----------------------------------
// DRONE Z POSITION
//-----------------------------------
#define DINITH 5						// drone initial height

//-----------------------------------
// STATE OF GAME
//-----------------------------------
enum GAME_STATE {
	STOPPED,			// game is stopped
	RUNNING,			// game is running
	PAUSED				// game is paused
};

struct pstate {							// panel state structure
	int 	ball_positioned;			// ball is positioned in map
	int 	drone_positioned;			// drone is positioned in map
	int 	simul_state;				// simulation state
	float 	launch_ball[GR_DIM];		// ball in throw box position
	float 	ball_pos[SP_DIM];			// ball in map position
	float 	drone_pos[SP_DIM];			// drone in map position
	float 	power;						// current power of throw
	float	dir;						// current direction of throw
};

//----------------------------------
// PUBLIC: KEYBOARD RELATED FUNCTIONS
//----------------------------------

// Blocking: Return only after ESC key press
void wait_esc_key();

// Non blocking: Return 1 if ESC key is pressed, 0 otherwise
int nb_get_esc_key();

//----------------------------------
// PUBLIC: INIT AND EXIT ALLEGRO LIB
//----------------------------------

// Start to draw user panel on screen
void init_panel();

// Exit and close the user panel
void exit_panel();

// Main graphic loop
void graphic_loop(struct pstate* panel, float* d_new_pos, float* b_new_pos);

//----------------------------
// PUBLIC: GETTER AND SETTER
//----------------------------

// Re-set panel struct to initial state
void p_reset(struct pstate* panel);

// Set the launch box ball at initial defaul coordinates
void launch_ball_init(struct pstate* panel);

// Set the drone initial Z-position in map
void map_drone_init(struct pstate* panel);

// Set simulation state
void set_simul_state(struct pstate* panel, int state);

// Get simulation state
int get_simul_state(struct pstate* panel);

// Set drone and ball map position from real coord
void set_map_pos(struct pstate* panel, float* d_real_pos, float* b_real_pos);

// Return 1 if drone OR ball are positioned, 0 otherwise
int is_obj_posit(struct pstate* panel);

// Return 1 if drone AND ball are positioned, 0 otherwise
int are_obj_posit(struct pstate* panel);

// Get real world coordinates and put in provided float*
void get_real_coord(struct pstate* panel, float* d_pos, float* b_pos);

// Get user panel power setted by user
float get_power(struct pstate* panel);

// Get direction of ball setted by user
float get_dir(struct pstate* panel);

#endif
