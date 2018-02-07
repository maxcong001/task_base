#include "task_base/include.hpp"
#define TASK1 "TASK1"
#define TASK2 "TASK2"
int i = 0;

class task_example_1 : public task_base
{
  public:
    task_example_1(std::string name) : task_base(name)
    {
        _name = name;
    }
    virtual bool on_message(TASK_MSG task_msg)
    {
        int tmp = 0;
        __LOG(debug, "receive message with type: " << static_cast<int>(task_msg.type));

        tmp = TASK_ANY_CAST<std::uint32_t>(task_msg.seq_id);
        __LOG(debug, "receive message with seqid : " << tmp);

#if 0
        if (tmp == 50)
        {
            __LOG(error, "exit now!!!!!");
            std::exit(EXIT_SUCCESS);
        }
#endif
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        task_manager::instance()->send2task(TASK2, MSG_TYPE::TASK_PUT, nullptr, i++);
        return true;
    }
};
class task_example_2 : public task_base
{

  public:
    task_example_2(std::string name) : task_base(name)
    {
        _name = name;
    }
    virtual bool on_message(TASK_MSG task_msg)
    {
        int tmp = 0;
        __LOG(debug, "receive message with type: " << static_cast<int>(task_msg.type));

        tmp = TASK_ANY_CAST<std::uint32_t>(task_msg.seq_id);
        __LOG(debug, "receive message with seqid : " << tmp);

#if 0
        if (tmp == 50)
        {
            __LOG(error, "exit now!!!!!");
            std::exit(EXIT_SUCCESS);
        }
#endif
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        task_manager::instance()->send2task(TASK1, MSG_TYPE::TASK_PUT, nullptr, i++);
        return true;
    }
};
int main()
{

    set_log_level(logger_iface::log_level::debug);

    auto ins = task_manager::instance();
    std::shared_ptr<task_base> example_1 = std::shared_ptr<task_example_1>(new task_example_1(std::string("TASK1")));
    //task_ptr_t example_1 = std::make_shared<task_example_1>(std::string("TASK1"));
    ins->add_tasks(example_1);
    std::shared_ptr<task_base> example_2 = std::shared_ptr<task_example_2>(new task_example_2(std::string("TASK2")));
    //task_ptr_t example_2 = std::make_shared<task_example_1>(std::string("TASK2"));
    ins->add_tasks(example_2);

    ins->init(false);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    ins->send2task(TASK1, MSG_TYPE::TASK_DEL, 10);
    std::this_thread::sleep_for(std::chrono::seconds(20));
    __LOG(error, "example exit!1");
    return 0;
}
