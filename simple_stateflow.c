/**
 ******************************************************************************
 * @file    simple_stateflow.c/h
 * @author  Enoky Bertram
 * @version V2.0.0
 * @date    Jun.4.2023
 * @brief   A Simple State Flow Switcher /一个简易状态切换器
 * @note    happyhappyhappy
 ******************************************************************************
 * @example
 * Please see demo. c
 * 请见demo.c
 *
 * @attention
 * 1. Every operation of initializing of the stateflow will return error codes. If the state machine cannot
 *    run, please check immediately if any error codes other than OK are returned during initialization.
 *    状态机初始化的所有操作都会返回错误码，如果状态机运行不了请第一时间检查在初始化时是否有返回任何除了OK以外的错误码。
 *
 * 2. When there is an error in the running result of the stateflow, please check immediately whether the
 *    corresponding method function was added incorrectly during initialization.
 *    当状态机运行结果有误时，请第一时间检查初始化时对应的方法函数是否添加有误。
 *
 * 3. When a certain state cannot be entered, please refer to the previous item, then check if the number of
 *    exit events in the previous state is configured correctly. The same goes for not being able to exit
 *    current state.
 *    当某一状态无法进入时，请参见上一条，然后检查该状态的上一状态的出口事件数量是否配置正确。无法退出状态同理。
 ******************************************************************************
 */

#include "simple_stateflow.h"

/**
 * @name    stateflow_execute
 * @brief   stateflow executing the current state
 * @param   stateflow   stateflow structure pointer
 * @return  void
 * @note    State internal call
 */
/**
 * @name    stateflow_execute
 * @brief   状态机 执行
 * @param   stateflow   状态机结构体地址
 * @return  void
 * @note    状态内部调用
 */
static void stateflow_execute(stateflow_s_t *stateflow);

/**
 * @name    stateflow_guard
 * @brief   stateflow detect event triggering status
 * @param   stateflow   stateflow structure pointer
 * @return  void
 * @note    State internal call
 */
/**
 * @name    stateflow_guard
 * @brief   状态机 检测事件触发状态
 * @param   stateflow   stateflow structure pointer
 * @return  void
 * @note    状态内部调用
 */
static void stateflow_guard(stateflow_s_t *stateflow);

/**
 * @name    stateflow_switch
 * @brief   stateflow state switching
 * @param   stateflow   stateflow structure pointer
 * @return  void
 * @note    State internal call
 */
/**
 * @name    stateflow_switch
 * @brief   状态机 状态切换
 * @param   stateflow   状态机结构体地址
 * @return  void
 * @note    状态内部调用
 */
static void stateflow_switch(stateflow_s_t *stateflow);

/**
 * @name    stateflow_state_entry_reset
 * @brief   reset the next entered state
 * @param   stateflow   stateflow structure pointer
 * @param   next_state  next state
 * @return  void
 * @note    State internal call
 */
/**
 * @name    stateflow_state_entry_reset
 * @brief   重置下一个进入的状态
 * @param   stateflow   状态机结构体地址
 * @param   next_state  下一个状态
 * @return  void
 * @note    状态内部调用
 */
static void stateflow_state_entry_reset(stateflow_s_t *stateflow, stateflow_state_table_e_t next_state);

/**
 * @name    SSF_Init
 * @brief   stateflow initialization
 * @param stateflow     stateflow structure pointer
 * @param initial_state initial state of stateflow system
 * @return  stateflow_error
 * @example SSF_Init(&test_state_flow, TEST_1);
 * @note    none
 */
/**
 * @name    SSF_Init
 * @brief   状态机初始化
 * @param stateflow     状态机结构体地址
 * @param initial_state 状态机系统初始状态
 * @return  stateflow_error
 * @example SSF_Init(&test_state_flow, TEST_1);
 * @note    无
 */
