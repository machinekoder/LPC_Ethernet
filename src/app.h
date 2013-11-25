#pragma once

/*
************************************************************************************************
*                                             INCLUDE FILES
************************************************************************************************
*/

#include <defines.h>
#include <includes.h>
#include <sys/stat.h>
#include "taskStart.h"
#include "ethernetLinkLayer.h"
//#include "taskButton.h"
//#include "taskLed.h"
//#include "taskUsbConnection.h"

/*
************************************************************************************************
*                                             GLOBAL DEFINES
************************************************************************************************
*/
#if 0
typedef enum {
    ApplicationState_Working  = 1u,
    ApplicationState_Idle     = 2u,
    ApplicationState_Test     = 3u,
    ApplicationState_Homing   = 4u
} ApplicationState;

typedef struct {
    int16 stepsX;
    int16 stepsY;
    int16 stepsZ;
} CommandBufferItem;

typedef struct {
    int32 targetX;
    int32 targetY;
    int32 targetZ;
    uint32 feed;
} QueueItem;

extern ApplicationState applicationState;
#endif
/*
************************************************************************************************
*                                         FUNCTION PROTOTYPES
************************************************************************************************
*/
#if 0
bool App_putIntoCommandPuffer (int32 newXum, int32 newYum, int32 newZum, uint32 feed);
bool App_cncCalibrateZentool (double measuredDistanceX, double measuredDistanceY, double measuredDistanceZ);

/** starts moving in Direction X.
 *  @param stepsX are the steps >0= +  <0= -
 */

#endif
