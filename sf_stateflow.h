#ifndef __STATEFLOW_H_
#define __STATEFLOW_H_

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <windows.h>

#define CLOCK_MAX_LIMIT 4294967290

typedef enum StateFlowStateTable
{
    STATE_NULL = 0,

    /*在下面这里添加自定义状态*/
    TEST_1,
    TEST_2,
    TEST_3,
    /*在上面这里添加自定义状态*/

    NUM_OF_STATE, // 保持在最后一个，不可删除
} stateflow_state_table_e_t;

typedef struct StateFlowDataBox
{
    int test;
} stateflow_message_box_s_t;

#define EHSF_MSG stateflow_msg

typedef struct StateFlowEvent
{
    bool event_triggering_state; // 本次执行事件触发状态

    stateflow_state_table_e_t toward_state; // 指向的状态

    uint8_t priority; // 事件优先级

    bool (*guard)(stateflow_message_box_s_t *stateflow_msg); // 事件检测方法
} stateflow_event_s_t;

typedef struct StateFlowState
{
    /*状态底层数据*/
    stateflow_state_table_e_t state_name; // 状态名称

    stateflow_event_s_t *exit_events;           // 状态出口事件
    uint8_t number_of_exit_events;              // 最大状态出口事件数量
    uint8_t number_of_exit_events_that_instack; // 已设置的状态出口事件数量

    void (*entry)(stateflow_message_box_s_t *stateflow_msg);  // 状态进入时方法
    void (*during)(stateflow_message_box_s_t *stateflow_msg); // 状态执行时方法
    void (*exit)(stateflow_message_box_s_t *stateflow_msg);   // 状态退出时方法

    /*状态运行数据*/
    bool is_need_to_reset; // 进入状态时是否需要重置状态运行数据
    uint32_t uptime;       // 状态持续时间
} stateflow_state_s_t;

#define STATE_METHOD_NULL NULL

typedef enum StateFlowError
{
    OK = 0,
    STATEFLOW_INIT_INPUT_ERROR,
    STATEFLOW_INIT_MALLOC_ERROR,
    STATE_CREATE_INPUT_ERROR,
    STATE_CREATE_MALLOC_ERROR,
    EXIT_EVENT_ADD_INPUT_ERROR,
    EXIT_EVENT_ADD_NUM_ERROR,
} stateflow_error;

typedef struct StateFlow
{
    stateflow_error status; // 状态机运行状态

    stateflow_state_s_t *state_list;     // 系统所有状态
    stateflow_state_table_e_t now_state; // 系统当前状态

    stateflow_state_table_e_t last_state; // 上一个状态，状态退出时更新

    uint32_t step_clock; // 系统步进时钟

    stateflow_message_box_s_t message_box;
} stateflow_s_t;

stateflow_error EHSF_Init(stateflow_s_t *stateflow, stateflow_state_table_e_t initial_state);

stateflow_error EHSF_CreateState(stateflow_s_t *stateflow, stateflow_state_table_e_t state_name,
                                 uint8_t number_of_exit_events, bool is_need_to_reset,
                                 void (*entry)(stateflow_message_box_s_t *stateflow_msg),
                                 void (*during)(stateflow_message_box_s_t *stateflow_msg),
                                 void (*exit)(stateflow_message_box_s_t *stateflow_msg));
stateflow_error EHSF_StateAddExitEvent(stateflow_s_t *stateflow, stateflow_state_table_e_t state_name,
                                       stateflow_state_table_e_t toward_state, uint8_t priority,
                                       bool (*guard)(stateflow_message_box_s_t *stateflow_msg));
void EHSF_Step(stateflow_s_t *stateflow);

#endif /* __STATEFLOW_H_ */