stateflow_error SSF_Init(stateflow_s_t *stateflow, stateflow_state_table_e_t initial_state)
{
    // 参数检查
    if ((initial_state == STATE_NULL) || (initial_state == NUM_OF_STATE))
        return stateflow->status = STATEFLOW_INIT_INPUT_ERROR, stateflow->status;

    // 为状态创建空间
    stateflow->state_list = NULL;
    stateflow->state_list = (stateflow_state_s_t *)malloc(NUM_OF_STATE * sizeof(stateflow_state_s_t));
    if (stateflow->state_list == NULL)
        return stateflow->status = STATEFLOW_INIT_STATELIST_MALLOC_ERROR, stateflow->status;
    memset(stateflow->state_list, 0, NUM_OF_STATE * sizeof(stateflow_state_s_t));

    // 设置系统初始状态
    stateflow->now_state = initial_state;

    // 初始化系统步进时钟
    stateflow->message_box.step_clock = 0;

    // 初始化状态持续时间
    stateflow->message_box.uptime = NULL;
    stateflow->message_box.uptime = (uint32_t *)malloc(NUM_OF_STATE * sizeof(uint32_t));
    if (stateflow->message_box.uptime == NULL)
        return stateflow->status = STATEFLOW_INIT_UPTIME_MALLOC_ERROR, stateflow->status;
    memset(stateflow->message_box.uptime, 0, NUM_OF_STATE * sizeof(uint32_t));

    return stateflow->status = OK, stateflow->status;
}

/**
 * @name    SSF_CreateState
 * @brief   create a state
 * @param stateflow             stateflow structure pointer
 * @param state_name            the name/enumeration value for this state
 * @param number_of_exit_events the total number of exit events owned by this state
 * @param is_need_to_reset      does this state need to be reset upon entry
 * @param entry                 Method when entering this state
 * @param during                Method when executing in this state
 * @param exit                  Method when exiting this state
 * @return  stateflow_error
 * @example SSF_CreateState(&test_state_flow, TEST_1, 1, false, entry_test_1, during_test_1, STATE_METHOD_NULL);
 * @note    none
 */
/**
 * @name    SSF_CreateState
 * @brief   创建一个状态
 * @param stateflow             状态机结构体地址
 * @param state_name            此状态的名称/枚举值
 * @param number_of_exit_events 此状态拥有的出口事件总数
 * @param is_need_to_reset      此状态在进入时是否需要重置
 * @param entry                 此状态进入时方法
 * @param during                此状态执行时方法
 * @param exit                  此状态退出时方法
 * @return  stateflow_error
 * @example SSF_CreateState(&test_state_flow, TEST_1, 1, false, entry_test_1, during_test_1, STATE_METHOD_NULL);
 * @note    无
 */
stateflow_error SSF_CreateState(stateflow_s_t *stateflow, stateflow_state_table_e_t state_name,
                                uint8_t number_of_exit_events, bool is_need_to_reset,
                                void (*entry)(stateflow_message_box_s_t *stateflow_msg),
                                void (*during)(stateflow_message_box_s_t *stateflow_msg),
                                void (*exit)(stateflow_message_box_s_t *stateflow_msg))
{
    /*状态机运行状态检查*/
    if (stateflow->status != OK)
        return stateflow->status;

    /*参数检查*/
    if ((state_name == STATE_NULL) || (state_name == NUM_OF_STATE))
        return stateflow->status = STATE_CREATE_INPUT_ERROR, stateflow->status;

    /*设置状态底层数据*/
    // 状态名称
    stateflow->state_list[state_name].state_name = state_name;

    // 设置状态的出口事件数量
    stateflow->state_list[state_name].number_of_exit_events = number_of_exit_events;
    // 为状态的出口事件创建空间
    stateflow->state_list[state_name].exit_events = NULL;
    stateflow->state_list[state_name].exit_events =
        (stateflow_event_s_t *)malloc(number_of_exit_events * sizeof(stateflow_event_s_t));
    if (stateflow->state_list[state_name].exit_events == NULL)
        return stateflow->status = STATE_CREATE_MALLOC_ERROR, stateflow->status;
    // 初始化已设置的状态出口事件数量
    stateflow->state_list[state_name].number_of_exit_events_that_instack = 0;

    // 设置状态进入时方法
    stateflow->state_list[state_name].entry = entry;
    // 设置状态进入时方法
    stateflow->state_list[state_name].during = during;
    // 设置状态进入时方法
    stateflow->state_list[state_name].exit = exit;

    /*设置状态运行数据*/
    stateflow->state_list[state_name].is_need_to_reset = is_need_to_reset; // 设置进入状态时是否需要重置状态运行数据

    return stateflow->status = OK, stateflow->status;
}

