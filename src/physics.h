//------------------------------------------------------------------------------------------
// PHYSICS_H: CONTAINS SOME USEFUL THINGS TO REPRESENT PHYSICS EVOLUTION OF SIMULATION WORLD
//------------------------------------------------------------------------------------------

#ifndef PHYSICS_H
#define PHYSICS_H

//------------------------------------
// PHYSICS CONSTANTS OF THE QUADCOPTER
//------------------------------------
#define NROTOR 			4		// number of rotor in a quadcopter
#define ROTMAXFORCE		15		// full power rotor thrust (N)
#define DMASS 			0.5		// total mass of the drone (kg)
#define ARMLENGHT 		0.2		// len of arm from centre of mass (m)
#define COEFF 			0.08	// coefficent of thrust (see readme)
#define INERTIAX		5.50E-3	// inertia of x (kg * m^2)
#define INERTIAY		5.50E-3	// inertia of y (kg * m^2)
#define INERTIAZ		1.00E-3	// inertia of z (kg * m^2)

//------------------------------------
// QUADCOPTER FINAL POSITION
//------------------------------------
#define DFINALH 		5		// drone final height

//------------------------------------
// PHYSICS CONSTANTS OF THE BALL
//------------------------------------
#define BMASS 			0.45	// total mass of the ball (kg)
#define RADIUS			0.5		// radius of the ball (m)

//------------------------------------
// SPACES CONSTANTS
//------------------------------------
#define GRAVITY 		9.81	// gravity constant
#define SP_DIM	 		3		// dimension of space
#define X 				0		// x is the first axis
#define Y 				1		// y is the first axis
#define Z 				2		// z is the first axis
#define FXR 			0		// front rotor
#define RXR 			1		// right rotor
#define BXR 			2		// rear rotor
#define LXR 			3		// left rotor
#define B2D_DIST_Z		1.5		// ball to drone collision distance (z axis)
#define B2D_DIST_XY		0.5		// ball to drone collision distance	(xy plan)	

//-------------------------------------
// SCALE AND BOUND
//-------------------------------------
#define DIRDOF			10		// number of degree of ball direction
#define MAXDIRL			-5		// max left direction
#define MAXDIRR			5		// max right direction
#define BLDIRSCALE		10		// direction attenuation
#define BLVELSCALEX		1		// X velocity of ball multiplier
#define BLVELSCALEY		1		// Y velocity of ball multiplier
#define BLVELSCALEZ		10		// Z velocity of ball multiplier
#define BLACCSCALEZ		2		// Z deceleration of ball multiplier
#define MAX_ACC_BOUND 	12		// limit the maximum desired acceleration
#define MAX_THR_BOUND 	0.85	// limit the maximux desired thrust

//-------------------------------------
// CONTROLLER GAINS AND BOUND
//-------------------------------------
#define KP_ACC			0.12	// X, Y gain of prop part of PD (acceleration)
#define KD_ACC			0.56	// X, Y gain of deriv part of PD (acceleration)
#define KP_ACC_Z		0.12	// Z gain of prop part of PD (acceleration)
#define KD_ACC_Z		0.56	// Z gain of deriv part of PD (acceleration)
#define KP_ANG 			0.030	// X, Y gain of proportional part of PID controller
#define KD_ANG 			0.023	// X, Y gain of derivative part of PID controller
#define KP_ANG_Z 		0.0025	// Z gain of proportional part of PID controller
#define KD_ANG_Z 		0.0026	// Z gain of derivative part of PID controller


struct dstate {					// drone state structure
	float 	rotor_dc[NROTOR];	// duty cycle [0, 1] imposed to rotor
	float 	bd_lin_vel[SP_DIM];	// linear velocity in body frame
	float 	bd_ang_vel[SP_DIM];	// angular velocity in body frame
	float 	fx_lin_pos[SP_DIM];	// linear position in fixed frame
	float 	fx_ang_pos[SP_DIM];	// angular position in fixed frame
	float 	fx_lin_vel[SP_DIM];	// linear velocity in fixed frame
	float 	fx_ang_vel[SP_DIM];	// angular velocity in fixed frame
};

struct bstate {					// ball structure 
	float 	position[SP_DIM];	// position of the ball in space in (m)
	float 	velocity[SP_DIM];	// velocity of the ball (m/s)
};

struct cstate {					// controller state structure
	float 	rotor_dc[NROTOR];	// duty cycle [0, 1] imposed to rotor
};

//----------------------------------------
// PUBLIC: DRONE PHYSICAL RELATED FUNCTIONS
//----------------------------------------

// Update the state of drone based on dt elapsed time
void d_up_state(struct dstate* drone, struct cstate* control, float dt);

//----------------------------------------
// PUBLIC: DRONE GETTER/SETTER
//----------------------------------------

// Set initial position of drone state
void d_set_init_pos(struct dstate* drone, float* position);

//------------------------------
// PUBLIC: BALL RELATED FUNCTIONS
//------------------------------

// Update the state of ball based on dt elapsed time
void b_up_state(struct bstate* ball, struct dstate* drone, float dt);

//----------------------------------------
// PUBLIC: BALL GETTER/SETTER
//----------------------------------------

// Set the initial position of ball state
void b_set_init_pos(struct bstate* ball, float* position);

// Set the initial velocity of ball towards origin
void b_set_init_vel(struct bstate* ball, float velocity, float direction);

//---------------------------------------------
// PUBLIC: CONTROLLER RELATED FUNCTIONS
//--------------------------------------------
	
// Try to stabilize drone (controller of stability main function)
void c_stab_control(
	struct dstate* drone, struct cstate* ctrl, float* des_angles, float th);

// Calculate ball final position and drives drone
void c_driver_control(
	struct dstate* drone, struct bstate* ball, struct cstate* control);

#endif