#include "scheduler.h"

#define INVALID_ID 255
#define WHEEL_SIZE 32
#define NO_NEXT_ID 255


typedef struct WheelNode {
    uint8_t taskIndex;
    uint8_t nextIndex;
} WheelNode;

// ======= CÁC BIẾN TOÀN CỤC =======
sTask SCH_tasks_G[SCH_MAX_TASKS];
uint8_t wheel_heads[WHEEL_SIZE];
uint8_t next_task_index[SCH_MAX_TASKS];
uint8_t task_in_use[SCH_MAX_TASKS];

uint8_t task_count = 0;
uint32_t current_tick = 0;
uint8_t Error_code_G = 0;


void SCH_Init(void) {
    for (int i = 0; i < WHEEL_SIZE; i++)
        wheel_heads[i] = NO_NEXT_ID;
    for (int i = 0; i < SCH_MAX_TASKS; i++)
        task_in_use[i] = 0;

    task_count = 0;
    current_tick = 0;
    Error_code_G = 0;
}


unsigned char SCH_Add_Task(void (*pFunction)(void), unsigned int delay, unsigned int period) {
    if (task_count >= SCH_MAX_TASKS) {
        Error_code_G = 1;
        return INVALID_ID;
    }

    uint8_t id = 0;
    while (id < SCH_MAX_TASKS && task_in_use[id]) id++;
    if (id >= SCH_MAX_TASKS) {
        Error_code_G = 1;
        return INVALID_ID;
    }

    task_in_use[id] = 1;
    SCH_tasks_G[id].pTask = pFunction;
    SCH_tasks_G[id].Delay = delay;
    SCH_tasks_G[id].Period = period;
    SCH_tasks_G[id].RunMe = 0;
    SCH_tasks_G[id].TaskID = id;

    uint32_t slot = (current_tick + delay) % WHEEL_SIZE;


    next_task_index[id] = wheel_heads[slot];
    wheel_heads[slot] = id;

    task_count++;
    return id;
}

void SCH_Update(void) {
    uint32_t slot = current_tick % WHEEL_SIZE;
    uint8_t id = wheel_heads[slot];
    uint8_t prev = NO_NEXT_ID;

    while (id != NO_NEXT_ID) {
        SCH_tasks_G[id].RunMe++;

        if (SCH_tasks_G[id].Period > 0) {
            uint32_t next_slot = (current_tick + SCH_tasks_G[id].Period) % WHEEL_SIZE;


            uint8_t next_id = next_task_index[id];
            if (prev == NO_NEXT_ID)
                wheel_heads[slot] = next_id;
            else
                next_task_index[prev] = next_id;

            next_task_index[id] = wheel_heads[next_slot];
            wheel_heads[next_slot] = id;

            id = next_id;
        } else {
            prev = id;
            id = next_task_index[id];
        }
    }

    current_tick++;
}


void SCH_Dispatch_Tasks(void) {
    uint32_t slot = (current_tick - 1) % WHEEL_SIZE;
    uint8_t id = wheel_heads[slot];
    uint8_t prev = NO_NEXT_ID;

    while (id != NO_NEXT_ID) {
        if (SCH_tasks_G[id].RunMe > 0) {
            SCH_tasks_G[id].RunMe--;

            if (SCH_tasks_G[id].pTask != NULL)
                (*SCH_tasks_G[id].pTask)();

            if (SCH_tasks_G[id].Period == 0) {

                uint8_t next_id = next_task_index[id];
                if (prev == NO_NEXT_ID)
                    wheel_heads[slot] = next_id;
                else
                    next_task_index[prev] = next_id;

                task_in_use[id] = 0;
                task_count--;
                id = next_id;
                continue;
            }
        }
        prev = id;
        id = next_task_index[id];
    }
}

uint8_t SCH_Delete_Task(uint32_t taskID) {
    if (taskID >= SCH_MAX_TASKS || !task_in_use[taskID])
        return 0;

    for (int i = 0; i < WHEEL_SIZE; i++) {
        uint8_t id = wheel_heads[i];
        uint8_t prev = NO_NEXT_ID;

        while (id != NO_NEXT_ID) {
            if (id == taskID) {
                uint8_t next_id = next_task_index[id];
                if (prev == NO_NEXT_ID)
                    wheel_heads[i] = next_id;
                else
                    next_task_index[prev] = next_id;

                task_in_use[id] = 0;
                task_count--;
                return 1;
            }
            prev = id;
            id = next_task_index[id];
        }
    }
    return 0;
}
