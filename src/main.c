//-----------------------------------------------------
//
// CATCHING DRONE: SIMULATION OF SMART QUADCOPTER 
//
//-----------------------------------------------------
#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <semaphore.h>
#include <stdbool.h>
#include <retif.h>
#include <unistd.h>
#include <sys/types.h>
#include "task.h" 
#include "physics.h"
#include "userpanel.h"
#include "udp.h"

//-----------------------------------------------------
// TASK CONSTANTS & STRUCTS
//-----------------------------------------------------

#define RTF_OK 1
#define RTF_FAIL 0
#define RTF_ERROR -1

enum TASK_IDS 
{
	DRV_TASK, 							// quadcopter controller task			
	DRN_TASK, 							// drone update state task
	BLL_TASK, 							// ball update state task
	UDP_TASK, 							// ddp packet sender task
	PNL_TASK, 							// user panel handler task
	SPV_TASK, 							// supervisor task
	NUM_TASK 							// number of task
};

typedef void *(*routine_p) (void *);

void* udp_task();
void* panel_task();
void* drone_task();
void* ball_task();
void* driver_task();
void* supervisor_task();

struct task
{
	enum TASK_IDS 		id;
	pthread_t 			ptid;
	uint64_t 			period;
	struct rtf_task 	rtf_t;
	struct rtf_params 	rtf_tp;
	routine_p 			start_routine;
};

#define DRV_TASK_PER	20				// drv task period (ms)
#define DRV_TASK_FUN	driver_task		// drv task routine
#define DRN_TASK_PER	30				// drn task period (ms)
#define DRN_TASK_FUN	drone_task		// drn task routine
#define BLL_TASK_PER	30				// bll task period (ms)
#define BLL_TASK_FUN	ball_task		// bll task routine
#define UDP_TASK_PER	30				// udp task period (ms)
#define UDP_TASK_FUN	udp_task		// udp task routine
#define PNL_TASK_PER	30				// pnl task period (ms)
#define PNL_TASK_FUN	panel_task		// pnl task routine
#define SPV_TASK_PER	50				// spv task period (ms)
#define SPV_TASK_FUN	supervisor_task	// spv task routine

//-----------------------------------------------------
// TASK INITIALIZER
//-----------------------------------------------------

#define TASK_PER(ID) 	ID##_PER		// give task period (ms) from task id
#define TASK_FUN(ID)	ID##_FUN		// give task routine ptr from task id
										// give task init params from task id
#define TASK_INIT(ID) 				\
{									\
	.id = ID, 						\
	.period = TASK_PER(ID), 		\
	.start_routine = TASK_FUN(ID) 	\
}

struct task tasks[] =
{
	TASK_INIT(DRV_TASK),
	TASK_INIT(DRN_TASK),
	TASK_INIT(BLL_TASK),
	TASK_INIT(UDP_TASK),
	TASK_INIT(PNL_TASK),
	TASK_INIT(SPV_TASK),
};

//-----------------------------------
// STATE OF GAME
//-----------------------------------
#define GAMESPEED 	1				// game speed multiplier

//-----------------------------------------------------
// UDP DESTINATION
//-----------------------------------------------------
#define DEST_IP "10.30.3.169"
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
pthread_mutex_t mutex_d, 				// drone struct mutex
				mutex_b, 				// ball struct mutex
				mutex_p, 				// panel struct mutex
				mutex_c;				// controller struct mutex

//------------------------------------------------------
// TASK UTILITY FUNCTIONS
//------------------------------------------------------
void tp_init();
bool t_create(enum TASK_IDS id);

//-----------------------------------------------------
// START/STOP TASK FUNCTIONS
//-----------------------------------------------------
void task_start(bool udp, bool drone, bool ball, bool driver);
void task_stop(bool udp, bool drone, bool ball, bool driver);

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

//---------------------------------------
// ERROR MANAGEMENT AND LOGS
//---------------------------------------
void err_exit(const char* msg, int errcode);

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

	// connect to the daemon via a UNIX socket
	if (rtf_connect() < 0)
		err_exit("> Unable to connect with RETIF\n", -1);

	// create main threads
	if (!t_create(SPV_TASK) || !t_create(PNL_TASK))
		err_exit("> Unable to create tasks with RETIF\n", -1);

	// app terminate when user panel is closed
	pthread_join(tasks[PNL_TASK].ptid, NULL);
}

//----------------------
// TASK ROUTINE FUNCTIONS
//----------------------

