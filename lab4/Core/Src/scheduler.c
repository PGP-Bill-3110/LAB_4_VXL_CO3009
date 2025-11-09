#include "scheduler.h"

#define INVALID_ID 255
#define WHEEL_SIZE 32

typedef struct sTaskNode {
    sTask task;
    struct sTaskNode* next;
} sTaskNode;

sTaskNode* timer_wheel[WHEEL_SIZE];
uint8_t task_count = 0;
uint32_t current_tick = 0;
uint8_t Error_code_G = 0;

static sTaskNode task_nodes[SCH_MAX_TASKS];
static uint8_t node_used[SCH_MAX_TASKS];

void SCH_Init(void) {
    for (int i = 0; i < WHEEL_SIZE; i++) timer_wheel[i] = NULL;
    for (int i = 0; i < SCH_MAX_TASKS; i++) { node_used[i] = 0;}
    task_count = 0;
    current_tick = 0;
    Error_code_G = 0;
}

static sTaskNode* alloc_node(void) {
    for (int i = 0; i < SCH_MAX_TASKS; i++) {
        if (!node_used[i]) {
            node_used[i] = 1;
            return &task_nodes[i];
        }
    }
    return NULL;
}

static void SCH_Delete_Task(sTaskNode* node) {
    int idx = node - task_nodes;
    if (idx >= 0 && idx < SCH_MAX_TASKS) node_used[idx] = 0;
}

unsigned char SCH_Add_Task(void (*pFunction)(), unsigned int delay, unsigned int period) {
    if (task_count >= SCH_MAX_TASKS) {
        Error_code_G = 1;
        return INVALID_ID;
    }
    sTaskNode* node = alloc_node();
    if (!node) return INVALID_ID;
    node->task.pTask = pFunction;
    node->task.Delay = delay;
    node->task.Period = period;
    node->task.RunMe = 0;
    node->task.TaskID = task_count;


    uint32_t slot = (current_tick + delay) % WHEEL_SIZE;
    node->next = timer_wheel[slot];
    timer_wheel[slot] = node;
    task_count++;
    return node->task.TaskID;
}

// O(1)
void SCH_Update(void) {
    uint32_t slot = current_tick % WHEEL_SIZE;
    sTaskNode* prev = NULL;
    sTaskNode* node = timer_wheel[slot];
    while (node) {
        node->task.RunMe++;

        if (node->task.Period > 0) {
            // move to future slot
            uint32_t next_slot = (current_tick + node->task.Period) % WHEEL_SIZE;

            // Remove from current slot
            if (prev) prev->next = node->next;
            else timer_wheel[slot] = node->next;

            // Insert into new slot
            sTaskNode* to_move = node;
            node = (prev) ? prev->next : timer_wheel[slot];

            to_move->next = timer_wheel[next_slot];
            timer_wheel[next_slot] = to_move;
        } else {
            // one-shot: just move to next
            prev = node;
            node = node->next;
        }
    }
    current_tick++;
}

void SCH_Dispatch_Tasks(void) {
    uint32_t slot = (current_tick - 1) % WHEEL_SIZE;
    sTaskNode* prev = NULL;
    sTaskNode* node = timer_wheel[slot];
    while (node) {
        if (node->task.RunMe > 0) {
            node->task.RunMe--;
            if (node->task.pTask) (*node->task.pTask)();
            if (node->task.Period == 0) {
                // Delete one-shot task
                if (prev) prev->next = node->next;
                else timer_wheel[slot] = node->next;
                free_node(node);
                task_count--;
                // Restart scan from prev/slot in case link changed
                node = (prev) ? prev->next : timer_wheel[slot];
                continue;
            }
        }
        prev = node;
        node = node->next;
    }
}
