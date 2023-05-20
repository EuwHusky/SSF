#include "sf_stateflow.h"

static void sf_state_execute(state_flow_s_t *stateflow);
static void sf_state_guard(state_flow_s_t *stateflow);
static void sf_state_switch(state_flow_s_t *stateflow);

void EHSF_Init(state_flow_s_t *stateflow, state_flow_state_table_e_t initial_state)
{
    // 为状态创建空间
    stateflow->state_list = (state_flow_state_s_t *)malloc(NUM_OF_STATE * sizeof(state_flow_state_s_t));

    // 设置系统初始状态
    stateflow->now_state = initial_state;
}

void EHSF_CreateState(state_flow_s_t *stateflow, state_flow_state_table_e_t state_name, uint8_t number_of_exit_events,
                      bool is_need_to_reset, void (*entry)(state_flow_message_box_s_t *stateflow_msg),
                      void (*during)(state_flow_message_box_s_t *stateflow_msg),
                      void (*exit)(state_flow_message_box_s_t *stateflow_msg))
{
    /*设置状态底层数据*/
    // 状态名称
    stateflow->state_list[state_name].state_name = state_name;

    // 设置状态的出口事件数量
    stateflow->state_list[state_name].number_of_exit_events = number_of_exit_events;
    // 为状态的出口事件创建空间
    stateflow->state_list[state_name].exit_events =
        (state_flow_event_s_t *)malloc(number_of_exit_events * sizeof(state_flow_event_s_t));
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
}

void EHSF_StateAddExitEvent(state_flow_s_t *stateflow, state_flow_state_table_e_t state_name,
                            state_flow_state_table_e_t toward_state, uint8_t priority,
                            bool (*guard)(state_flow_message_box_s_t *stateflow_msg))
{
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
}

void EHSF_Step(state_flow_s_t *stateflow)
{
    // 执行
    sf_state_execute(stateflow);

    // 检测
    sf_state_guard(stateflow);

    // 切换
    sf_state_switch(stateflow);
}

static void sf_state_execute(state_flow_s_t *stateflow)
{
    // 执行状态执行时方法
    if (stateflow->state_list[stateflow->now_state].during != NULL)
        stateflow->state_list[stateflow->now_state].during(&stateflow->message_box);
}
static void sf_state_guard(state_flow_s_t *stateflow)
{
    // 更新所有已设置的出口事件的触发状态
    for (uint8_t i = 0; i < stateflow->state_list[stateflow->now_state].number_of_exit_events_that_instack; i++)
    {
        stateflow->state_list[stateflow->now_state].exit_events[i].event_triggering_state =
            stateflow->state_list[stateflow->now_state].exit_events[i].guard(&stateflow->message_box);
    }
}
static void sf_state_switch(state_flow_s_t *stateflow)
{
    state_flow_state_table_e_t next_state = stateflow->now_state;
    uint8_t temp_priority = 255;

    /*检测所有已设置的出口事件的触发状态及优先级,确定下一状态*/
    for (uint8_t i = 0; i < stateflow->state_list[stateflow->now_state].number_of_exit_events_that_instack; i++)
    {
        if (true == stateflow->state_list[stateflow->now_state].exit_events[i].event_triggering_state)
        {
            // 重置出口事件的触发状态
            stateflow->state_list[stateflow->now_state].exit_events[i].event_triggering_state == false;

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

        // 执行下一状态进入时方法
        if (stateflow->state_list[next_state].entry != NULL)
            stateflow->state_list[next_state].entry(&stateflow->message_box);
    }
}
