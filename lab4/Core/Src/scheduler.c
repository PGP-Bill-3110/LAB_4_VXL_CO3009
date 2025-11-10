#include "scheduler.h"

#define INVALID_ID 255
#define WHEEL_SIZE 32

typedef struct sTaskID {
    sTask task;
    struct sTaskID* next;
} sTaskID;

sTaskID* timer_wheel[WHEEL_SIZE];
uint8_t task_count = 0;
uint32_t current_tick = 0;
uint8_t Error_code_G = 0;


static sTaskID task_IDs[SCH_MAX_TASKS];
static uint8_t ID_used[SCH_MAX_TASKS];

void SCH_Init(void) {
    for (int i = 0; i < WHEEL_SIZE; i++) timer_wheel[i] = NULL;
    for (int i = 0; i < SCH_MAX_TASKS; i++) ID_used[i] = 0;
    task_count = 0;
    current_tick = 0;
    Error_code_G = 0;
}


static sTaskID* alloc_ID(void) {
    for (int i = 0; i < SCH_MAX_TASKS; i++) {
        if (!ID_used[i]) {
            ID_used[i] = 1;
            return &task_IDs[i];
        }
    }
    return NULL;
}


static void free_ID(sTaskID* ID) {
    int idx = ID - task_IDs;
    if (idx >= 0 && idx < SCH_MAX_TASKS) {
        ID_used[idx] = 0;
    }
}

unsigned char SCH_Add_Task(void (*pFunction)(), unsigned int delay, unsigned int period) {
    if (task_count >= SCH_MAX_TASKS) {
        Error_code_G = 1;
        return INVALID_ID;
    }

    sTaskID* ID = alloc_ID();
    if (!ID) return INVALID_ID;

    ID->task.pTask = pFunction;
    ID->task.Delay = delay;
    ID->task.Period = period;
    ID->task.RunMe = 0;
    ID->task.TaskID = task_count;

    uint32_t slot = (current_tick + delay) % WHEEL_SIZE;
    ID->next = timer_wheel[slot];
    timer_wheel[slot] = ID;

    task_count++;
    return ID->task.TaskID;
}

//  O(1)
void SCH_Update(void) {
    uint32_t slot = current_tick % WHEEL_SIZE;
    sTaskID* prev = NULL;
    sTaskID* ID = timer_wheel[slot];

    while (ID) {
        ID->task.RunMe++;

        if (ID->task.Period > 0) {
            uint32_t next_slot = (current_tick + ID->task.Period) % WHEEL_SIZE;


            if (prev) prev->next = ID->next;
            else timer_wheel[slot] = ID->next;


            sTaskID* to_move = ID;
            ID = (prev) ? prev->next : timer_wheel[slot];

            to_move->next = timer_wheel[next_slot];
            timer_wheel[next_slot] = to_move;
        } else {
            prev = ID;
            ID = ID->next;
        }
    }

    current_tick++;
}

void SCH_Dispatch_Tasks(void) {
    uint32_t slot = (current_tick - 1) % WHEEL_SIZE;
    sTaskID* prev = NULL;
    sTaskID* ID = timer_wheel[slot];

    while (ID) {
        if (ID->task.RunMe > 0) {
            ID->task.RunMe--;
            if (ID->task.pTask) (*ID->task.pTask)();


            if (ID->task.Period == 0) {
                if (prev) prev->next = ID->next;
                else timer_wheel[slot] = ID->next;

                free_ID(ID);
                task_count--;

                ID = (prev) ? prev->next : timer_wheel[slot];
                continue;
            }
        }
        prev = ID;
        ID = ID->next;
    }
}
