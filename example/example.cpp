
#include "task_base/task_base.hpp"
#define TASK1 "task1"
#define TASK2 "task2"
int i = 0;
void tsk0_func(TASK_ANY task_msg)
{
    if (task_msg.type() == typeid(int))
    {
        int tmp = TASK_ANY_CAST<int>(task_msg);
        __LOG(debug, "receive message: " << tmp);
        if (tmp == 26)
        {
            __LOG(error, "exit now!!!!!");
            std::exit(EXIT_SUCCESS);
        }
    }
    else
    {
        __LOG(error, "not support type");
    }

    task_mamager::instance()->send2task(TASK1, i++);
    std::this_thread::sleep_for(std::chrono::seconds(1));
#if 0
    
    if (i > 20)
    {
        std::exit(EXIT_SUCCESS);
    }
#endif
}

void tsk1_func(TASK_ANY task_msg)
{
    if (task_msg.type() == typeid(int))
    {
        __LOG(debug, "receive message: " << TASK_ANY_CAST<int>(task_msg));
    }
    else
    {
        __LOG(error, "not support type");
    }
    task_mamager::instance()->send2task(TASK2, i++);
    std::this_thread::sleep_for(std::chrono::seconds(1));
}
void tsk2_func(TASK_ANY task_msg)
{
    if (task_msg.type() == typeid(int))
    {
        __LOG(debug, "receive message: " << TASK_ANY_CAST<int>(task_msg));
    }
    else
    {
        __LOG(error, "not support type");
    }
    task_mamager::instance()->send2task(TASK0, i++);
    std::this_thread::sleep_for(std::chrono::seconds(1));
}
int main()
{
    set_log_level(logger_iface::log_level::debug);

    auto ins = task_mamager::instance();
    ins->add_tasks(TASK0, tsk0_func);
    ins->add_tasks(TASK1, tsk1_func);
    ins->add_tasks(TASK2, tsk2_func);

    int timerID001 = 100;
    translib::Timer::ptr_p timer001 = translib::TimerManager::instance()->getTimer(&timerID001);
    timer001->startOnce(1000, [] {
        auto ins = task_mamager::instance();
        ins->send2task(TASK0, 10);

    });


    ins->init();
}
