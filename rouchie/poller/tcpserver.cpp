#include "tcpserver.h"

#include <spdlog/spdlog.h>

#include "base/exception.h"

#include "uv.h"

class TcpSessionContext {
public:
    TcpSessionContext();
    uv_tcp_t *Handle() const;

    std::shared_ptr<uv_tcp_t> _context;
    std::vector<char> _data;
};

TcpSessionContext::TcpSessionContext() {
    _context = std::make_shared<uv_tcp_t>();
    _context->data = this;
}

uv_tcp_t * TcpSessionContext::Handle() const {
    return _context.get();
}

class TcpServerContext {
public:
    explicit TcpServerContext(const std::shared_ptr<uv_loop_t> &_loop, bool is_ipv6 = false);

    uv_loop_t *Loop() const;
    uv_tcp_t *Handle() const;

    std::shared_ptr<uv_loop_t> _loop;
    std::shared_ptr<uv_tcp_t> _context;

    bool _is_ipv6 = false;
};

TcpServerContext::TcpServerContext(const std::shared_ptr<uv_loop_t> &_loop, bool is_ipv6) : _loop(_loop),
    _is_ipv6(is_ipv6) {
    _context = std::make_shared<uv_tcp_t>();
    _context->data = this;
}

uv_loop_t *TcpServerContext::Loop() const {
    return _loop.get();
}

uv_tcp_t *TcpServerContext::Handle() const {
    return _context.get();
}

static void tcpAlloc(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
    auto* context = static_cast<TcpSessionContext *>(handle->data);
    if (suggested_size > context->_data.size()) {
        context->_data.resize(suggested_size*2);
    }
    buf->base = context->_data.data();
    buf->len = context->_data.size();
}

static void tcpRead(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
    if (nread > 0) {
    } else if (nread < 0) {
        if (nread != UV_EOF) {
            SPDLOG_ERROR("tcp read error: {}", uv_strerror(nread));
        }
        uv_close(reinterpret_cast<uv_handle_t *>(stream), [](uv_handle_t *h) {
            delete static_cast<TcpSessionContext *>(h->data);
        });
    }
}

static void tcpConnection(uv_stream_t *server, int status) {
    if (status < 0) {
        SPDLOG_ERROR("tcp connection error: {}", uv_strerror(status));
        return;
    }

    const auto* sessionContext = new TcpSessionContext;
    auto *handle = sessionContext->Handle();

    const auto *serverContext = static_cast<TcpServerContext *>(server->data);
    auto *loop = serverContext->Loop();

    int nRet = uv_tcp_init(loop, handle);
    if (0 != nRet) {
        throw Exception("uv tcp init failed", nRet);
    }

    auto error = [](const int n, const char *msg, uv_tcp_t *h) {
        SPDLOG_ERROR("{}: {}", msg, uv_strerror(n));
        uv_close(reinterpret_cast<uv_handle_t *>(h), [](uv_handle_t *h) {
            delete static_cast<TcpSessionContext *>(h->data);
        });
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

    SPDLOG_INFO("tcp connection: {}", fmt::ptr(handle));
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

static void tcpListen(TcpServerContext *netContext, int port, const std::string &ip, int backlog) {
    auto *loop = netContext->Loop();
    auto *handle = netContext->Handle();

    sockaddr_in addr4{};
    uv_ip4_addr(ip.c_str(), port, &addr4);
    tcpListen_l(loop, handle, reinterpret_cast<sockaddr *>(&addr4), backlog);

    sockaddr_in6 addr6{};
    uv_ip6_addr(ip.c_str(), port, &addr6);
    tcpListen_l(loop, handle, reinterpret_cast<sockaddr *>(&addr6), backlog);
}


UVTcpServer::UVTcpServer(const std::shared_ptr<uv_loop_t> &_loop, int port, const std::string &ip, int backlog)
    : NET(), _ip(ip), _port(port), _backlog(backlog) {
    auto *ipv4 = new TcpServerContext(_loop);
    auto *ipv6 = new TcpServerContext(_loop, true);
    tcpListen(ipv4, port, ip, backlog);
    tcpListen(ipv6, port, ip, backlog);
}
