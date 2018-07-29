//-----------------------------------------------------
//
// CATCHING DRONE: SIMULATION OF SMART QUADCOPTER 
//
//-----------------------------------------------------
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <semaphore.h>
#include "ptask.h" 
#include "physics.h"
#include "userpanel.h"
#include "udp.h"

//-----------------------------------------------------
// TASK CONSTANTS
//-----------------------------------------------------
#define DRV_TASK 	0			// quadcopter controller task			
#define DRN_TASK	1			// drone update state task
#define BLL_TASK	2			// ball update state task
#define UDP_TASK	3			// ddp packet sender task
#define PNL_TASK	4			// user panel handler task
#define SPV_TASK	5			// supervisor task
#define NUM_TASK	6			// number of task
#define DRV_PER		20			// drv task period (ms)
#define DRN_PER		30			// drn task period (ms)
#define BLL_PER		30			// bll task period (ms)
#define UDP_PER		30			// udp task period (ms)
#define PNL_PER		30			// pnl task period (ms)
#define SPV_PER		50			// spv task period (ms)
#define MSTOS(NUM)	NUM/1000.0	// millisecond to second macro
#define DRV_PRIO	2			// drv task priority [1low-99high]
#define DRN_PRIO	3			// drn task priority [1low-99high]
#define BLL_PRIO	3			// bll task priority [1low-99high]
#define UDP_PRIO	3			// udp task priority [1low-99high]
#define PNL_PRIO	2			// pnl task priority [1low-99high]
#define SPV_PRIO	1			// spv task priority [1low-99high]

//-----------------------------------
// STATE OF GAME
//-----------------------------------
#define STOPPED 	0			// game is stopped
#define RUNNING 	1			// game is running
#define PAUSED		2			// game is paused
#define GAMESPEED 	1			// game speed multiplier

//-----------------------------------------------------
// UDP DESTINATION
//-----------------------------------------------------
#define DEST_IP "131.114.193.90"
#define UDP_PORT 8000

//-----------------------------------------------------
// SIMULATION GLOBAL DATA STRUCTURES
//-----------------------------------------------------
struct dstate drone = 	{0};	// drone state structure 
struct bstate ball = 	{0};	// ball state structure
struct pstate panel = 	{0};	// panel state structure
struct cstate control = {0}; 	// controller state structure

//-----------------------------------------------------
// TASK GLOBAL DATA STRUCTURE
//-----------------------------------------------------
struct task_par tp[NUM_TASK] 	// vector of task parameter
							= {0};
pthread_t task_id[NUM_TASK];	// task id vector
pthread_mutex_t mutex_d, 		// drone struct mutex
				mutex_b, 		// ball struct mutex
				mutex_p, 		// panel struct mutex
				mutex_c;		// controller struct mutex

//-----------------------------------------------------
// TASK ROUTINE FUNCTIONS
//-----------------------------------------------------
void* udp_task();
void* panel_task();
void* drone_task();
void* ball_task();
void* driver_task();
void* supervisor_task();

//------------------------------------------------------
// TASK PARAMETER UTILITY FUNCTIONS
//------------------------------------------------------
void tp_init();

//-----------------------------------------------------
// START/STOP TASK FUNCTIONS
//-----------------------------------------------------
void task_start(int udp, int drone, int ball, int driver);
void task_stop(int udp, int drone, int ball, int driver);

//-------------------------
// INIT/RESET OBJECT FUNCTIONS
//-------------------------
void obj_reset();
void obj_init(struct pstate* p_copy);

//---------------------------------------
// STATE TRANSITION FUNCTIONS (see readme)
//---------------------------------------
void react_to_stop(int prev_state, int* first_run, struct pstate* p_copy);
void react_to_run(int prev_state);
void react_to_pause(int prev_state);

//----------------------
// MAIN FUNCTION
//----------------------

int main() {
	// stuff init
	tp_init();
	mutex_init(&mutex_d);
	mutex_init(&mutex_c);
	mutex_init(&mutex_b);
	mutex_init(&mutex_p);
	p_reset(&panel);

	// create main threads
	p_task_create(&task_id[SPV_TASK], supervisor_task, &tp[SPV_TASK]);
	p_task_create(&task_id[PNL_TASK], panel_task, &tp[PNL_TASK]);

	// app terminate when user panel is closed
	wait_for_task_end(task_id[PNL_TASK]);
}

//----------------------
// TASK ROUTINE FUNCTIONS
//----------------------

// ---
// Send UDP packet with information about drone and ball pos
// return: void
// ---
void* udp_task() {
	struct 	dstate d_copy;	// copy of drone state structure
	struct 	bstate b_copy;	// copy of ball state structure
	int 	sock;			// descriptor of a socket
	
	sock = udp_init(DEST_IP, UDP_PORT);
	set_period(&tp[UDP_TASK]);
	
	while(1) {
		safe_copy(&mutex_d, &d_copy, &drone, sizeof(struct dstate));
		safe_copy(&mutex_b, &b_copy, &ball, sizeof(struct bstate));

		udp_grap_send(
			sock, d_copy.fx_lin_pos, d_copy.fx_ang_pos, b_copy.position);

		if(deadline_miss(&tp[UDP_TASK])) 
			deadline_handle(tp, NUM_TASK);
		wait_for_period(&tp[UDP_TASK]);
	}	
}

