#include "simple_stateflow.h"

void entry_test_1(stateflow_message_box_s_t *stateflow_msg)
{
    printf("状态进入\r\n");
}
void during_test_1(stateflow_message_box_s_t *stateflow_msg)
{
    SSF_MSG->test++;
    printf("状态1\r\n");
    printf("%d\r\n", SSF_MSG->test);
}
bool guard_test_1_to_test_2(stateflow_message_box_s_t *stateflow_msg)
{
    if (SSF_MSG->test == 5)
        return true;

    return false;
}

void during_test_2(stateflow_message_box_s_t *stateflow_msg)
{
    SSF_MSG->test--;
    printf("状态2\r\n");
    printf("%d\r\n", SSF_MSG->test);
}
void exit_test_2(stateflow_message_box_s_t *stateflow_msg)
{
    printf("状态退出\r\n");
}
bool guard_test_2_to_test_1(stateflow_message_box_s_t *stateflow_msg)
{
    if (SSF_MSG->test == 1)
        return true;

    return false;
}
bool guard_test_2_to_test_3(stateflow_message_box_s_t *stateflow_msg)
{
    if (SSF_MSG->test == 1)
        return true;

    return false;
}

void during_test_3(stateflow_message_box_s_t *stateflow_msg)
{
    SSF_MSG->test += 6;
    printf("状态3\r\n");
    printf("%d\r\n", SSF_MSG->test);
}

int main()
{
    stateflow_s_t test_state_flow = {0};

    printf("%d\r\n", SSF_Init(&test_state_flow, TEST_1));

    printf("%d\r\n",
           SSF_CreateState(&test_state_flow, TEST_1, 1, false, entry_test_1, during_test_1, STATE_METHOD_NULL));
    printf("%d\r\n", SSF_StateAddExitEvent(&test_state_flow, TEST_1, TEST_2, 0, guard_test_1_to_test_2));

    printf("%d\r\n", SSF_CreateState(&test_state_flow, TEST_2, 2, true, STATE_METHOD_NULL, during_test_2, exit_test_2));
    printf("%d\r\n", SSF_StateAddExitEvent(&test_state_flow, TEST_2, TEST_1, 0, guard_test_2_to_test_1));
    printf("%d\r\n", SSF_StateAddExitEvent(&test_state_flow, TEST_2, TEST_3, 1, guard_test_2_to_test_3));

    printf("%d\r\n",
           SSF_CreateState(&test_state_flow, TEST_3, 0, false, STATE_METHOD_NULL, during_test_3, STATE_METHOD_NULL));

    while (1)
    {
        printf("=========================\r\n");
        printf("now state's uptime:%d\r\n", test_state_flow.message_box.uptime[test_state_flow.now_state]);
        SSF_Step(&test_state_flow);
        printf("=========================\r\n");
        Sleep(2000);
    }
}
