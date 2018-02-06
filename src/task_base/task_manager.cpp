#include <task_base/task_manager.hpp>

bool manager_task::on_before_loop()
{
    // start timer for heart beat
    // 1. timer will first check if we had received the hb response
    // 2. if hb response does not come, call the restart fuction
    // after the hb response check
    // 3. send hb to all the tasks
    // 4. clear the hb map

    _timer_mgr.getTimer()->startForever(_hb_itval, [this]() {
        // if this is the first loop and do not have
        thread_local bool first_loop = true;
        if (first_loop)
        {
            //hb_map.clear();
            // first loop, there is no HB response
            // do nothing
            return;
        }
        first_loop = false;

        auto ins = task_manager::instance();
        std::map<std::string, task_ptr_t> tmp_task_map = ins->task_map;
        // first check if the last response returns.
        for (auto hb_iter : hb_map)
        {
            if (hb_iter.second)
            {
                // HB response
            }
            else
            {
                // HB rep does not received
                // 1. get the task ptr_t
                std::string name = hb_iter.first;
                auto iter = tmp_task_map.find(name);
                if (iter == tmp_task_map.end())
                {
                    __LOG(warn, "no such a task named : " << name);
                    return;
                }
                // 2. get the task cb function
                iter->second->restart();
            }
        }

        // clear the hb info
        // do not just call hb_map.clear(); in case add new task
        // clear the HB map
        hb_map.clear();
        for (auto it : tmp_task_map)
        {
            hb_map.insert(std::pair<std::string, bool>(it.first, false));
        }

        // send heartbeat
        ins->send_hb_all();

    });
    return true;
}
bool manager_task::on_message(TASK_MSG msg) 
{
    if (msg.type == MSG_TYPE::TASK_HB)
    {
        hb_map.insert(std::pair<std::string, bool>(TASK_ANY_CAST<std::string>(msg.body), true));
    }
    return true;
}
