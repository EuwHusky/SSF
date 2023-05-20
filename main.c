#include "sf_stateflow.h"

bool det(state_flow_message_box_s_t *stateflow_msg)
{
    if (SF_MSG->test == 5)
        return true;

    return false;
}
bool det1(state_flow_message_box_s_t *stateflow_msg)
{
    if (SF_MSG->test == 1)
        return true;

    return false;
}
bool det2(state_flow_message_box_s_t *stateflow_msg)
{
    if (SF_MSG->test == 1)
        return true;

    return false;
}
void happy(state_flow_message_box_s_t *stateflow_msg)
{
    SF_MSG->test++;
    printf("状态1\r\n");
    printf("%d\r\n", SF_MSG->test);
}
void happyhappy(state_flow_message_box_s_t *stateflow_msg)
{
    SF_MSG->test--;
    printf("状态2\r\n");
    printf("%d\r\n", SF_MSG->test);
}
void happyhappyhappy(state_flow_message_box_s_t *stateflow_msg)
{
    SF_MSG->test += 6;
    printf("状态3\r\n");
    printf("%d\r\n", SF_MSG->test);
}
void hahahahahaha(state_flow_message_box_s_t *stateflow_msg)
{
    printf("我来力哈哈哈哈哈\r\n");
}
void wuwuwuwuwuwu(state_flow_message_box_s_t *stateflow_msg)
{
    printf("啊我寄了呜呜呜呜\r\n");
}

int main()
{
    state_flow_s_t test_state_flow = {0};
    EHSF_Init(&test_state_flow, TEST_1);

    EHSF_CreateState(&test_state_flow, TEST_1, 1, false, hahahahahaha, happy, NULL);
    EHSF_StateAddExitEvent(&test_state_flow, TEST_1, TEST_2, 0, det);

    EHSF_CreateState(&test_state_flow, TEST_2, 2, false, NULL, happyhappy, wuwuwuwuwuwu);
    EHSF_StateAddExitEvent(&test_state_flow, TEST_2, TEST_1, 2, det1);
    EHSF_StateAddExitEvent(&test_state_flow, TEST_2, TEST_3, 1, det2);

    EHSF_CreateState(&test_state_flow, TEST_3, 0, false, NULL, happyhappyhappy, NULL);

    while (1)
    {
        EHSF_Step(&test_state_flow);
        Sleep(500);
    }
}
