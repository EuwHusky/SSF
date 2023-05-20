#include "eh_stateflow.h"

static void stateflow_execute(stateflow_s_t *stateflow);
static void stateflow_guard(stateflow_s_t *stateflow);
static void stateflow_switch(stateflow_s_t *stateflow);
static void stateflow_state_entry_reset(stateflow_s_t *stateflow, stateflow_state_table_e_t next_state);

stateflow_error EHSF_Init(stateflow_s_t *stateflow, stateflow_state_table_e_t initial_state)
{
    // 参数检查
    if ((initial_state == STATE_NULL) || (initial_state == NUM_OF_STATE))
        return stateflow->status = STATEFLOW_INIT_INPUT_ERROR, stateflow->status;

    // 为状态创建空间
    stateflow->state_list = NULL;
    stateflow->state_list = (stateflow_state_s_t *)malloc(NUM_OF_STATE * sizeof(stateflow_state_s_t));
    if (stateflow->state_list == NULL)
        return stateflow->status = STATEFLOW_INIT_MALLOC_ERROR, stateflow->status;

    // 设置系统初始状态
    stateflow->now_state = initial_state;

    // 初始化系统步进时钟
    stateflow->step_clock = 0;

    return stateflow->status = OK, stateflow->status;
}

stateflow_error EHSF_CreateState(stateflow_s_t *stateflow, stateflow_state_table_e_t state_name,
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
    stateflow->state_list[state_name].uptime = 0;

    return stateflow->status = OK, stateflow->status;
}

stateflow_error EHSF_StateAddExitEvent(stateflow_s_t *stateflow, stateflow_state_table_e_t state_name,
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

void EHSF_Step(stateflow_s_t *stateflow)
{
    // 执行
    stateflow_execute(stateflow);

    // 检测
    stateflow_guard(stateflow);

    // 切换
    stateflow_switch(stateflow);

    // 系统步进时钟更新
    stateflow->step_clock++;
    if (stateflow->step_clock > CLOCK_MAX_LIMIT)
    {
        stateflow->step_clock = CLOCK_MAX_LIMIT;
    }
}

static void stateflow_execute(stateflow_s_t *stateflow)
{
    // 执行状态执行时方法
    if (stateflow->state_list[stateflow->now_state].during != NULL)
        stateflow->state_list[stateflow->now_state].during(&stateflow->message_box);

    // 更新状态持续时间
    stateflow->state_list[stateflow->now_state].uptime++;
}
static void stateflow_guard(stateflow_s_t *stateflow)
{
    // 更新所有已设置的出口事件的触发状态
    for (uint8_t i = 0; i < stateflow->state_list[stateflow->now_state].number_of_exit_events_that_instack; i++)
    {
        stateflow->state_list[stateflow->now_state].exit_events[i].event_triggering_state =
            stateflow->state_list[stateflow->now_state].exit_events[i].guard(&stateflow->message_box);
    }
}
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
static void stateflow_state_entry_reset(stateflow_s_t *stateflow, stateflow_state_table_e_t next_state)
{
    if (stateflow->state_list[next_state].is_need_to_reset)
    {
        // 重置此状态的运行数据
        stateflow->state_list[next_state].uptime = 0; // 状态持续时间
    }
}
