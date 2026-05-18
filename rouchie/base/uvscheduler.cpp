#include "uvscheduler.h"
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <spdlog/spdlog.h>

#include "exception.h"

std::shared_ptr<UvScheduler> UvScheduler::Create() {
    std::shared_ptr<UvScheduler> scheduler(new UvScheduler());
    scheduler->Init();
    return scheduler;
}

UvScheduler::~UvScheduler() {
    SPDLOG_TRACE("~UvScheduler");
}

void UvScheduler::Stop() {
    if (_loop) {
        uv_close(reinterpret_cast<uv_handle_t *>(&_async), nullptr);
        uv_stop(_loop.get());
    }
}

void UvScheduler::Start() {
    std::stringstream oss;
    oss << std::this_thread::get_id();

    SPDLOG_INFO("UvScheduler start with {} thread", oss.str());

    uv_run(_loop.get(), UV_RUN_DEFAULT);
    _loop.reset();

    SPDLOG_INFO("UvScheduler {} thread done", oss.str());
}

void UvScheduler::Post(Task task) {
    std::lock_guard<std::mutex> lock(_mutex);
    _tasks.push_back(task);
    uv_async_send(&_async);
}

void UvScheduler::PostHighPriority(Task task) {
    std::lock_guard<std::mutex> lock(_mutex);
    _tasks.push_front(task);
    uv_async_send(&_async);
}

NET::Ptr UvScheduler::TcpStart(const std::string &host, int port, int backlog) {
    // 创建 TCP 服务器的通用逻辑
    auto createServer = [this, host, port, backlog](sockaddr *addr, const std::string& ipVersion) -> TcpServerInfo::Ptr {
        const auto server = std::make_shared<uv_tcp_t>();

        auto* context = new NetContext;
        context->context = server;
        context->data = this;

        server->data = context;

        uv_tcp_init(_loop.get(), server.get());

        int nRet = uv_tcp_bind(server.get(), addr, 0);
        if (0 != nRet) {
            SPDLOG_WARN("uv tcp bind {} failed: {}", ipVersion, uv_strerror(nRet));
            return nullptr;
        }

        nRet = uv_listen(reinterpret_cast<uv_stream_t *>(server.get()), backlog, [](uv_stream_t *server, int status) {
            auto *context = static_cast<NetContext *>(server->data);
            auto *thiz = static_cast<UvScheduler *>(context->data);
            thiz->TcpAccept(server, status);
        });
        if (0 != nRet) {
            SPDLOG_WARN("uv tcp listen {} failed: {}", ipVersion, uv_strerror(nRet));
            return nullptr;
        }

        SPDLOG_INFO("{} server started on {}:{}", ipVersion, host, port);

        const auto info = std::make_shared<TcpServerInfo>();
        info->host = host;
        info->port = port;
        info->server = server;
        info->context = context;

        return info;
    };

    // 创建 IPv4 服务器
    sockaddr_in addr4{};
    uv_ip4_addr(host.c_str(), port, &addr4);

    const auto ipv4 = createServer(reinterpret_cast<sockaddr *>(&addr4), "IPv4");
    if (ipv4) {
        _tcpServers[ipv4->context] = ipv4;
    }

    // 创建 IPv6 服务器（失败不影响 IPv4）
    sockaddr_in6 addr6{};
    uv_ip6_addr(host.c_str(), port, &addr6);

    const auto ipv6 = createServer(reinterpret_cast<sockaddr *>(&addr6), "IPv6");
    if (ipv6) {
        _tcpServers[ipv6->context] = ipv6;
    }

    // 至少有一个成功
    if (!ipv4 && !ipv6) {
        throw Exception("Failed to start both IPv4 and IPv6 servers", -1);
    }

    return std::make_shared<TCP>(ipv4, ipv6);
}

