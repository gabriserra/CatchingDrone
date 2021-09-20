#include "physics.h"
#include <math.h>
#include <float.h>
#include <string.h>
#include <stdio.h>

#define UNUSED_VAR(x) ((void) (x))

//--------------------------------
// PRIVATE: MATH UTILITIES
//--------------------------------

// ---
// Get the sign (-1 1 0) of float number
// float num: number whose we want to evaluate sign
// return: int - 1 positive num, -1 negative num, or 0
// ---
static int sign(float num) {
	if (num > 0) 
		return 1;
	if (num < 0) 
		return -1;
	return 0;
}

//-----------------------------------------
// PRIVATE: DRONE PHYSICAL RELATED FUNCTIONS
//-----------------------------------------

// ---
// Compute drone thrust. Leave the result in T vector
// dstate* drone: pointer to drone state structure,
// float* T: pointer to Vector[3] in which result is leaved
// return: void
// ---
static void d_calc_thrust(struct dstate* drone, float* T) {
	int 	i;			// array index [0-NROTOR]
	float 	total = 0;	// total thrust of rotor
	
	// thrust of drone is the sum of each dc * the rotor max force
	for(i = 0; i < NROTOR; i++)
		total += drone->rotor_dc[i] * ROTMAXFORCE;
	
	// in body frame thrust is only act in Z axis
	T[X] = 0;
	T[Y] = 0;
	T[Z] = total;
}

// ---
// Compute torques. Leave the result in M vector
// dstate* drone: pointer to drone state structure,
// float* M: pointer to Vector[3] in which result is leaved
// return: void
// ---
static void d_calc_torques(struct dstate* drone, float* M) {
	int 	i;				// array index [0-NROTOR]
	float 	force[NROTOR];	// force of each rotor
	
	// force of each rotor is the rotor dc * the rotor max force
	for(i = 0; i < NROTOR; i++)
		force[i] = drone->rotor_dc[i] * ROTMAXFORCE;
		
	// torques are calculated with the difference of force * a coeff
	M[X] = ARMLENGHT * (force[LXR] - force[RXR]); 
	M[Y] = ARMLENGHT * (force[BXR] - force[FXR]);
	M[Z] = COEFF * (force[LXR] + force[RXR] - force[BXR] - force[FXR]);
}

// ---
// Compute linear acceleration of drone. Leave the result in fx_lin_acc vector
// dstate* drone: pointer to drone state structure,
// float* fx_lin_acc: pointer to Vector[3] in which result is leaved
// return: void
// ---
static void d_calc_lin_acc(struct dstate* drone, float* fx_lin_acc) {
	float 	phi, theta, psi;	// fixed angular position of drone
	float 	R[SP_DIM];			// rotation matrix
	float	T[SP_DIM];			// thrust vector
	
	d_calc_thrust(drone, T);
	
	phi = drone->fx_ang_pos[X];
	theta = drone->fx_ang_pos[Y];
	psi = drone->fx_ang_pos[Z];
	
	// rotation matrix calculation
	R[X] = sinf(theta);
	R[Y] = sinf(phi) * cosf(theta);
	R[Z] = cosf(theta) * cosf(phi);
	UNUSED_VAR(psi);
	
	// fixed linear acc is the body frame thrust projected on axis 
	fx_lin_acc[X] = ((T[Z] / DMASS) * R[X]);
	fx_lin_acc[Y] = ((T[Z] / DMASS) * R[Y]);
	fx_lin_acc[Z] = ((T[Z] / DMASS) * R[Z]) - GRAVITY;
}

// ---
// Compute angular acceleration of drone. Leave the result in bd_ang_acc vector
// dstate* drone: pointer to drone state structure,
// float* bd_ang_acc: pointer to Vector[3] in which result is leaved
// return: void
// ---
static void d_calc_ang_acc(struct dstate* drone, float* bd_ang_acc) {
	float 	p, q, r;	// body angular velocity of drone
	float 	M[SP_DIM];	// torques vector

	p = drone->bd_ang_vel[X];
	q = drone->bd_ang_vel[Y];
	r = drone->bd_ang_vel[Z];
	
	d_calc_torques(drone, M);
	
	// body ang acceleration is calculated starting from speed and inertia
	bd_ang_acc[X] = (1 / INERTIAX) * (M[X] - (INERTIAY - INERTIAZ) * q * r);
	bd_ang_acc[Y] = (1 / INERTIAY) * (M[Y] - (INERTIAZ - INERTIAX) * r * p);
	bd_ang_acc[Z] = (1 / INERTIAZ) * (M[Z] - (INERTIAX - INERTIAY) * p * q);
}


