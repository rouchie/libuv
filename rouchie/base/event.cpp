#include "event.h"

std::shared_ptr<Event> Event::Create() {
    std::shared_ptr<Event> event(new Event());
    event->Init();
    return event;
}

Event::~Event() {
    uv_loop_close(_loop);
}

void Event::Start() const {
    uv_run(_loop, UV_RUN_DEFAULT);
}

void Event::Execute(const Task &task) {
    std::lock_guard<std::mutex> lock(_mutex);
    _tasks.push_back(task);
    uv_async_send(&_async);
}

void Event::FirstExecute(const Task &task) {
    std::lock_guard<std::mutex> lock(_mutex);
    _tasks.push_front(task);
    uv_async_send(&_async);
}

void Event::DoExecute() {
    std::list<Task> tasks;
    {
        std::lock_guard<std::mutex> lock(_mutex);
        tasks.swap(_tasks);
    }
    for (const auto& task : tasks) {
        task();
    }
}

Event::Event() : _async() {
    _loop = uv_default_loop();
    uv_loop_init(_loop);
}

void Event::Init() {
    _async.data = this;
    uv_async_init(_loop, &_async, [](uv_async_t* async) {
        const auto event = static_cast<Event*>(async->data);
        event->DoExecute();
    });
}