// ---
// Send UDP packet with information about drone and ball pos
// return: void
// ---
void* udp_task() 
{
	struct 	dstate d_copy;	// copy of drone state structure
	struct 	bstate b_copy;	// copy of ball state structure
	struct 	rtf_task *t;	// pointer to rtf task struct
	int 	sock;			// descriptor of a socket
	
	sock = udp_init(DEST_IP, UDP_PORT);

	if (sock < 0)
		err_exit("> Unable to open the socket connection\n", -1);

	t = &tasks[UDP_TASK].rtf_t;
	
	if (rtf_task_attach(t, gettid()) != RTF_OK)
		err_exit("> Unable to attach UDP thread to RETIF\n", -1);

	rtf_task_start(t);
	
	while(1) 
	{
		safe_copy(&mutex_d, &d_copy, &drone, sizeof(struct dstate));
		safe_copy(&mutex_b, &b_copy, &ball, sizeof(struct bstate));

		udp_grap_send(
			sock, d_copy.fx_lin_pos, d_copy.fx_ang_pos, b_copy.position
		);

		rtf_task_wait_period(t);
	}
}

// ---
// Handle user panel and update with new drone/ball pos
// return: void
// ---
void* panel_task() 
{
	struct 	pstate p_copy;			// copy of panel state structure
	struct 	dstate d_copy;			// copy of drone state structure
	struct 	bstate b_copy;			// copy of ball state structure
	struct 	rtf_task *t;			// pointer to rtf task struct
	int 	esc_key_pressed = 0;	// boolean that indicates esc key pressed
	
	init_panel();
	
	t = &tasks[PNL_TASK].rtf_t;
	
	if (rtf_task_attach(t, gettid()) != RTF_OK)
		err_exit("> Unable to attach PANEL thread to RETIF\n", -1);

	rtf_task_start(t);
		
	while(!esc_key_pressed) 
	{
		safe_copy(&mutex_d, &d_copy, &drone, sizeof(struct dstate));
		safe_copy(&mutex_b, &b_copy, &ball, sizeof(struct bstate));	
		safe_copy(&mutex_p, &p_copy, &panel, sizeof(struct pstate));
		
		graphic_loop(&p_copy, d_copy.fx_lin_pos, b_copy.position);
		
		safe_copy(&mutex_p, &panel, &p_copy, sizeof(struct pstate));

		esc_key_pressed = nb_get_esc_key();
		rtf_task_wait_period(t);
	}

	exit_panel();
}

// ---
// Update ball structure in time
// return: void
// ---
void* ball_task() 
{
	struct 	bstate b_copy;	// copy of ball state structure
	struct 	dstate d_copy;	// copy of drone state structure
	struct 	rtf_task *t;	// pointer to rtf task struct
	float	dt;				// elapsed time
	
	dt = MILLI_TO_SEC(BLL_TASK_PER) * GAMESPEED;

	t = &tasks[BLL_TASK].rtf_t;
	
	if (rtf_task_attach(t, gettid()) != RTF_OK)
		err_exit("> Unable to attach BALL thread to RETIF\n", -1);

	rtf_task_start(t);
		
	while(1) 
	{
		safe_copy(&mutex_d, &d_copy, &drone, sizeof(struct dstate));
		safe_copy(&mutex_b, &b_copy, &ball, sizeof(struct bstate));
		
		b_up_state(&b_copy, &d_copy, dt);
		
		safe_copy(&mutex_b, &ball, &b_copy, sizeof(struct bstate));

		rtf_task_wait_period(t);
	}
}

// ---
// Update drone structure in time
// return: void
// ---
void* drone_task() 
{
	struct 	dstate d_copy;	// copy of drone state structure
	struct 	cstate c_copy;	// copy of controller state structure
	struct 	rtf_task *t;	// pointer to rtf task struct
	float 	dt;				// elapsed time
	
	dt = MILLI_TO_SEC(DRN_TASK_PER) * GAMESPEED;

	t = &tasks[DRN_TASK].rtf_t;
	
	if (rtf_task_attach(t, gettid()) != RTF_OK)
		err_exit("> Unable to attach DRONE thread to RETIF\n", -1);

	rtf_task_start(t);
		
	while(1) 
	{
		safe_copy(&mutex_d, &d_copy, &drone, sizeof(struct dstate));
		safe_copy(&mutex_c, &c_copy, &control, sizeof(struct cstate));
		
		d_up_state(&d_copy, &c_copy, dt);
		
		safe_copy(&mutex_d, &drone, &d_copy, sizeof(struct dstate));

		rtf_task_wait_period(t);
	}
}

