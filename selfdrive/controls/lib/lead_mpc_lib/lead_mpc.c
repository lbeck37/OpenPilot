#include "acado_common.h"
#include "acado_auxiliary_functions.h"
#include "selfdrive/common/modeldata.h"
#include <stdio.h>
#include <math.h>

#define NX          ACADO_NX  /* Number of differential state variables.  */
#define NXA         ACADO_NXA /* Number of algebraic variables. */
#define NU          ACADO_NU  /* Number of control inputs. */
#define NOD         ACADO_NOD  /* Number of online data values. */

#define NY          ACADO_NY  /* Number of measurements/references on nodes 0..N - 1. */
#define NYN         ACADO_NYN /* Number of measurements/references on node N. */

#define N           ACADO_N   /* Number of intervals in the horizon. */

ACADOvariables acadoVariables;
ACADOworkspace acadoWorkspace;

typedef struct {
  double x_ego, v_ego, a_ego, x_l, v_l;
} state_t;


typedef struct {
  double x_ego[N+1];
  double v_ego[N+1];
  double a_ego[N+1];
  double j_ego[N];
  double x_l[N+1];
  double v_l[N+1];
  double t[N+1];
  double cost;
} log_t;

void init(double ttcCost, double distanceCost, double accelerationCost, double jerkCost){
  acado_initializeSolver();
  int    i;
  const int STEP_MULTIPLIER = 3;

  /* Initialize the states and controls. */
  for (i = 0; i < NX * (N + 1); ++i)  acadoVariables.x[ i ] = 0.0;
  for (i = 0; i < NU * N; ++i)  acadoVariables.u[ i ] = 0.0;

  /* Initialize the measurements/reference. */
  for (i = 0; i < NY * N; ++i)  acadoVariables.y[ i ] = 0.0;
  for (i = 0; i < NYN; ++i)  acadoVariables.yN[ i ] = 0.0;

  /* MPC: initialize the current state feedback. */
  for (i = 0; i < NX; ++i) acadoVariables.x0[ i ] = 0.0;
  // Set weights

  for (i = 0; i < N; i++) {
    double f = 20 * (T_IDXS[i+1] - T_IDXS[i]);
    // Setup diagonal entries
    acadoVariables.W[NY*NY*i + (NY+1)*0] = ttcCost * f; // exponential cost for time-to-collision (ttc)
    acadoVariables.W[NY*NY*i + (NY+1)*1] = distanceCost * f; // desired distance
    acadoVariables.W[NY*NY*i + (NY+1)*2] = accelerationCost * f; // acceleration
    acadoVariables.W[NY*NY*i + (NY+1)*3] = jerkCost * f; // jerk
  }
  acadoVariables.WN[(NYN+1)*0] = ttcCost * STEP_MULTIPLIER; // exponential cost for danger zone
  acadoVariables.WN[(NYN+1)*1] = distanceCost * STEP_MULTIPLIER; // desired distance
  acadoVariables.WN[(NYN+1)*2] = accelerationCost * STEP_MULTIPLIER; // acceleration

}


int run_mpc(state_t * x0, log_t * solution, double predicted_lead_x[N+1], double predicted_lead_v[N+1]){
  int i;

  for (i = 0; i < N + 1; ++i){
    acadoVariables.od[i*NOD] = predicted_lead_x[i];
    acadoVariables.od[i*NOD+1] =predicted_lead_v[i];
  }

  acadoVariables.x[0] = acadoVariables.x0[0] = x0->x_ego;
  acadoVariables.x[1] = acadoVariables.x0[1] = x0->v_ego;
  acadoVariables.x[2] = acadoVariables.x0[2] = x0->a_ego;

  acado_preparationStep();
  acado_feedbackStep();

  for (i = 0; i <= N; i++){
    solution->x_ego[i] = acadoVariables.x[i*NX];
    solution->v_ego[i] = acadoVariables.x[i*NX+1];
    solution->a_ego[i] = acadoVariables.x[i*NX+2];

    if (i < N){
      solution->j_ego[i] = acadoVariables.u[i];
    }
  }
  solution->cost = acado_getObjective();

  // Dont shift states here. Current solution is closer to next timestep than if
  // we shift by 0.2 seconds.

  return acado_getNWSR();
}
