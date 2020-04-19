#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <type_traits>

#include "espp/log.h"
#include "espp/lts.h"

namespace espp {

class TaskBase {
public:
    static
    void Delay(UBaseType_t delayTicks)
    {
        vTaskDelay(delayTicks);
    }

    static
    void DelayMs(UBaseType_t delayMs)
    {
        Delay(pdMS_TO_TICKS(delayMs));
    }
};

class Task: public TaskBase {
public:
    explicit
    Task(UBaseType_t priority = 0):
        _handle(),
        _priority(priority)
    {
        vTaskSetThreadLocalStoragePointer(nullptr, static_cast<BaseType_t>(LTS::TASK), this);
    }

    explicit
    Task(const char* name):
        _handle(),
        _priority(0),
        _name(name)
    {
        vTaskSetThreadLocalStoragePointer(nullptr, static_cast<BaseType_t>(LTS::TASK), this);
    }

    UBaseType_t priority() const
    {
        return _priority;
    }

    configSTACK_DEPTH_TYPE stack_depth() const
    {
        return 4096;
    }

    TaskHandle_t& handle()
    {
        return _handle;
    }

    const char* name()
    {
        if (_name == nullptr) {
            return "";
        }
        return _name;
    }

    void init_run()
    {
    }


    template<class InstTask>
    static void _task_function(void* pTask)
    {
        InstTask& task = *reinterpret_cast<InstTask*>(pTask);
        task.init_run();
        task.run();
        vTaskDelete(NULL);
    }

    template<class InstTask>
    static void Start(InstTask& task)
    {
        static_assert(std::is_base_of<Task, InstTask>::value, "expect Task");
        auto run_func = Task::_task_function<InstTask>;
        auto param = reinterpret_cast<void*>(&task);
        xTaskCreate(run_func, task.name(), task.stack_depth(), param, task.priority(), &task.handle());
    }


protected:
    TaskHandle_t _handle;
    UBaseType_t _priority;

private:
    const char* _name = nullptr;
};

}