// ---
// Calculate new rotor dc based on ball position
// return: void
// ---
void* driver_task() 
{
	struct 	dstate d_copy;	// copy of drone state structure
	struct 	bstate b_copy;	// copy of ball state structure
	struct 	cstate c_copy;	// copy of controller state structure
	struct 	rtf_task *t;	// pointer to rtf task struct

	t = &tasks[DRV_TASK].rtf_t;
	
	if (rtf_task_attach(t, gettid()) != RTF_OK)
		err_exit("> Unable to attach DRIVER thread to RETIF\n", -1);

	rtf_task_start(t);

	while(1) 
	{		
		safe_copy(&mutex_d, &d_copy, &drone, sizeof(struct dstate));
		safe_copy(&mutex_b, &b_copy, &ball, sizeof(struct bstate));
		
		c_driver_control(&d_copy, &b_copy, &c_copy);
		
		safe_copy(&mutex_c, &control, &c_copy, sizeof(struct cstate));

		rtf_task_wait_period(t);
	}
}

// ---
// Take care of state change starting/stopping task
// return: void
// ---
void* supervisor_task() 
{
	struct 	pstate p_copy;				// copy of drone state structure
	int 	first_run = 1;				// first run after reset?
	int 	curr_state, next_state;		// current and next states of simul
	struct 	rtf_task *t;				// pointer to rtf task struct
	
	curr_state = next_state = STOPPED;
	
	t = &tasks[SPV_TASK].rtf_t;

	int tid = gettid();
	
	if (rtf_task_attach(t, gettid()) != RTF_OK)
		err_exit("> Unable to attach SUPERVISOR thread to RETIF\n", -1);

	rtf_task_start(t);
	
	while(1) 
	{
		safe_copy(&mutex_p, &p_copy, &panel, sizeof(struct pstate));
		next_state = get_simul_state(&p_copy);

		switch (next_state) 
		{
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

		rtf_task_wait_period(t);
	}
}

//--------------------------------
// TASK UTILITY FUNCTIONS
//--------------------------------

// ---
// Initialize each task parameter struct with correct param
// return: void
// ---
void tp_init() 
{
	for (int i = 0; i < NUM_TASK; i++)
		rtf_params_set_period(&tasks[i].rtf_tp, tasks[i].period);
}

bool t_create(enum TASK_IDS id)
{
	rtf_task_init(&tasks[id].rtf_t);

	if (rtf_task_create(&tasks[id].rtf_t, &tasks[id].rtf_tp) != 1)
		return false;
	
	return (pthread_create(&tasks[id].ptid, NULL, tasks[id].start_routine, NULL) == 0);
}

//-------------------------
// START/STOP TASK FUNCTIONS
//-------------------------

// ---
// Create required task and leave id in TP structure
// bool udp: if udp = true, udp task will be created
// bool drone: if drone = true, drone task will be created
// bool ball: if ball = true, ball task will be created
// bool driver: if uddriverp = true, driver task will be created
// return: void
// ---
void task_start(bool udp, bool drone, bool ball, bool driver) {
	if(udp)
		t_create(UDP_TASK);
	if(drone)
		t_create(DRN_TASK);
	if(ball)
		t_create(BLL_TASK);
	if(driver)
		t_create(DRV_TASK);
}

// ---
// Kill choosen task
// bool udp: if udp = true, udp task will be killed
// bool drone: if drone = true, drone task will be killed
// bool ball: if ball = true, ball task will be killed
// bool driver: if uddriverp = true, driver task will be killed
// return: void
// ---
void task_stop(bool udp, bool drone, bool ball, bool driver) {
	if(udp)
		pthread_cancel(tasks[UDP_TASK].ptid);
	if(drone)
		pthread_cancel(tasks[DRN_TASK].ptid);
	if(ball)
		pthread_cancel(tasks[BLL_TASK].ptid);
	if(driver)
		pthread_cancel(tasks[DRV_TASK].ptid); 
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
	non_safe_reset(&drone, sizeof(struct dstate));
	non_safe_reset(&ball, sizeof(struct bstate));
	non_safe_reset(&control, sizeof(struct cstate));
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
void react_to_stop(int prev_state, int* first_run, struct pstate* p_copy) 
{
	switch(prev_state) 
	{
		case STOPPED:
			// drone and ball init
			obj_init(p_copy);
			// udp started
			task_start(*first_run, false, false, false);
			*first_run = 0;
			break;
		default:
			// udp/psx stop
			task_stop(true, true, true, true);
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
void react_to_run(int prev_state) 
{
	switch(prev_state) 
	{
		case STOPPED:
			// drone/ball/driver start
			task_start(false, true, true, true);
			break;
		case PAUSED:
			// udp/drone/ball/driver start
			task_start(true, true, true, true);
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
			task_stop(true, true, true, true);
			break;
		default:
			break;
	}
}

//---------------------------------------
// ERROR MANAGEMENT AND LOGS
//---------------------------------------
void err_exit(const char* msg, int errcode)
{
	printf("%s", msg);
	exit(errcode);
}