// ---
// Handle user panel and update with new drone/ball pos
// return: void
// ---
void* panel_task() {
	struct 	pstate p_copy;			// copy of panel state structure
	struct 	dstate d_copy;			// copy of drone state structure
	struct 	bstate b_copy;			// copy of ball state structure
	int 	esc_key_pressed = 0;	// boolean that indicates esc key pressed
	
	init_panel();	
	set_period(&tp[PNL_TASK]);
	
	while(!esc_key_pressed) {
		safe_copy(&mutex_d, &d_copy, &drone, sizeof(struct dstate));
		safe_copy(&mutex_b, &b_copy, &ball, sizeof(struct bstate));	
		safe_copy(&mutex_p, &p_copy, &panel, sizeof(struct pstate));
		
		graphic_loop(&p_copy, d_copy.fx_lin_pos, b_copy.position);
		
		safe_copy(&mutex_p, &panel, &p_copy, sizeof(struct pstate));

		esc_key_pressed = nb_get_esc_key();
		if(deadline_miss(&tp[PNL_TASK])) 
			deadline_handle(tp, NUM_TASK);
		wait_for_period(&tp[PNL_TASK]);
	}
	exit_panel();
}

// ---
// Update ball structure in time
// return: void
// ---
void* ball_task() {
	struct 	bstate b_copy;	// copy of ball state structure
	struct 	dstate d_copy;	// copy of drone state structure
	float	dt;				// elapsed time
	
	dt = MSTOS(BLL_PER) * GAMESPEED;
	set_period(&tp[BLL_TASK]);
		
	while(1) {
		safe_copy(&mutex_d, &d_copy, &drone, sizeof(struct dstate));
		safe_copy(&mutex_b, &b_copy, &ball, sizeof(struct bstate));
		
		b_up_state(&b_copy, &d_copy, dt);
		
		safe_copy(&mutex_b, &ball, &b_copy, sizeof(struct bstate));

		if(deadline_miss(&tp[BLL_TASK])) 
			deadline_handle(tp, NUM_TASK);
		wait_for_period(&tp[BLL_TASK]);
	}
}

// ---
// Update drone structure in time
// return: void
// ---
void* drone_task() {
	struct 	dstate d_copy;	// copy of drone state structure
	struct 	cstate c_copy;	// copy of controller state structure
	float 	dt;				// elapsed time
	
	dt = MSTOS(DRN_PER) * GAMESPEED;
	set_period(&tp[DRN_TASK]);
		
	while(1) {
		safe_copy(&mutex_d, &d_copy, &drone, sizeof(struct dstate));
		safe_copy(&mutex_c, &c_copy, &control, sizeof(struct cstate));
		
		d_up_state(&d_copy, &c_copy, dt);
		
		safe_copy(&mutex_d, &drone, &d_copy, sizeof(struct dstate));

		if(deadline_miss(&tp[DRN_TASK])) 
			deadline_handle(tp, NUM_TASK);
		wait_for_period(&tp[DRN_TASK]);
	}
}

// ---
// Calculate new rotor dc based on ball position
// return: void
// ---
void* driver_task() {
	struct 	dstate d_copy;	// copy of drone state structure
	struct 	bstate b_copy;	// copy of ball state structure
	struct 	cstate c_copy;	// copy of controller state structure

	set_period(&tp[DRV_TASK]);
	
	while(1) {		
		safe_copy(&mutex_d, &d_copy, &drone, sizeof(struct dstate));
		safe_copy(&mutex_b, &b_copy, &ball, sizeof(struct bstate));
		
		c_driver_control(&d_copy, &b_copy, &c_copy);
		
		safe_copy(&mutex_c, &control, &c_copy, sizeof(struct cstate));

		if(deadline_miss(&tp[DRV_TASK])) 
			deadline_handle(tp, NUM_TASK);
		wait_for_period(&tp[DRV_TASK]);
	}
}

// ---
// Take care of state change starting/stopping task
// return: void
// ---
void* supervisor_task() {
	struct 	pstate p_copy;				// copy of drone state structure
	int 	first_run = 1;				// first run after reset?
	int 	curr_state, next_state;		// current and next states of simul
	
	curr_state = next_state = STOPPED;
	set_period(&tp[SPV_TASK]);

	while(1) {
		safe_copy(&mutex_p, &p_copy, &panel, sizeof(struct pstate));
		next_state = get_simul_state(&p_copy);

		switch (next_state) {
			case STOPPED:
				react_to_stop(curr_state, &first_run, &p_copy);
				break;
			case RUNNING:
				react_to_run(curr_state);	
				break;
			case PAUSED:
				react_to_pause(curr_state);
				break;
			default:
				break;		
		}
		curr_state = next_state;

		if(deadline_miss(&tp[SPV_TASK])) 
			deadline_handle(tp, NUM_TASK);
		wait_for_period(&tp[SPV_TASK]);
	}
}