// ---
// Calculate the body angular velocity of drone from fixed angular velocity
// dstate* drone: pointer to drone state structure,
// return: void
// ---
static void d_calc_bd_ang_vel(struct dstate* drone) {
	float 	phi, theta, psi;				// drone fixed ang position
	float 	phi_dot, theta_dot, psi_dot;	// drone fixed ang velocity
	
	phi = drone->fx_ang_pos[X];
	theta = drone->fx_ang_pos[Y];
	psi = drone->fx_ang_pos[Z];
	UNUSED_VAR(psi);

	phi_dot = drone->fx_ang_vel[X];
	theta_dot = drone->fx_ang_vel[Y];
	psi_dot = drone->fx_ang_vel[Z];

	// body ang vel is calculated from fx ang vel projected on axis	
	drone->bd_ang_vel[X] = 
		phi_dot - (sinf(theta) * psi_dot);
	drone->bd_ang_vel[Y] = 
		(cosf(phi) * theta_dot) + (cosf(theta) * sinf(phi) * psi_dot);
	drone->bd_ang_vel[Z] = 
		(-sinf(phi) * theta_dot) + (cosf(theta) * cosf(phi) * psi_dot);
}

// ---
// Calculate the fixed ang velocity of drone starting from body ang velocity
// dstate* drone: pointer to drone state structure,
// return: void
// ---
static void d_calc_fx_ang_vel(struct dstate* drone) {
	float 	p, q, r;			// drone body ang velocity
	float 	phi, theta, psi;	// drone fixed angular position
	
	p = drone->bd_ang_vel[X];
	q = drone->bd_ang_vel[Y];
	r = drone->bd_ang_vel[Z];
	
	phi = drone->fx_ang_pos[X];
	theta = drone->fx_ang_pos[Y];
	psi = drone->fx_ang_pos[Z];
	UNUSED_VAR(psi);
	
	// fixed ang velocity is calculated from ang velocity projected on axis
	drone->fx_ang_vel[X] = 
		p + (sinf(phi) * tanf(theta) * q) + (cosf(phi) * tanf(theta) * r); 
	drone->fx_ang_vel[Y] = 
		(cosf(phi) * q) - (sinf(phi) * r);
	drone->fx_ang_vel[Z] = 
		((sinf(phi) / cosf(theta)) * q) + ((cosf(phi) / cosf(theta)) * r);	
}

// ---
// Update the body ang velocity of drone from body ang acceleration and time
// dstate* drone: pointer to drone state structure,
// float dt: elapsed time,
// float* bd_ang_acc: pointer to Vector[3] that contains accelerations
// return: void
// ---
static void d_up_bd_ang_vel(
		struct dstate* drone, float dt, float* bd_ang_acc) {
	
	int 	i;	// array index [0-SP_DIM]

	// new ang velocity is old one + passed time * ang acceleration
	for(i = 0; i < SP_DIM; i++)
		drone->bd_ang_vel[i] += dt * bd_ang_acc[i];
}

// ---
// Update the fixed ang position of drone from fixed angular velocity and time
// dstate* drone: pointer to drone state structure,
// float dt: elapsed time,
// return: void
// ---
static void d_up_fx_ang_pos(struct dstate* drone, float dt) {
	int 	i;	// array index [0-SP_DIM]
	
	// new ang position is old one + passed time * ang velocity
	for(i = 0; i < SP_DIM; i++)
		drone->fx_ang_pos[i] += dt * drone->fx_ang_vel[i];
}

// ---
// Update the fixed lin velocity of drone from fixed lin acceleration and time
// dstate* drone: pointer to drone state structure,
// float dt: elapsed time,
// float* fx_lin_acc: pointer to Vector[3] that contains accelerations
// return: void
// ---
static void d_up_fx_lin_vel(
		struct dstate* drone, float dt, float* fx_lin_acc) {
	
	int 	i;	// array index [0-SP_DIM]
		
	// new lin velocity is old one + passed time * lin acceleration
	for(i = 0; i < SP_DIM; i++)
		drone->fx_lin_vel[i] += dt * fx_lin_acc[i];
}

