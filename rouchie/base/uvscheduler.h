#pragma once

#include <memory>
#include <functional>
#include <mutex>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "uv.h"
#include "taskscheduler.h"
#include "netscheduler.h"

class NetContext {
public:
    using Ptr = std::shared_ptr<NetContext>;

    std::shared_ptr<uv_tcp_t> context;
    void* data;
};

class TcpSessionInfo {
public:
    using Ptr = std::shared_ptr<TcpSessionInfo>;
    ~TcpSessionInfo() {
        const auto s = session.lock();
        if (s) {
            uv_close(reinterpret_cast<uv_handle_t *>(s.get()), [](uv_handle_t* handle) {
                delete static_cast<NetContext*>(handle->data);
            });
        }
    }

    std::weak_ptr<uv_tcp_t> session;
    NetContext* context;

    std::vector<char> _data;
};

class TcpServerInfo {
public:
    using Ptr = std::shared_ptr<TcpServerInfo>;

    ~TcpServerInfo() {
        const auto s = server.lock();
        if (s) {
            uv_close(reinterpret_cast<uv_handle_t *>(s.get()), [](uv_handle_t* handle) {
                delete static_cast<NetContext*>(handle->data);
            });
        }
    }

    std::string host;
    int port;

    std::weak_ptr<uv_tcp_t> server;
    NetContext* context;

    std::unordered_set<TcpSessionInfo::Ptr> sessions;
};

class TCP : public NET {
public:
    TCP(TcpServerInfo::Ptr ipv4, TcpServerInfo::Ptr ipv6) : _ipv4Server(std::move(ipv4)),
                                                               _ipv6Server(std::move(ipv6)) {
    }
    ~TCP() override = default;

private:
    TcpServerInfo::Ptr _ipv4Server;
    TcpServerInfo::Ptr _ipv6Server;
};

/**
 * 基于 libuv 的任务调度器实现
 */
class UvScheduler : public TaskScheduler, public NetScheduler, public std::enable_shared_from_this<UvScheduler> {
public:
    static std::shared_ptr<UvScheduler> Create();

    ~UvScheduler() override;

    void Start() override;
    void Stop() override;

    void Post(Task task) override;
    void PostHighPriority(Task task) override;

    NET::Ptr TcpStart(const std::string &host, int port, int backlog) override;

private:
    UvScheduler();

    void Init();
    void HandleTasks();

    TcpServerInfo::Ptr FindTcpServerInfo(uv_tcp_t* server) const;
    TcpSessionInfo::Ptr FindTcpSessionInfo(uv_tcp_t* session) const;

    // TCP private methods
    void TcpAccept(uv_stream_t *server, int status);
    void TcpAlloc(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
    void TcpRead(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf);

private:
    std::shared_ptr<uv_loop_t> _loop;
    uv_async_t _async;

    std::mutex _mutex;
    std::list<Task> _tasks;

    std::unordered_map<void*, std::weak_ptr<TcpServerInfo>> _tcpServers;
    std::unordered_map<void*, std::weak_ptr<TcpSessionInfo>> _tcpSessions;
};