/**
 * @name    SSF_StateAddExitEvent
 * @brief   add an exit event for this state
 * @param stateflow     stateflow structure pointer
 * @param state_name    name/enumeration value of the state to which it belongs
 * @param toward_state  the state pointed to by this exit event
 * @param priority      the triggering priority of this exit event
 * @param guard         the detection method for this exit event
 * @return  stateflow_error
 * @example SSF_StateAddExitEvent(&test_state_flow, TEST_1, TEST_2, 0, guard_test_1_to_test_2);
 * @note    none
 */
/**
 * @name    SSF_StateAddExitEvent
 * @brief   为状态添加一个出口事件
 * @param stateflow     状态机结构体地址
 * @param state_name    所属状态的名称/枚举值
 * @param toward_state  此出口事件指向的状态
 * @param priority      此出口事件的触发优先级
 * @param guard         此出口事件的检测方法
 * @return  stateflow_error
 * @example SSF_StateAddExitEvent(&test_state_flow, TEST_1, TEST_2, 0, guard_test_1_to_test_2);
 * @note    无
 */
stateflow_error SSF_StateAddExitEvent(stateflow_s_t *stateflow, stateflow_state_table_e_t state_name,
                                      stateflow_state_table_e_t toward_state, uint8_t priority,
                                      bool (*guard)(stateflow_message_box_s_t *stateflow_msg))
{
    // 状态机运行状态检查
    if (stateflow->status != OK)
        return stateflow->status;

    // 事件数量检查
    if (stateflow->state_list[state_name].number_of_exit_events_that_instack ==
        stateflow->state_list[state_name].number_of_exit_events)
        return EXIT_EVENT_ADD_NUM_ERROR;

    // 参数检查
    if ((state_name == STATE_NULL) || (state_name == NUM_OF_STATE) || (toward_state == STATE_NULL) ||
        (toward_state == NUM_OF_STATE))
        return stateflow->status = EXIT_EVENT_ADD_INPUT_ERROR, stateflow->status;

    // 触发状态初始化
    stateflow->state_list[state_name]
        .exit_events[stateflow->state_list[state_name].number_of_exit_events_that_instack]
        .event_triggering_state = false;

    // 设置出口事件所指向的状态
    stateflow->state_list[state_name]
        .exit_events[stateflow->state_list[state_name].number_of_exit_events_that_instack]
        .toward_state = toward_state;

    // 设置出口事件优先级
    stateflow->state_list[state_name]
        .exit_events[stateflow->state_list[state_name].number_of_exit_events_that_instack]
        .priority = priority;

    // 设置出口事件检测方法
    stateflow->state_list[state_name]
        .exit_events[stateflow->state_list[state_name].number_of_exit_events_that_instack]
        .guard = guard;

    // 已设置的状态出口事件数量加一
    stateflow->state_list[state_name].number_of_exit_events_that_instack++;

    return stateflow->status = OK, stateflow->status;
}

/**
 * @name    SSF_Step
 * @brief   stateflow executes a step cycle
 * @param stateflow     stateflow structure pointer
 * @return  void
 * @example SSF_Step(&test_state_flow);
 * @note    none
 */
/**
 * @name    SSF_Step
 * @brief   状态机执行一个步进周期
 * @param stateflow     状态机结构体地址
 * @return  void
 * @example SSF_Step(&test_state_flow);
 * @note    无
 */
void SSF_Step(stateflow_s_t *stateflow)
{
    // 执行
    stateflow_execute(stateflow);

    // 检测
    stateflow_guard(stateflow);

    // 切换
    stateflow_switch(stateflow);

    // 系统步进时钟更新
    stateflow->message_box.step_clock++;
    if (stateflow->message_box.step_clock > CLOCK_MAX_LIMIT)
    {
        stateflow->message_box.step_clock = CLOCK_MAX_LIMIT;
    }
}

/**
 * @name    stateflow_execute
 * @brief   stateflow executing the current state
 * @param   stateflow   stateflow structure pointer
 * @return  void
 * @note    State internal call
 */
/**
 * @name    stateflow_execute
 * @brief   状态机 执行
 * @param   stateflow   状态机结构体地址
 * @return  void
 * @note    状态内部调用
 */
static void stateflow_execute(stateflow_s_t *stateflow)
{
    // 执行状态执行时方法
    if (stateflow->state_list[stateflow->now_state].during != NULL)
        stateflow->state_list[stateflow->now_state].during(&stateflow->message_box);

    // 更新状态持续时间
    stateflow->message_box.uptime[stateflow->now_state]++;
}