// ---
// Update the fixed lin position of drone from fixed lin velocity and time
// dstate* drone: pointer to drone state structure,
// float dt: elapsed time,
// return: void
// ---
static void d_up_fx_lin_pos(struct dstate* drone, float dt) {
	int 	i; // array index [0-SP_DIM]

	// new lin position is old one + passed time * lin velocity
	for(i = 0; i < SP_DIM; i++)
		drone->fx_lin_pos[i] += dt * drone->fx_lin_vel[i];

	// if lin position < 0, we have reached the floor
	if(drone->fx_lin_pos[Z] < 0)
		drone->fx_lin_vel[Z] = 0;
}

// ---
// Set drone rotor duty cycle
// dstate* drone: pointer to drone state structure,
// float* rotor_dc: pointer to Vector[4] that contains new rotor DC
// return: void
// ---
void d_set_rotor_dc(struct dstate* drone, float* rotor_dc) {
	int 	i; // array index [0-SP_DIM]
	for(i = 0; i < NROTOR; i++)
		drone->rotor_dc[i] = rotor_dc[i];
}

//-----------------------------------------
// PUBLIC: DRONE PHYSICAL RELATED FUNCTIONS
//-----------------------------------------

// ---
// Update the state of drone based on dt elapsed time and new rotor dc
// dstate* drone: pointer to drone state structure,
// float* rotor_dc: new rotor duty cicle,
// float dt: elapsed time
// return: void
// ---
void d_up_state(struct dstate* drone, struct cstate* control, float dt) {
	float 	fx_lin_acc[SP_DIM];		// fx frame linear acceleration
	float 	bd_ang_acc[SP_DIM]; 	// bd frame angular acceleration

	d_set_rotor_dc(drone, control->rotor_dc);

	d_calc_lin_acc(drone, fx_lin_acc);
	d_calc_ang_acc(drone, bd_ang_acc);
	
	d_up_bd_ang_vel(drone, dt, bd_ang_acc);
	d_calc_fx_ang_vel(drone);
	d_up_fx_ang_pos(drone, dt);
	d_up_fx_lin_vel(drone, dt, fx_lin_acc);
	d_up_fx_lin_pos(drone, dt);
}

//----------------------------------------
// PUBLIC: DRONE GETTER/SETTER
//----------------------------------------

// ---
// Set initial position of drone state
// dstate* drone: pointer to drone state structure,
// float* position: pointer to Vector[3] that contains new position
// return: void
// ---
void d_set_init_pos(struct dstate* drone, float* position) {
	int 	i; // array index [0-SP_DIM]
	for(i = 0; i < SP_DIM; i++)
		drone->fx_lin_pos[i] = position[i];
}

//----------------------------------------
// PRIVATE: BALL PHYSICAL RELATED FUNCTIONS
//----------------------------------------

// ---
// Update the velocity of ball from acceleration and time
// bstate* ball: pointer to ball state structure,
// float dt: elapsed time
// return: void
// ---
static void b_up_vel(struct bstate* ball, float dt) {
	// new velocity is passed time * acceleration
	ball->velocity[Z] -= dt * (GRAVITY / BLACCSCALEZ);

	// if ball have reached floor ball is stopped
	if(ball->position[Z] < 0)
		ball->velocity[X] = ball->velocity[Y] = ball->velocity[Z] = 0;

}

// ---
// Update the position of ball from velocity and time
// bstate* ball: pointer to ball state structure,
// float dt: elapsed time
// return: void
// ---
static void b_up_pos(struct bstate* ball, float dt) {
	// new position is passed time * acceleration
	ball->position[Z] += dt * ball->velocity[Z];

	if(ball->position[Z] < 0) {
		ball->position[Z] = 0;
	} else {
		ball->position[X] += dt * ball->velocity[X];
		ball->position[Y] += dt * ball->velocity[Y];
	}
}

