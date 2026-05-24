#include "tcpserver.h"

#include <comcat.h>

#include "net/basesession.h"
#include "base/exception.h"
#include "buffer/vectorbuffer.h"

#include "spdlog/spdlog.h"
#include "uv.h"

class TcpSessionContext : public BaseContext {
public:
    static uv_tcp_t *Create(BaseSession::Ptr session);

    uv_tcp_t *Handle() const;

    void Alloc(size_t suggested_size, uv_buf_t *buf);

    void Read(ssize_t nread, const uv_buf_t *buf) const;

protected:
    explicit TcpSessionContext(BaseSession::Ptr session);

private:
    BaseSession::Ptr _session;
    std::shared_ptr<uv_tcp_t> _handle;
    std::vector<char> _data;
};

uv_tcp_t *TcpSessionContext::Create(BaseSession::Ptr session) {
    const auto context = new TcpSessionContext(std::move(session));
    return context->Handle();
}

TcpSessionContext::TcpSessionContext(BaseSession::Ptr session) : _session(std::move(session)) {
    _handle = std::make_shared<uv_tcp_t>();
    _handle->data = this;
}

uv_tcp_t *TcpSessionContext::Handle() const {
    return _handle.get();
}

void TcpSessionContext::Alloc(const size_t suggested_size, uv_buf_t *buf) {
    if (_data.size() < suggested_size) {
        _data.resize(suggested_size);
    }
    buf->base = _data.data();
    buf->len = static_cast<ULONG>(_data.size());
}

void TcpSessionContext::Read(const ssize_t nread, const uv_buf_t *buf) const {
    if (nread < 0) {
        if (nread == UV_EOF) {
            _session->OnError(Exception("eof", CODE_EOF, static_cast<int>(nread)));
        } else {
            _session->OnError(Exception("tcp read error", CODE_EXCEPTION, static_cast<int>(nread)));
        }
        uv_close(reinterpret_cast<uv_handle_t *>(_handle.get()), CloseCallback);
        return;
    }

    try {
        _session->OnRead(std::make_shared<VectorBuffer>(buf->base, nread));
    } catch (const Exception &e) {
        _session->OnError(e);
        uv_close(reinterpret_cast<uv_handle_t *>(_handle.get()), CloseCallback);
    }
}

class TcpServerContext : public BaseContext {
public:
    static uv_tcp_t *Create(EventPoller::Ptr poller, LoopContext::Ptr loop, EventPoller::CreateSessionFunc func);

    uv_loop_t *Loop() const;

    uv_tcp_t *Handle() const;

    BaseSession::Ptr CreateSession() const;

protected:
    TcpServerContext(EventPoller::Ptr poller, LoopContext::Ptr loop, EventPoller::CreateSessionFunc func);

private:
    EventPoller::Ptr _poller;
    LoopContext::Ptr _loop;
    std::shared_ptr<uv_tcp_t> _context;
    EventPoller::CreateSessionFunc _createSessionFunc;
};

uv_tcp_t *TcpServerContext::Create(EventPoller::Ptr poller, LoopContext::Ptr loop, EventPoller::CreateSessionFunc func) {
    const auto context = new TcpServerContext(std::move(poller), std::move(loop), std::move(func));
    return context->Handle();
}

uv_loop_t *TcpServerContext::Loop() const {
    return _loop->Handle();
}

uv_tcp_t *TcpServerContext::Handle() const {
    return _context.get();
}

BaseSession::Ptr TcpServerContext::CreateSession() const {
    return _createSessionFunc(_poller);
}

TcpServerContext::TcpServerContext(EventPoller::Ptr poller, LoopContext::Ptr loop, EventPoller::CreateSessionFunc func) : _poller(std::move(poller)), _loop(std::move(loop)),
    _createSessionFunc(std::move(func)) {
    _context = std::make_shared<uv_tcp_t>();
    _context->data = this;
}

static void tcpAlloc(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
    auto *context = static_cast<TcpSessionContext *>(handle->data);
    context->Alloc(suggested_size, buf);
}

