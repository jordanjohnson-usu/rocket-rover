#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

////////////////////////////////////////////////////////////////////////////////
//
// Rover Configuration
//
// Change these macros to adjust settings for various rover behavior.
//
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////
////////// Rover drive speeds //////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

// The speed (0-249) at which the rover drives when exiting the canister
#define EXIT_SPEED      249

// The speed (0-249) at which the rover drives after exiting the canister
#define DRIVE_SPEED     249

////////// Rover drive speeds //////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////
////////// Timing for different states of the rover's main state machine ///////////////////
////////////////////////////////////////////////////////////////////////////////////////////
/*
Do not change the flight values for testing!
Instead, commment out the flight section and uncomment the test section
*/

////////// FLIGHT //////////
// #define WAIT_FOR_LAUNCH_LED_OFF_TIME    (uint32_t) 10 * ONE_SECOND     // LED turns green for this amount of time before turning off
// #define WAIT_FOR_LANDING_LED_OFF_TIME   (uint32_t) 10 * ONE_SECOND     // LED turns red for this amount of time before turning off
// #define WAIT_FOR_LANDING_TIME           (uint32_t) 10 * ONE_MINUTE     // Rover waits this amount of time after launch before attempting to exit the canister
// #define EXIT_TIME                       (uint32_t) 10 * ONE_SECOND     // Rover drives forward this amount of time in an attempt to exit the canister
// #define DRIVE_FORWARD_DELAY             (uint32_t) 100                 // Delay between exiting canister and driving forward to reduce back EMF if the rover wheels change direction
// #define DRIVE_TIME                      (uint32_t) 30 * ONE_SECOND     // Rover drives forward this amount of time
// #define DISPENSE_TIME                   (uint32_t) 1  * ONE_MINUTE     // Rover runs dispenser motor this amount of time (actually takes about 35 seconds)
// #define SIGNAL_ONBOARD_DATA_CUBE_TIME   (uint32_t) 10 * ONE_SECOND     // Rover waits this amount of time after dispensing and before signaling the onboard data cube
////////// FLIGHT //////////


////////// TEST //////////
#define WAIT_FOR_LAUNCH_LED_OFF_TIME    (uint32_t) 2  * ONE_SECOND
#define WAIT_FOR_LANDING_LED_OFF_TIME   (uint32_t) 2  * ONE_SECOND
#define WAIT_FOR_LANDING_TIME           (uint32_t) 5  * ONE_SECOND
#define EXIT_TIME                       (uint32_t) 2  * ONE_SECOND
#define DRIVE_FORWARD_DELAY             (uint32_t) 100
#define DRIVE_TIME                      (uint32_t) 10 * ONE_SECOND
#define DISPENSE_TIME                   (uint32_t) 35 * ONE_SECOND
#define SIGNAL_ONBOARD_DATA_CUBE_TIME   (uint32_t) 5  * ONE_SECOND
////////// TEST //////////

////////// Timing for different states of the rover's main state machine ///////////////////



////////////////////////////////////////////////////////////////////////////////////////////
////////// Launch detection settings ///////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

#define LAUNCH_FORCE 3                                      // Force in Gs the accelerometer watches for to detect launch

// #define LAUNCH_FORCE_CNT_THRESHOLD 58                       // Number of samples out of 64 that must be >= LAUNCH_FORCE to trigger is_launched
                                                            // 58 is 90% of 64 samples

#define LAUNCH_FORCE_CNT_THRESHOLD 2                      // TEST //

////////// Launch detection settings ///////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////
////////// No movement detection settings //////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

// Upper and lower threshold of acceleration in (1/2) m/s/s the accelerometer watches for to detect that the rover is not moving
// No motion is 19.6 (1/2) m/s/s
#define NO_MOVEMENT_TOLERANCE 5

////////// No movement detection settings //////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////
////////// Accelerometer Callibration //////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
/*
Uncomment the section for the rover controller serial number you are using.
Please comment both sections out when you push so we get an error if we forget to configure this.
*/

////////// Rover Controller SN1 //////////
// #define X_AXIS_ERROR ?
// #define Y_AXIS_ERROR ?
// #define Z_AXIS_ERROR ?
////////// Rover Controller SN1 //////////


////////// Rover Controller SN2 //////////
// #define X_AXIS_ERROR 2
// #define Y_AXIS_ERROR 2
// #define Z_AXIS_ERROR 0
////////// Rover Controller SN2 //////////

////////// Accelerometer Callibration //////////////////////////////////////////////////////

#endif // CONFIG_H