/**
 * @name    stateflow_guard
 * @brief   stateflow detect event triggering status
 * @param   stateflow   stateflow structure pointer
 * @return  void
 * @note    State internal call
 */
/**
 * @name    stateflow_guard
 * @brief   状态机 检测事件触发状态
 * @param   stateflow   stateflow structure pointer
 * @return  void
 * @note    状态内部调用
 */
static void stateflow_guard(stateflow_s_t *stateflow)
{
    // 更新所有已设置的出口事件的触发状态
    for (uint8_t i = 0; i < stateflow->state_list[stateflow->now_state].number_of_exit_events_that_instack; i++)
    {
        stateflow->state_list[stateflow->now_state].exit_events[i].event_triggering_state =
            stateflow->state_list[stateflow->now_state].exit_events[i].guard(&stateflow->message_box);
    }
}

/**
 * @name    stateflow_switch
 * @brief   stateflow state switching
 * @param   stateflow   stateflow structure pointer
 * @return  void
 * @note    State internal call
 */
/**
 * @name    stateflow_switch
 * @brief   状态机 状态切换
 * @param   stateflow   状态机结构体地址
 * @return  void
 * @note    状态内部调用
 */
static void stateflow_switch(stateflow_s_t *stateflow)
{
    stateflow_state_table_e_t next_state = stateflow->now_state;
    uint8_t temp_priority = 255;

    /*检测所有已设置的出口事件的触发状态及优先级,确定下一状态*/
    for (uint8_t i = 0; i < stateflow->state_list[stateflow->now_state].number_of_exit_events_that_instack; i++)
    {
        if (true == stateflow->state_list[stateflow->now_state].exit_events[i].event_triggering_state)
        {
            // 重置出口事件的触发状态
            stateflow->state_list[stateflow->now_state].exit_events[i].event_triggering_state = false;

            // 若仅有一个事件被触发，则将该事件的指向状态设为下一状态
            // 若多个事件被触发，则将所有触发的事件中最高优先级事件的指向状态设为下一状态
            if (next_state == stateflow->now_state) // 第一个触发的事件，不进行优先级判定
            {
                next_state = stateflow->state_list[stateflow->now_state].exit_events[i].toward_state;
                temp_priority = stateflow->state_list[stateflow->now_state].exit_events[i].priority;
            }
            else /*(next_state != stateflow->now_state)*/ // 后续触发的事件，进行优先级判定
            {
                if (stateflow->state_list[stateflow->now_state].exit_events[i].priority < temp_priority)
                {
                    next_state = stateflow->state_list[stateflow->now_state].exit_events[i].toward_state;
                    temp_priority = stateflow->state_list[stateflow->now_state].exit_events[i].priority;
                }
            }
        }
    }

    /*状态切换*/
    if (next_state != stateflow->now_state)
    {
        // 执行当前状态退出时方法
        if (stateflow->state_list[stateflow->now_state].exit != NULL)
            stateflow->state_list[stateflow->now_state].exit(&stateflow->message_box);

        //  更新系统状态记录
        stateflow->last_state = stateflow->now_state;
        // 更新系统当前状态为已触发的出口事件所指向的状态
        stateflow->now_state = next_state;

        // 重置下一状态运行数据
        stateflow_state_entry_reset(stateflow, next_state);
        // 执行下一状态进入时方法
        if (stateflow->state_list[next_state].entry != NULL)
            stateflow->state_list[next_state].entry(&stateflow->message_box);
    }
}

/**
 * @name    stateflow_state_entry_reset
 * @brief   reset the next entered state
 * @param   stateflow   stateflow structure pointer
 * @param   next_state  next state
 * @return  void
 * @note    State internal call
 */
/**
 * @name    stateflow_state_entry_reset
 * @brief   重置下一个进入的状态
 * @param   stateflow   状态机结构体地址
 * @param   next_state  下一个状态
 * @return  void
 * @note    状态内部调用
 */
static void stateflow_state_entry_reset(stateflow_s_t *stateflow, stateflow_state_table_e_t next_state)
{
    if (stateflow->state_list[next_state].is_need_to_reset)
    {
        // 重置此状态的运行数据
        stateflow->message_box.uptime[stateflow->now_state] = 0; // 状态持续时间
        // stateflow->state_list[next_state].uptime = 0;            // 状态持续时间
    }
}
