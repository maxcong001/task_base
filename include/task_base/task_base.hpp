#include <task_base/util.hpp>

using task_func = std::function<void(TASK_MSG msg)>;
class task_base
{
  public:
    task_base(std::string name) : _hb_itval(1000), _name(name), _evfd(-1), _loop(), _timer_mgr(_loop)
    {
    }
    task_base() = delete;
    ~task_base()
    {
        if (_evfd >= 0)
        {
            close(_evfd);
            _evfd = -1;
        }
    }
    virtual void restart()
    {
        // to do
    }
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
        on_before_loop(this);
        _loop.start(new_thread);
        return true;
    }
    virtual bool on_before_loop(task_base *_this_ptr)
    {
        return true;
    }
    // set the callback function for evnet coming
    virtual bool on_message(TASK_MSG msg) = 0;
    void process_msg(uint64_t num)
    {
        __LOG(debug, "task with id : " << _evfd << " receive message");
        {
            std::lock_guard<std::mutex> lck(mtx);
            // actually process all the messages
            swap(_task_queue, _tmp_task_queue);
        }
        while (_tmp_task_queue.size() != 0)
        {
            auto tmp = _tmp_task_queue.front();
            on_message(tmp);
            _tmp_task_queue.pop();
        }
    }
    void in_queue(TASK_MSG msg)
    {
        __LOG(debug, "inqueue for task with id :" << _evfd);
        std::lock_guard<std::mutex> lck(mtx);
        _task_queue.emplace(msg);
    }
    int get_id()
    {
        return _evfd;
    }
    translib::Loop &get_loop()
    {
        return _loop;
    }

    std::string get_task_name()
    {
        return _name;
    }
    void set_hb_interval(std::uint32_t interval)
    {
        _hb_itval = interval;
    }
    std::uint32_t _hb_itval;
    std::mutex mtx;
    TASK_QUEUE _task_queue;
    TASK_QUEUE _tmp_task_queue;

    //   int _task_id;
    // int event fd
    // task name
    std::string _name;
    int _evfd;

    // note: do not change the sequence of _loop and _event_server
    // _event_server should distructure first!!!!
    translib::Loop _loop;
    std::shared_ptr<translib::EventFdServer> _event_server;
    // timer.
    translib::TimerManager _timer_mgr;
    std::uint32_t _hb_itval;
};

class manager_task : public task_base
{
  public:
    manager_task(std::string name) : _name(TASK0)
    {
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
            auto ins = task_mamager::instance();
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
                        return false;
                    }
                    // 2. get the task cb function
                    task_func tmp_cb = iter->second->restart();
                }
            }

            // send heartbeat
            send_hb_all();
            // clear the hb info
            // do not just call hb_map.clear(); in case add new task
            // clear the HB map
            hb_map.clear();
            for (auto it : tmp_task_map)
            {
                hb_map.insert(std::pair<std::string, bool>(it.first, false));
            }

        });
    }

    virtual bool on_before_loop(task_base *_this_ptr)
    {
        auto ins = task_mamager::instance();
        // send HB message to all
        ins->send_hb_all();
    }
    virtual bool on_message(TASK_MSG msg)
    {
        if (msg.type == MSG_TYPE::TASK_HB)
        {
            hb_map.insert(std::pair<std::string, bool>(TASK_ANY_CAST<std::string>(msg.body), true));
        }
    }
    std::map<std::string, bool> hb_map;
};