// ---
// Check if ball has collided with drone or floor
// bstate* ball: pointer to ball state structure
// dstate* drone: pointer to ball state structure
// return: void
// ---
static int b_check_collision(struct bstate* ball, struct dstate* drone) {
	int 	i;				// array index [0-SP_DIM]
	int		coll_d, coll_f;	// collision with drone or with floor
	float 	b2d_d_xy = 0;	// ball to drone distance (only xy)
	float	b2d_d_z;		// ball to drone distance (only z)

	// calc distance in abs value in x-y plan
	for(i = 0; i < SP_DIM-1; i++)
		b2d_d_xy += powf(ball->position[i] - drone->fx_lin_pos[i], 2);
	sqrt(b2d_d_xy);

	// calc distance with sign in z axis
	b2d_d_z = ball->position[Z] - drone->fx_lin_pos[Z];

	// collision condition with drone and floor
	coll_d = b2d_d_xy < B2D_DIST_XY && b2d_d_z < B2D_DIST_Z && b2d_d_z > 0;
	coll_f = ball->position[Z] < 0;

	// if collision is happened, ball is stopped!
	if(coll_d || coll_f) {
		ball->velocity[X] = ball->velocity[Y] = ball->velocity[Z] = 0;
		return 1;
	}
	return 0;
}

//----------------------------------------
// PUBLIC: BALL PHYSICAL RELATED FUNCTIONS
//----------------------------------------

// ---
// Update the state of ball based on dt elapsed time
// bstate* ball: pointer to ball state structure
// dstate* drone: pointer to drone state structure
// float dt: elapsed time
// return: void
// ---
void b_up_state(struct bstate* ball, struct dstate* drone, float dt) {
	if(!b_check_collision(ball, drone)) {
		b_up_vel(ball, dt);
		b_up_pos(ball, dt);
	}
}

//----------------------------------------
// PUBLIC: BALL GETTER/SETTER
//----------------------------------------

// ---
// Set the initial position of ball state
// bstate* ball: pointer to ball state structure,
// float* position: pointer to Vector[3] that contains new position
// return: void
// ---
void b_set_init_pos(struct bstate* ball, float* position) {
	int 	i; // array index [0-SP_DIM]
	for(i = 0; i < SP_DIM; i++)
		ball->position[i] = position[i];
}

// ---
// Set the initial velocity of ball towards origin
// bstate* ball: pointer to ball state structure,
// float velocity: velocity new value
// float direction: direction [-5,5] from right to left
// return: void
// ---
void b_set_init_vel(struct bstate* ball, float velocity, float direction) {
	float 	dir_x, dir_y;			// direction x and y towards center of map
	float	b2o;					// ball to origin distance
	float	angle_from_0;			// angle from 0 to x,y point
	float	dir_mod_x, dir_mod_y;	// trajectory dir modifier
	
	// avoid ball direction out of bound
	if(direction < MAXDIRL || direction > MAXDIRR)
		direction = 0;

	// direction is "attenuated" using BLDIRSCALE coeff
	direction = direction / BLDIRSCALE;

	// calculate direction to reach center of the map
	b2o = fabs(ball->position[X]) + fabs(ball->position[Y]);
	dir_x = - ball->position[X] / b2o;
	dir_y = - ball->position[Y] / b2o;

	// calculate the angle between 0 and current point
	angle_from_0 = atan2f(ball->position[Y], ball->position[X]);
	
	// calculate right and left velocity modifier
	dir_mod_x = sin(angle_from_0) * direction;
	dir_mod_y = cos(angle_from_0) * -direction;

	// ball moves toward choosen direction
	ball->velocity[X] = (dir_x + dir_mod_x) * velocity * BLVELSCALEX;
	ball->velocity[Y] = (dir_y + dir_mod_y) * velocity * BLVELSCALEY;
	ball->velocity[Z] = velocity * BLVELSCALEZ;
}

//----------------------------------------------
// PRIVATE: CONTROLLER EMULATED SENSORS FUNCTIONS
//----------------------------------------------

// ---
// Emulate a gyro sensor to get ang position of drone. Leave result in ang_pos
// dstate* drone: pointer to drone state structure
// float* ang_pos: pointer to Vector[3] in which result is leaved
// return: void
// ---
static void c_gyro_get_pos(struct dstate* drone, float* ang_pos) {
	int 	i;	// array index [0-SP_DIM]
	for(i = 0; i < SP_DIM; i++)
		ang_pos[i] = drone->fx_ang_pos[i];
}

// ---
// Emulate a gyro sensor to get ang velocity of drone. Leave result in ang_vel
// dstate* drone: pointer to drone state structure
// float* ang_vel: pointer to Vector[3] in which result is leaved
// return: void
// ---
static void c_gyro_get_vel(struct dstate* drone, float* ang_vel) {
	int 	i; // array index [0-SP_DIM]
	for(i = 0; i < SP_DIM; i++)
		ang_vel[i] = drone->fx_ang_vel[i];	
}