//--------------------------------
// TASK PARAMETER UTILITY FUNCTIONS
//--------------------------------

// ---
// Initialize each task parameter struct with correct param
// return: void
// ---
void tp_init() {
	set_tp_param(&tp[DRV_TASK], DRV_PER, DRV_PRIO);
	set_tp_param(&tp[DRN_TASK], DRN_PER, DRN_PRIO);
	set_tp_param(&tp[BLL_TASK], BLL_PER, BLL_PRIO);
	set_tp_param(&tp[UDP_TASK], UDP_PER, UDP_PRIO);
	set_tp_param(&tp[PNL_TASK], PNL_PER, PNL_PRIO);
	set_tp_param(&tp[SPV_TASK], SPV_PER, SPV_PRIO);
}

//-------------------------
// START/STOP TASK FUNCTIONS
//-------------------------

// ---
// Create required task and leave id in TP structure
// int udp: if udp = 1, udp task will be created
// int drone: if drone = 1, drone task will be created
// int ball: if ball = 1, ball task will be created
// int driver: if uddriverp = 1, driver task will be created
// return: void
// ---
void task_start(int udp, int drone, int ball, int driver) {
	if(udp)
		p_task_create(&task_id[UDP_TASK], udp_task, &tp[UDP_TASK]);
	if(drone)
		p_task_create(&task_id[DRN_TASK], drone_task, &tp[DRN_TASK]);
	if(ball)
		p_task_create(&task_id[BLL_TASK], ball_task, &tp[BLL_TASK]);
	if(driver)
		p_task_create(&task_id[DRV_TASK], driver_task, &tp[DRV_TASK]);
}

// ---
// Kill choosen task
// int udp: if udp = 1, udp task will be killed
// int drone: if drone = 1, drone task will be killed
// int ball: if ball = 1, ball task will be killed
// int driver: if uddriverp = 1, driver task will be killed
// return: void
// ---
void task_stop(int udp, int drone, int ball, int driver) {
	if(udp)
		p_task_kill(task_id[UDP_TASK]);
	if(drone)
		p_task_kill(task_id[DRN_TASK]);
	if(ball)
		p_task_kill(task_id[BLL_TASK]);
	if(driver)
		p_task_kill(task_id[DRV_TASK]); 
}

//-------------------------
// INIT/RESET OBJECT FUNCTIONS
//-------------------------

// ---
// Init drone and ball with data from panel
// pstate* panel: pointer to panel state structure
// return: void
// ---
void obj_init(struct pstate* p_copy) {
	float 	pw, dir;			// power and direction of ball
	float 	d_init_pos[SP_DIM];	// drone init position
	float	b_init_pos[SP_DIM];	// ball init position

	// get pos, dir and power from panel
	get_real_coord(p_copy, d_init_pos, b_init_pos);
	pw = get_power(p_copy);
	dir = get_dir(p_copy);
	
	// set data to drone and ball
	d_set_init_pos(&drone, d_init_pos);
	b_set_init_pos(&ball, b_init_pos);
	b_set_init_vel(&ball, pw / 2, dir);
}

// ---
// Reset ball, controller and drone structure
// return: void
// ---
void obj_reset() {
	n_safe_reset(&drone, sizeof(struct dstate));
	n_safe_reset(&ball, sizeof(struct bstate));
	n_safe_reset(&control, sizeof(struct cstate));
}

//---------------------------------------
// STATE TRANSITION FUNCTIONS (see readme)
//---------------------------------------

// ---
// React to input STOP state
// pstate* panel: pointer to panel state structure
// int prev_state: old simulation state
// int* first_run: indicate the first run after a reset
// return: void
// ---
void react_to_stop(int prev_state, int* first_run, struct pstate* p_copy) {
	switch(prev_state) {
		case STOPPED:
			// drone and ball init
			obj_init(p_copy);
			// udp started
			task_start(*first_run, 0, 0, 0);
			*first_run = 0;
			break;
		default:
			// udp/psx stop
			task_stop(1, 1, 1, 1);
			*first_run = 1;
			// reset all
			obj_reset();
			break;
	}
}

// ---
// React to input RUN state
// int prev_state: old simulation state
// return: void
// ---
void react_to_run(int prev_state) {
	switch(prev_state) {
		case STOPPED:
			// drone/ball/driver start
			task_start(0, 1, 1, 1);
			break;
		case PAUSED:
			// udp/drone/ball/driver start
			task_start(1, 1, 1, 1);
			break;
		default:
			break;
	}			
}

// ---
// React to input PAUSE state
// int prev_state: old simulation state
// return: void
// ---
void react_to_pause(int prev_state) {
	switch(prev_state) {
		case RUNNING:
			// udp/drone/ball/driver start
			task_stop(1, 1, 1, 1);
			break;
		default:
			break;
	}
}