static void tcpRead(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
    auto *context = static_cast<TcpSessionContext *>(stream->data);
    context->Read(nread, buf);
}

static void tcpConnection(uv_stream_t *server, int status) {
    if (status < 0) {
        SPDLOG_ERROR("tcp connection error: {}", uv_strerror(status));
        return;
    }

    const auto *serverContext = static_cast<TcpServerContext *>(server->data);
    auto *loop = serverContext->Loop();

    auto *handle = TcpSessionContext::Create(serverContext->CreateSession());

    int nRet = uv_tcp_init(loop, handle);
    if (0 != nRet) {
        SPDLOG_ERROR("uv tcp init failed: {}", uv_strerror(nRet));
        return;
    }

    auto error = [](const int n, const char *msg, uv_tcp_t *h) {
        SPDLOG_ERROR("{}: {}", msg, uv_strerror(n));
        uv_close(reinterpret_cast<uv_handle_t *>(h), CloseCallback);
    };

    nRet = uv_accept(server, reinterpret_cast<uv_stream_t *>(handle));
    if (0 != nRet) {
        error(nRet, "tcp accept failed", handle);
        return;
    }

    nRet = uv_read_start(reinterpret_cast<uv_stream_t *>(handle), tcpAlloc, tcpRead);
    if (0 != nRet) {
        error(nRet, "tcp read start failed", handle);
        return;
    }
}

static void tcpListen_l(uv_loop_t *loop, uv_tcp_t *handle, const struct sockaddr *addr, int backlog) {
    int nRet = uv_tcp_init(loop, handle);
    if (0 != nRet) {
        throw Exception("uv tcp init failed", nRet);
    }

    nRet = uv_tcp_bind(handle, addr, 0);
    if (0 != nRet) {
        throw Exception("uv tcp bind failed", nRet);
    }

    nRet = uv_listen(reinterpret_cast<uv_stream_t *>(handle), backlog, tcpConnection);
    if (0 != nRet) {
        throw Exception("uv tcp listen failed", nRet);
    }
}

static void tcpListen(uv_tcp_t *handle, int port, const std::string &ip, int backlog) {
    const auto context = static_cast<TcpServerContext *>(handle->data);

    sockaddr_in6 addr6{};
    uv_ip6_addr(ip.c_str(), port, &addr6);
    tcpListen_l(context->Loop(), handle, reinterpret_cast<sockaddr *>(&addr6), backlog);

    SPDLOG_INFO("uv tcp listen ip({}) port({}) success", ip, port);
}

NET::Ptr UVTcpServer::Create(EventPoller::Ptr poller, LoopContext::Ptr loop, int port, const std::string &ip,
                             int backlog, EventPoller::CreateSessionFunc func) {
    auto server = std::shared_ptr<UVTcpServer>(
        new UVTcpServer(std::move(poller), std::move(loop), port, ip, backlog, func));
    server->Start();
    return server;
}

UVTcpServer::~UVTcpServer() {
    auto handle = _handle;
    _poller->FirstSync([handle]() {
        uv_close(reinterpret_cast<uv_handle_t *>(handle), CloseCallback);
    });
}

bool UVTcpServer::Live() const {
    return _live;
}

std::string UVTcpServer::Error() const {
    return _error;
}

UVTcpServer::UVTcpServer(EventPoller::Ptr poller, LoopContext::Ptr loop, int port, const std::string &ip, int backlog,
                         EventPoller::CreateSessionFunc func)
    : NET(ip, port, backlog), _poller(std::move(poller)), _loop(std::move(loop)) {
    _handle = TcpServerContext::Create(_poller, _loop, std::move(func));
}

void UVTcpServer::Start() {
    auto self = shared_from_this();
    auto task = [this, self]() {
        try {
            tcpListen(_handle, _port, _ip, _backlog);
            _live = true;
            _error = "ok";
        } catch (const Exception &e) {
            _live = false;
            _error = e.what();
            SPDLOG_ERROR("tcp server start error: {} (error code: {})", _error, uv_strerror(e.code()));
        }
    };

    _poller->FirstSync(task);
}
