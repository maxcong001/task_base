#include <task_base/util.hpp>
void evfdCallback(int fd, short event, void *args);
using task_func = std::function<void(TASK_ANY msg)>;
class task_base
{
  public:
    bool init(bool new_thread)
    {

        _evfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
        if (_evfd < 0)
        {
            __LOG(error, "!!!!!!!!create event fd fail!");
            return false;
        }
        __LOG(debug, "init task with ID :" << _evfd);
        // start eventfd server
        try
        {
            _event_server = std::make_shared<translib::EventFdServer>(_loop.ev(), _evfd, evfdCallback, this);
        }
        catch (std::exception &e)
        {
            __LOG(error, "!!!!!!!!!!!!exception happend when trying to create event fd server, info :" << e.what());
            return false;
        }
        _loop.start(new_thread);
        return true;
    }

    // set the callback function for evnet coming
    bool set_msg_cb(task_func cb_fun)
    {
        _task_func = cb_fun;
        return true;
    }
    void process_msg(uint64_t num)
    {
        // actually process all the messages
        swap(_task_queue, _tmp_task_queue);
        while (_tmp_task_queue.size() != 0)
        {
            auto tmp = _tmp_task_queue.front();
            _task_func(tmp);
            _tmp_task_queue.pop();
        }
    }
    void in_queue(TASK_ANY msg)
    {
        std::lock_guard<std::mutex> lck(mtx);
        _task_queue.emplace(msg);
    }
    int get_id()
    {
        return _evfd;
    }
    std::mutex mtx;
    TASK_QUEUE _task_queue;
    TASK_QUEUE _tmp_task_queue;
    //   int _task_id;
    // int event fd
    int _evfd;
    task_func _task_func;
    translib::Loop _loop;
    std::shared_ptr<translib::EventFdServer> _event_server;
};

typedef std::shared_ptr<task_base> task_ptr_t;

class task_mamager
{
  public:
    static task_mamager *instance()
    {
        static task_mamager *ins = new task_mamager();
        return ins;
    }
    bool send2task(std::string name, TASK_ANY msg)
    {
        auto it = task_map.find(name);
        if (it == task_map.end())
        {
            __LOG(warn, "no such a task named : " << name);
            return false;
        }
        it->second->in_queue(msg);
        // send eventfd message
        uint64_t one;
        int ret = write(it->second->get_id(), &one, sizeof(one));
        if (ret != sizeof(one))
        {
            __LOG(error, "write event fd : " << it->second->get_id() << " fail");
            return false;
        }
        return true;
    }
    bool add_tasks(std::string name, task_ptr_t task)
    {
        // no log needed here, this is called only
        // when init
        task_map[name] = task;
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
    bool add_tasks(std::string name, task_func cb_fun)
    {
        task_ptr_t tmp_task_ptr_t = std::make_shared<task_base>();
        tmp_task_ptr_t->set_msg_cb(cb_fun);
        add_tasks(name, tmp_task_ptr_t);
        return true;
    }

    bool init(bool _poll = true)
    {
        if (task_map.find(TASK0) == task_map.end())
        {
            __LOG(error, "!!!!!!!!!at lease task0 should be provided!!");
            return false;
        }
        if (_poll)
        {
            for (auto it : task_map)
            {
                if (it.first.compare(TASK0))
                {
                    it.second->init(true);
                }
                else
                {
                    __LOG(debug, "task0 do not need init, it will init later");
                }
            }
            task_map[TASK0]->init(false);
        }
        else
        {
            for (auto it : task_map)
            {
                it.second->init(true);
            }
        }
        return true;
    }
    std::map<std::string, task_ptr_t> task_map;
};