// ---
// Emulate a gps sensor to get lin position of drone. Leave result in lin_pos
// dstate* drone: pointer to drone state structure
// float* lin_pos: pointer to Vector[3] in which result is leaved
// return: void
// ---
static void c_gps_get_pos(struct dstate* drone, float* lin_pos) {
	int 	i; // array index [0-SP_DIM]
	for(i = 0; i < SP_DIM; i++)
		lin_pos[i] = drone->fx_lin_pos[i];
}

// ---
// Emulate an accelerometer to get lin vel of drone. Leave result in lin_vel
// dstate* drone: pointer to drone state structure
// float* lin_vel: pointer to Vector[3] in which result is leaved
// return: void
// ---
static void c_acce_get_vel(struct dstate* drone, float* lin_vel) {
	int 	i; // array index [0-SP_DIM]
	for(i = 0; i < SP_DIM; i++)
		lin_vel[i] = drone->fx_lin_vel[i];	
}

// ---
// Emulate a proximity sensor to get lin pos of ball. Leave result in lin_pos
// bstate* ball: pointer to ball state structure,
// float* lin_pos: pointer to Vector[3] in which result is leaved
// return: void
// ---
static void c_prox_get_pos(struct bstate* ball, float* lin_pos) {
	int 	i; // array index [0-SP_DIM]
	for(i = 0; i < SP_DIM; i++)
		lin_pos[i] = ball->position[i];
}

// ---
// Emulate a proximity sensor to get lin vel of ball. Leave result in lin_vel
// bstate* ball: pointer to ball state structure,
// float* lin_vel: pointer to Vector[3] in which result is leaved
// return: void
// ---
static void c_prox_get_vel(struct bstate* ball, float* lin_vel) {
	int 	i; // array index [0-SP_DIM]
	for(i = 0; i < SP_DIM; i++)
		lin_vel[i] = ball->velocity[i];
}

//---------------------------------------------
// PRIVATE: CONTROLLER RELATED FUNCTIONS
//---------------------------------------------

// ---
// Starting from desired thrust and torques compute rotor forces
// float thrust: desired thrust,
// float* M: pointer to Vector[3] that contains desired torques,
// float* rotor_forces: pointer to Vector[3] in which result is leaved
// return: void
// ---
static void c_torque_to_forces(float thrust, float* M, float* rotor_forces) {
	float 	m_con_1, m_con_2;	// coefficent (see readme)
	
	m_con_1 = 1 / (2 * ARMLENGHT);
	m_con_2 = 1 / (4 * COEFF);

	rotor_forces[FXR] = (0.25 * thrust) - (m_con_1 * M[Y]) + (m_con_2 * M[Z]);
	rotor_forces[RXR] = (0.25 * thrust) - (m_con_1 * M[X]) + (m_con_2 * M[Z]);
	rotor_forces[BXR] = (0.25 * thrust) + (m_con_1 * M[Y]) - (m_con_2 * M[Z]);
	rotor_forces[LXR] = (0.25 * thrust) + (m_con_1 * M[X]) + (m_con_2 * M[Z]);
}

// ---
// Calculate the desired torques from gyro and accel data (PD controller)
// float* ut: pointer to Vector[3] in which result is leaved
// float* des_ang: pointer to Vector[3] that contains desired angle pos
// float* act_pos: pointer to Vector[3] that contains actual angle pos
// float* act_vel: pointer to Vector[3] that contains actual angle vel
// return: void
// ---
static void c_ctrl_torque(
		float* ut, float* des_ang, float* act_pos, float* act_vel) {	
	
	ut[X] = KP_ANG * (des_ang[X] - act_pos[X]) - KD_ANG * act_vel[X];
	ut[Y] = KP_ANG * (des_ang[Y] - act_pos[Y]) - KD_ANG * act_vel[Y];
	ut[Z] = KP_ANG_Z * (des_ang[Z] - act_pos[Z]) - KD_ANG_Z * act_vel[Z];
}

