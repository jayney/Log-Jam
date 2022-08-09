#ifndef __ADSR_H
  #define __ADSR_H
  #include <unistd.h>

// ADSR status definitions
#define PHASE_STOPPED       0
#define PHASE_ATTACK        1
#define PHASE_ATTACK_DONE   2
#define PHASE_DECAY         3
#define PHASE_DECAY_DONE    4
#define PHASE_SUSTAIN       5
#define PHASE_RELEASE       6

// ADSR Directions
#define ENV_UP              1
#define ENV_DOWN            0

//ADSR Shape - Note, LIN only applies to attack, and decay/release default to POW2
#define SHAPE_LIN           0
#define SHAPE_POW2          1
#define SHAPE_POW3          2
#define SHAPE_POW4          3

// Function Protoypes
uint8_t get_adsr_status(uint8_t egen);
void adsr_start(uint8_t egen);
void adsr_restart(uint8_t egen);
void adsr_release(uint8_t egen);
void adsr_init(uint8_t egen, float a_val, float d_val, float s_val, float r_val, uint8_t shape);
void adsr_set(uint8_t egen, uint8_t mode, float value);
float adsr_get_val(uint8_t egen);


#endif  