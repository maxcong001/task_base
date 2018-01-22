

#include <queue>
#include <iostream>
#include <array>
#include <map>
#include <string>
#include <memory>
#include <thread>
#include <cstdlib>

#include "translib/loop.h"
#include "translib/eventClient.h"
#include "translib/eventServer.h"
#include "translib/timerManager.h"
#include "translib/timer.h"
#include "logger/logger.hpp"

#if __cplusplus >= 201703L
// use std::any
#include <any>
#define TASK_ANY std::any
#define TASK_ANY_CAST std::any_cast
#else
#include <boost/any.hpp>
#define TASK_ANY boost::any
#define TASK_ANY_CAST boost::any_cast
#endif

typedef std::queue<TASK_ANY> TASK_QUEUE;
#define TASK0 "task0"
