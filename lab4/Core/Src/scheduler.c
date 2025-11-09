#include "scheduler.h"

#define INVALID_ID 255

sTask SCH_tasks_G[SCH_MAX_TASKS];
uint8_t Error_code_G = 0;


static uint8_t task_count = 0;


void SCH_Init(void) {
    for (int i = 0; i < SCH_MAX_TASKS; i++) {
        SCH_tasks_G[i].pTask = 0;
        SCH_tasks_G[i].Delay = 0;
        SCH_tasks_G[i].Period = 0;
        SCH_tasks_G[i].RunMe = 0;
        SCH_tasks_G[i].TaskID = INVALID_ID;
    }
    task_count = 0;
    Error_code_G = 0;
}


void SCH_Update(void) {

    for (uint8_t i = 0; i < task_count; i++) {
        if (SCH_tasks_G[i].pTask != 0) {
            if (SCH_tasks_G[i].Delay > 0) {
                SCH_tasks_G[i].Delay--;
            } else {
                SCH_tasks_G[i].RunMe++;
                if (SCH_tasks_G[i].Period > 0) {
                    SCH_tasks_G[i].Delay = SCH_tasks_G[i].Period;
                }
            }
        }
    }
}


void SCH_Dispatch_Tasks(void) {
    for (uint8_t i = 0; i < task_count; i++) {
        if (SCH_tasks_G[i].RunMe > 0) {
            SCH_tasks_G[i].RunMe--;
            if (SCH_tasks_G[i].pTask != 0) {
                (*SCH_tasks_G[i].pTask)();
            }
            if (SCH_tasks_G[i].Period == 0) {
                SCH_Delete_Task(i);
            }
        }
    }
}


unsigned char SCH_Add_Task(void (*pFunction)(), unsigned int DELAY, unsigned int PERIOD) {
    if (task_count >= SCH_MAX_TASKS) {
        Error_code_G = 1;
        return INVALID_ID;
    }

    uint8_t id = task_count;

    SCH_tasks_G[id].pTask = pFunction;
    SCH_tasks_G[id].Delay = DELAY;
    SCH_tasks_G[id].Period = PERIOD;
    SCH_tasks_G[id].RunMe = 0;
    SCH_tasks_G[id].TaskID = id;

    task_count++;
    return id;
}


uint8_t SCH_Delete_Task(uint32_t taskID) {
    if (taskID >= task_count) return 0;
    task_count--;
    if (taskID != task_count) {
        SCH_tasks_G[taskID] = SCH_tasks_G[task_count];
        SCH_tasks_G[taskID].TaskID = taskID;
    }


    SCH_tasks_G[task_count].pTask = 0;
    SCH_tasks_G[task_count].Delay = 0;
    SCH_tasks_G[task_count].Period = 0;
    SCH_tasks_G[task_count].RunMe = 0;
    SCH_tasks_G[task_count].TaskID = INVALID_ID;

    return 1;
}
