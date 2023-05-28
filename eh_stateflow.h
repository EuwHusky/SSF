#ifndef __STATEFLOW_H_
#define __STATEFLOW_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/*在下面这里添加自定义头文件*/
#include <windows.h>
/*在上面这里添加自定义头文件*/

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
    uint32_t step_clock; // 系统步进时钟
    /*在下面这里添加自定义数据*/
    int test;
    /*在上面这里添加自定义数据*/
} stateflow_message_box_s_t;

/*在下面这里添加自定义数据宏*/
#define EHSF_MSG stateflow_msg // 状态机信箱指针
/*在上面这里添加自定义数据宏*/

#define CLOCK_MAX_LIMIT 4294967290 // 步进时钟上限

/**
 * @brief 状态机 事件结构体
 */
typedef struct StateFlowEvent
{
    bool event_triggering_state; // 本次执行事件触发状态

    stateflow_state_table_e_t toward_state; // 指向的状态

    uint8_t priority; // 事件优先级

    bool (*guard)(stateflow_message_box_s_t *stateflow_msg); // 事件检测方法
} stateflow_event_s_t;

#define GUARD_TRIGGERED true      // 出口事件触发
#define GUARD_NOT_TRIGGERED false // 出口事件未触发

/**
 * @brief 状态机 状态结构体
 */
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

#define STATE_METHOD_NULL NULL // 空方法

/**
 * @brief 状态机 运行状态
 */
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

/**
 * @brief 状态机 结构体
 */
typedef struct StateFlow
{
    stateflow_error status; // 状态机运行状态

    stateflow_state_s_t *state_list;     // 系统所有状态
    stateflow_state_table_e_t now_state; // 系统当前状态

    stateflow_state_table_e_t last_state; // 上一个状态，状态退出时更新

    stateflow_message_box_s_t message_box;
} stateflow_s_t;

/**
 * @name    EHSF_Init
 * @brief   状态机初始化
 * @param stateflow     状态机结构体地址
 * @param initial_state 状态机系统初始状态
 * @return  stateflow_error
 * @note    无
 */
stateflow_error EHSF_Init(stateflow_s_t *stateflow, stateflow_state_table_e_t initial_state);

/**
 * @name    EHSF_CreateState
 * @brief   创建一个状态
 * @param stateflow             状态机结构体地址
 * @param state_name            此状态的名称/枚举值
 * @param number_of_exit_events 此状态拥有的出口事件总数
 * @param is_need_to_reset      此状态在进入时是否需要重置
 * @param entry                 此状态进入时方法
 * @param during                此状态执行时方法
 * @param exit                  此状态退出时方法
 * @return  stateflow_error
 * @note    无
 */
stateflow_error EHSF_CreateState(stateflow_s_t *stateflow, stateflow_state_table_e_t state_name,
                                 uint8_t number_of_exit_events, bool is_need_to_reset,
                                 void (*entry)(stateflow_message_box_s_t *stateflow_msg),
                                 void (*during)(stateflow_message_box_s_t *stateflow_msg),
                                 void (*exit)(stateflow_message_box_s_t *stateflow_msg));

/**
 * @name    EHSF_StateAddExitEvent
 * @brief   为状态添加一个出口事件
 * @param stateflow     状态机结构体地址
 * @param state_name    所属状态的名称/枚举值
 * @param toward_state  此出口事件指向的状态
 * @param priority      此出口事件的触发优先级
 * @param guard         此出口事件的检测方法
 * @return  stateflow_error
 * @note    无
 */
stateflow_error EHSF_StateAddExitEvent(stateflow_s_t *stateflow, stateflow_state_table_e_t state_name,
                                       stateflow_state_table_e_t toward_state, uint8_t priority,
                                       bool (*guard)(stateflow_message_box_s_t *stateflow_msg));

/**
 * @name    EHSF_Step
 * @brief   状态机执行一个步进周期
 * @param stateflow     状态机结构体地址
 * @return  void
 * @note    无
 */
void EHSF_Step(stateflow_s_t *stateflow);

#endif /* __STATEFLOW_H_ */
