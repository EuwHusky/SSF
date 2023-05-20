#include "sf_stateflow.h"

bool det(stateflow_message_box_s_t *stateflow_msg)
{
    if (EHSF_MSG->test == 5)
        return true;

    return false;
}
bool det1(stateflow_message_box_s_t *stateflow_msg)
{
    if (EHSF_MSG->test == 1)
        return true;

    return false;
}
bool det2(stateflow_message_box_s_t *stateflow_msg)
{
    if (EHSF_MSG->test == 1)
        return true;

    return false;
}
void happy(stateflow_message_box_s_t *stateflow_msg)
{
    EHSF_MSG->test++;
    printf("状态1\r\n");
    printf("%d\r\n", EHSF_MSG->test);
}
void happyhappy(stateflow_message_box_s_t *stateflow_msg)
{
    EHSF_MSG->test--;
    printf("状态2\r\n");
    printf("%d\r\n", EHSF_MSG->test);
}
void happyhappyhappy(stateflow_message_box_s_t *stateflow_msg)
{
    EHSF_MSG->test += 6;
    printf("状态3\r\n");
    printf("%d\r\n", EHSF_MSG->test);
}
void hahahahahaha(stateflow_message_box_s_t *stateflow_msg)
{
    printf("我来力哈哈哈哈哈\r\n");
}
void wuwuwuwuwuwu(stateflow_message_box_s_t *stateflow_msg)
{
    printf("啊我寄了呜呜呜呜\r\n");
}

int main()
{
    stateflow_s_t test_state_flow = {0};

    printf("%d\r\n", EHSF_Init(&test_state_flow, TEST_1));

    printf("%d\r\n", EHSF_CreateState(&test_state_flow, TEST_1, 1, false, hahahahahaha, happy, STATE_METHOD_NULL));
    printf("%d\r\n", EHSF_StateAddExitEvent(&test_state_flow, TEST_1, TEST_2, 0, det));

    printf("%d\r\n", EHSF_CreateState(&test_state_flow, TEST_2, 2, true, STATE_METHOD_NULL, happyhappy, wuwuwuwuwuwu));
    printf("%d\r\n", EHSF_StateAddExitEvent(&test_state_flow, TEST_2, TEST_1, 0, det1));
    printf("%d\r\n", EHSF_StateAddExitEvent(&test_state_flow, TEST_2, TEST_3, 1, det2));

    printf("%d\r\n",
           EHSF_CreateState(&test_state_flow, TEST_3, 0, false, STATE_METHOD_NULL, happyhappyhappy, STATE_METHOD_NULL));

    while (1)
    {
        printf("=========================\r\n");
        EHSF_Step(&test_state_flow);
        printf("this state's uptime:%d\r\n", test_state_flow.state_list[test_state_flow.now_state].uptime);
        printf("=========================\r\n");
        Sleep(500);
    }
}
