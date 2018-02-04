
#include "task_base/task_base.hpp"
#define TASK1 "task1"
#define TASK2 "task2"
int i = 0;
void tsk0_func(TASK_MSG task_msg)
{
    int tmp = 0;
    __LOG(debug, "receive message with type: " << static_cast<int>(task_msg.type));

    if (task_msg.body.type() == typeid(int))
    {
        tmp = TASK_ANY_CAST<int>(task_msg.body);
        __LOG(debug, "receive message with body : " << tmp);
    }
    else
    {
        __LOG(error, "not support type");
    }

    if (tmp == 26)
    {
        __LOG(error, "exit now!!!!!");
        std::exit(EXIT_SUCCESS);
    }

    task_mamager::instance()->send2task(TASK1, MSG_TYPE::TASK_PUT, i++);
    std::this_thread::sleep_for(std::chrono::seconds(1));
#if 0
    
    if (i > 20)
    {
        std::exit(EXIT_SUCCESS);
    }
#endif
}
void tsk1_func(TASK_MSG task_msg)
{
    int tmp = 0;
    __LOG(debug, "receive message with type: " << static_cast<int>(task_msg.type));

    if (task_msg.body.type() == typeid(int))
    {
        tmp = TASK_ANY_CAST<int>(task_msg.body);
        __LOG(debug, "receive message with body : " << tmp);
    }
    else
    {
        __LOG(error, "not support type");
    }

    task_mamager::instance()->send2task(TASK2, MSG_TYPE::TASK_PUT, i++);
    std::this_thread::sleep_for(std::chrono::seconds(1));
}
void tsk2_func(TASK_MSG task_msg)
{
    int tmp = 0;
    __LOG(debug, "receive message with type: " << static_cast<int>(task_msg.type));

    if (task_msg.body.type() == typeid(int))
    {
        tmp = TASK_ANY_CAST<int>(task_msg.body);
        __LOG(debug, "receive message with body : " << tmp);
    }
    else
    {
        __LOG(error, "not support type");
    }

    task_mamager::instance()->send2task(TASK0, MSG_TYPE::TASK_PUT, i++);
    std::this_thread::sleep_for(std::chrono::seconds(1));
}
int main()
{
    set_log_level(logger_iface::log_level::debug);

    auto ins = task_mamager::instance();
    ins->add_tasks(TASK0, tsk0_func);
    ins->add_tasks(TASK1, tsk1_func);
    ins->add_tasks(TASK2, tsk2_func);



    ins->init(false);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    ins->send2task(TASK0, MSG_TYPE::TASK_DEL, 10);
    std::this_thread::sleep_for(std::chrono::seconds(20));
}
