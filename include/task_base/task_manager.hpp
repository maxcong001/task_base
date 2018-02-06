#include <task_base/util.hpp>

typedef std::shared_ptr<task_base> task_ptr_t;
void tsk0_func(TASK_MSG task_msg);
class task_mamager : _hb_itval(1000), _seq_id(0)
{
  public:
    static task_mamager *instance()
    {
        static task_mamager *ins = new task_mamager();
        return ins;
    }
    bool send2task(std::string name, MSG_TYPE type, TASK_ANY body, std::uint32_t seq_id = 0)
    {
        // actually here need a lock here
        // but if you start and tasks and do not add more tasks
        // the lock is not needed
        auto it = task_map.find(name);
        if (it == task_map.end())
        {
            __LOG(warn, "no such a task named : " << name);
            return false;
        }
        TASK_MSG msg;
        msg.type = type;
        msg.body = body;
        msg.seq_id = seq_id;
        it->second->in_queue(msg);
        // send eventfd message
        uint64_t one = 1;
        int ret = write(it->second->get_id(), &one, sizeof(one));
        if (ret != sizeof(one))
        {
            __LOG(error, "write event fd : " << it->second->get_id() << " fail");
            return false;
        }
        else
        {
            __LOG(debug, "send to eventfd : " << it->second->get_id());
        }
        return true;
    }
    // this should be called by taask0
    // send HB message with body equals to remote task name
    bool send_hb_all()
    {
        for (auto it : task_map)
        {
            if (it.first.compare(TASK0))
            {
            }
            else
            {
                send2task(it.first, TASK_HB, it.first, _seq_id);
                _seq_id++;
            }
        }
    }
    bool add_tasks(std::string name, task_ptr_t task)
    {
        // no lock needed here, this is called only
        // when init
        task_map[name] = task;
        return true;
    }
    bool del_tasks(std::string name)
    {
        task_map.erase(name);
        return true;
    }
    int get_task_id(std::string name)
    {
        auto it = task_map.find(name);
        if (it == task_map.end())
        {
            __LOG(warn, "no such a task named : " << name);
            return -1;
        }
        return it->second->get_id();
    }
    // when you add task at run time, please set init to true,
    // else set init to false
    bool add_tasks(std::string name, task_func cb_fun, bool init = false)
    {
        task_ptr_t tmp_task_ptr_t = std::make_shared<task_base>();
        tmp_task_ptr_t->set_msg_cb(cb_fun);
        add_tasks(name, tmp_task_ptr_t);
        if (init)
        {
            tmp_task_ptr_t->init(true);
        }
        return true;
    }
    // note  if _poll is set to true, it will hang here and wait for incoming message
    bool init(bool _poll = true)
    {
        add_tasks(TASK0, tsk0_func);
        if (task_map.find(TASK0) == task_map.end())
        {
            __LOG(error, "!!!!!!!!!at lease task0 should be provided!!");
            return false;
        }

        for (auto it : task_map)
        {
            if (!it.first.compare(TASK0))
            {
                // this is task0
                // 1. heartbeat related
                it.second._timer_mgr.getTimer()->startForever(_hb_itval, [this]() {
                    // if this is the first loop and do not have
                    thread_local bool first_loop = true;
                    if (first_loop)
                    {
                        // first loop, there is no HB response
                        // do nothing
                        return;
                    }
                    first_loop = false;
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
                            auto iter = task_map.find(name);
                            if (iter == task_map.end())
                            {
                                __LOG(warn, "no such a task named : " << name);
                                return false;
                            }
                            // 2. get the task cb function
                            task_func tmp_cb = iter->second->get_msg_cb();
                            // 3. delete the task
                            // note: this is called when timer fired, actually not in the loop of task_map
                            // So we can delete it here not break the iter
                            task_map.erase(name);
                            // 4. add the task again
                            add_tasks(name, tmp_cb);
                            // 5. init the task
                            auto new_task_iter = task_map.find(name);
                            if (new_task_iter == task_map.end())
                            {
                                __LOG(warn, "no such a task named : " << name);
                                return false;
                            }
                            new_task_iter->init(true);
                        }
                    }
                    // clear the hb info
                    hb_map.clear();
                    // send heartbeat
                    send_hb_all();
                });
                // 2. init the task0
                if (_poll)
                {
                    it.second->init(false);
                }
                else
                {
                    it.second->init(true);
                }
            }
            else
            {
                // this is not task 0, start a new task
                it.second->init(true);
            }
        }
        return true;
    }

    std::map<std::string, task_ptr_t> task_map;
    // heart beat interval
    std::uint32_t _hb_itval;
    std::atomic<std::uint32_t> _seq_id;
};