// ---
// Set the rotor duty cycle of drone starting from rotor forces
// cstate* control: pointer to controller state structure
// float* rotor_forces: pointer to Vector[3] that contains desired forces
// return: void
// ---
static void c_set_rotor_dc(struct cstate* control, float* rotor_forces) {
	int 	i;					// array index [0-NROTOR]
	float 	max, min, slope;	// min and max rotor forces
	
	// if desired force is < 0, is saturated to 0
	min = 0;
	for(i = 0; i < NROTOR; i++)
		if(rotor_forces[i] < min)
			min = rotor_forces[i];
	
	// if desired force is > 0, is saturated to ROTMAXFORCE
	max = ROTMAXFORCE;
	for(i = 0; i < NROTOR; i++)
		if(rotor_forces[i] > max)
			max = rotor_forces[i];

	// avoid divide by 0 exception		
	if(max - min == 0)
		slope = FLT_MAX;
	else
		slope = 1 / (max - min);

	// rotor force is mapped between 0-1 (duty cicle)	
	for(i = 0; i < NROTOR; i++)	
		control->rotor_dc[i] = slope * (rotor_forces[i] - min);
}

// ---
// Limit the desired acceleration to a bound value
// float* acc_des: pointer to Vector[3] that contains desired acceleration
// return: void
// ---
static void c_limit_power(float* acc_des) {
	int 	i; // array index [0-SP_DIM]
			
	for(i = 0; i < SP_DIM; i++)
		if(fabs(acc_des[i]) > MAX_ACC_BOUND)
			acc_des[i] = (sign(acc_des[i]) * MAX_ACC_BOUND);
}

// ---
// Try to predict the ball final position at given height (see readme)
// float* ball_pos: pointer to Vector[3] that contains actual ball position
// float* ball_vel: pointer to Vector[3] that contains actual ball velocity
// float final_height: final Z position of the ball
// float* final_pos: pointer to Vector[3] in which result is leaved
// return: void
// ---
void c_calc_ball_pos(
		float* b_pos_a, float* b_vel_a, float b_height_f, float* b_pos_f) {

	float 	t_act, t_settl;		// elapsed time after thrown and final
	float	b_vel_i;			// ball initial velocity					
	float	sqrt_term;			// terms that will be sqrt-ed
	float 	pos_i[SP_DIM];		// initial position of the ball
	float	g;					// deceleration of the ball

	g = GRAVITY / BLACCSCALEZ;
	sqrt_term = b_vel_a[Z] * b_vel_a[Z] + 2 * g * b_pos_a[Z];
	
	// avoid possible < 0 exception 
	if(sqrt_term < 0)
		sqrt_term = 0;
	
	t_act = (b_vel_a[Z] - sqrt(sqrt_term)) / -g;
	b_vel_i = b_vel_a[Z] + g * t_act;
	
	pos_i[X] = b_pos_a[X] - b_vel_a[X] * t_act;
	pos_i[Y] = b_pos_a[Y] - b_vel_a[Y] * t_act;
	
	sqrt_term = b_vel_i * b_vel_i - 2 * g * b_height_f;

	if(sqrt_term < 0)
		sqrt_term = 0;
	
	t_settl = (b_vel_i + sqrt(sqrt_term)) / g;

	b_pos_f[X] = t_settl * b_vel_a[X] + pos_i[X];
	b_pos_f[Y] = t_settl * b_vel_a[Y] + pos_i[Y];
	b_pos_f[Z] = b_height_f;
}

// ---
// Calculate the desired acceleration from ball final position (PD control)
// float* ut: pointer to Vector[3] in which result is leaved
// float* b_pos_f: pointer to Vector[3] that contains ball final pos (des pos)
// float* d_pos_a: pointer to Vector[3] that contains actual drone position
// float* d_vel_a: pointer to Vector[3] that contains actual drone velocity
// return: void
// ---
static void c_ctrl_accel(
		float* ut, float* b_pos_f, float* d_pos_a, float* d_vel_a) {	
	
	ut[X] = (KP_ACC * (b_pos_f[X] - d_pos_a[X])) - KD_ACC * d_vel_a[X];
	ut[Y] = (KP_ACC * (b_pos_f[Y] - d_pos_a[Y])) - KD_ACC * d_vel_a[Y];
	ut[Z] = (KP_ACC_Z * (b_pos_f[Z] - d_pos_a[Z])) - KD_ACC_Z * d_vel_a[Z];
}

