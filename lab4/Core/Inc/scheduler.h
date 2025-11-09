/*
 * scheduler.h
 *
 *  Created on: Oct 25, 2025
 *      Author: M S I
 */

#ifndef INC_SCHEDULER_H_
#define INC_SCHEDULER_H_
#include "main.h"

#define SCH_MAX_TASKS  10

typedef struct {
    void (*pTask)(void);
    uint32_t Delay;
    uint32_t Period;
    uint8_t RunMe;
    uint32_t TaskID;
} sTask;

extern sTask SCH_tasks_G[SCH_MAX_TASKS];
extern uint8_t Error_code_G;

void SCH_Init(void);
void SCH_Update(void);
unsigned char SCH_Add_Task(void (*pFunction)(), unsigned int DELAY, unsigned int PERIOD);
void SCH_Dispatch_Tasks(void);
uint8_t SCH_Delete_Task(uint32_t taskID);


#endif /* INC_SCHEDULER_H_ */