void UvScheduler::TcpAccept(uv_stream_t *server, int status) {
    const auto session = std::make_shared<uv_tcp_t>();

    auto* context = new NetContext;
    context->context = session;
    context->data = this;

    session->data = context;

    if (0 != uv_tcp_init(_loop.get(), session.get())) {
    }

    uv_accept(server, reinterpret_cast<uv_stream_t *>(session.get()));

    auto alloc = [](uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
        auto *context = static_cast<NetContext *>(handle->data);
        auto *thiz = static_cast<UvScheduler *>(context->data);
        thiz->TcpAlloc(handle, suggested_size, buf);
    };

    auto read = [](uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
        auto *context = static_cast<NetContext *>(stream->data);
        auto *thiz = static_cast<UvScheduler *>(context->data);
        thiz->TcpRead(stream, nread, buf);
    };

    uv_read_start(reinterpret_cast<uv_stream_t *>(session.get()), alloc, read);

    SPDLOG_INFO("tcp session new: {}", fmt::ptr(session.get()));

    const auto info = std::make_shared<TcpSessionInfo>();
    info->session = session;

    _tcpSessions[context] = info;

    // 所有session都绑定到server，这样server关闭的时候，所有session都自动关闭了
    const auto it = _tcpServers.find(server->data);
    if (it != _tcpServers.end()) {
        const auto serverInfo = it->second.lock();
        serverInfo->sessions.emplace(info);
    }
}

void UvScheduler::TcpAlloc(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
    SPDLOG_INFO("tcp session alloc: {}", fmt::ptr(handle));

    const auto it = _tcpSessions.find(handle->data);
    if (it == _tcpSessions.end()) {
        return;
    }

    const auto sessionInfo = it->second.lock();
    if (sessionInfo->_data.size() < suggested_size) {
        sessionInfo->_data.resize(suggested_size*2);
    }

    buf->base = sessionInfo->_data.data();
    buf->len = static_cast<ULONG>(sessionInfo->_data.size());
}

void UvScheduler::TcpRead(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
    if (nread == UV_EOF) {
        SPDLOG_INFO("tcp session eof: {}", fmt::ptr(stream));
        // session 断开
        return;
    }
    SPDLOG_INFO("tcp session read: {} {}:{}", fmt::ptr(stream), nread, std::string(buf->base, nread));
}

UvScheduler::UvScheduler() : _loop(nullptr), _async() {
    _loop = std::shared_ptr<uv_loop_t>(new uv_loop_t(), [](uv_loop_t *loop) {
        uv_loop_close(loop);
        delete loop;
    });

    const int ret = uv_loop_init(_loop.get());
    if (ret != 0) {
        throw std::runtime_error("Failed to init uv loop: " + std::string(uv_strerror(ret)));
    }

    SPDLOG_TRACE("UvScheduler");
}

void UvScheduler::Init() {
    _async.data = this;
    const int ret = uv_async_init(_loop.get(), &_async, [](uv_async_t *async) {
        const auto scheduler = static_cast<UvScheduler *>(async->data);
        scheduler->HandleTasks();
    });

    if (ret != 0) {
        throw std::runtime_error("Failed to init async: " + std::string(uv_strerror(ret)));
    }
}

void UvScheduler::HandleTasks() {
    std::list<Task> tasks;
    {
        std::lock_guard<std::mutex> lock(_mutex);
        tasks.swap(_tasks);
    }
    for (const auto &task: tasks) {
        task();
    }
}

TcpServerInfo::Ptr UvScheduler::FindTcpServerInfo(uv_tcp_t* server) const {
    const auto it = _tcpServers.find(server);
    if (it == _tcpServers.end()) {
        return nullptr;
    }
    return it->second.lock();
}

TcpSessionInfo::Ptr UvScheduler::FindTcpSessionInfo(uv_tcp_t* session) const {
    const auto it = _tcpSessions.find(session);
    if (it == _tcpSessions.end()) {
        return nullptr;
    }
    return it->second.lock();
}