// ---
// Calculate the angles desired to obtain given acceleration
// float* acc: pointer to Vector[3] that contains acceleration
// float* des_ang: pointer to Vector[3] in which result is leaved
// return: void
// ---
static void c_acc_to_ang(float* acc, float* des_ang) {
	des_ang[X] = atanf(acc[Y] / (GRAVITY + acc[Z]));
	des_ang[Y] = atanf(acc[X] / (acc[Z] + GRAVITY));
	des_ang[Z] = 0;
}

// ---
// Calculate the desired thrust from desired and actual ang position
// float* des_ang: pointer to Vector[3] that contains desired angles
// float* act_ang: pointer to Vector[3] that contains actual angles
// float* des_acc: pointer to Vector[3] that contains desired velocity
// return: float - desired thrust value
// ---
static float c_calc_des_thrust(
		float* des_ang, float* act_ang, float* des_acc) {
	
	float 	R[SP_DIM];	// rotation matrix
	float	thrust;		// des thrust

	R[X] = sin(des_ang[Y]) * cos(des_ang[X]);
	R[Y] = - sin(des_ang[Y]);
	R[Z] = cos(des_ang[Y]) * cos(des_ang[X]);

	thrust = ((DMASS * GRAVITY) / 
				(cosf(act_ang[Y]) * cosf(act_ang[X]))) 
					+ des_acc[Z] * R[Z];

	return thrust;
}

// ---
// Limit the desired thrust to 85% to avoid lose maneuverability
// float* acc_des: pointer to Vector[3] that contains desired acceleration
// return: float - limited thrust value
// ---
static float c_limit_thrust(float thrust) {
	if(thrust > (ROTMAXFORCE * NROTOR * MAX_THR_BOUND))
		thrust = ROTMAXFORCE * NROTOR * MAX_THR_BOUND;
	return thrust;
}

//---------------------------------------------
// PUBLIC: CONTROLLER RELATED FUNCTIONS
//--------------------------------------------

// ---
// Try to stabilize drone (controller of stability main function)
// dstate* drone: pointer to drone state structure,
// cstate* control: pointer to controller state structure
// float* des_ang: pointer to Vector[3] that contains desired angles
// float th: desired thrust
// return: void
// ---
void c_stab_control(
		struct dstate* drone, struct cstate* ctrl, float* des_ang, float th) {
	
	float 	act_pos[SP_DIM], act_vel[SP_DIM]; 	// actual data from gyro
	float	ut[SP_DIM];							// desired torques
	float 	rotor_force[NROTOR];				// desired rotor forces
	
	// get data from sensors
	c_gyro_get_pos(drone, act_pos);
	c_gyro_get_vel(drone, act_vel);
	
	// calculate and impose new rotor forces
	c_ctrl_torque(ut, des_ang, act_pos, act_vel);
	c_torque_to_forces(th, ut, rotor_force);
	c_set_rotor_dc(ctrl, rotor_force);
}

// ---
// Calculate ball final position and drives drone
// dstate* drone: pointer to drone state structure,
// bstate* ball: pointer to ball state structure,
// cstate* control: pointer to controller state structure
// return: void
// ---
void c_driver_control(
		struct dstate* drone, struct bstate* ball, struct cstate* control) {

	float 	b_pos_act[SP_DIM], b_vel_act[SP_DIM];		// ball actual pos/vel
	float 	b_pos_f[SP_DIM];							// ball final pos
	float	d_lin_pos[SP_DIM], d_lin_vel[SP_DIM];		// drone actual pos/vel
	float	d_ang_pos[SP_DIM];							// drone act ang pos
	float	des_acc[SP_DIM], des_ang[SP_DIM], des_th;	// desired attitude
		
	// Get data from sensors
	c_prox_get_pos(ball, b_pos_act);
	c_prox_get_vel(ball, b_vel_act);
	c_gps_get_pos(drone, d_lin_pos);
	c_acce_get_vel(drone, d_lin_vel);
	c_gyro_get_pos(drone, d_ang_pos);

	// Calculate ball final position
	c_calc_ball_pos(b_pos_act, b_vel_act, DFINALH, b_pos_f);

	// Calculate drone new angles and thrust
	c_ctrl_accel(des_acc, b_pos_f, d_lin_pos, d_lin_vel);
	c_limit_power(des_acc);	
	c_acc_to_ang(des_acc, des_ang);
	des_th = c_calc_des_thrust(des_ang, d_ang_pos, des_acc);
	des_th = c_limit_thrust(des_th);

	// Actuate the required attitude		
	c_stab_control(drone, control, des_ang, des_th);
}