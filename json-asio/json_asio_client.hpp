#ifndef JSON_ASIO_CLIENT_HPP
#define JSON_ASIO_CLIENT_HPP


#include <optional>
#include "detail/json_asio_connection.hpp"


namespace json_asio
{


// an object for maintaining persistent connections with
// servers, allowing you to easily create new connections.
template<typename Protocol>
class Client
{
private:
    class Connection :
        public detail::Connection<Protocol>
    {
    public:
        Connection(
            Client<Protocol>& client, boost::asio::ip::tcp::socket sock,
            std::string uri, Protocol protocol,
            std::optional<boost::json::value> initial_message) :
            detail::Connection<Protocol>(std::move(sock), std::move(protocol)),
            client(client), uri(std::move(uri)),
            initial_message(std::move(initial_message))
        {
            send_uri_size();
        }

    private:
        Client<Protocol>& client;
        const std::string uri;
        std::optional<boost::json::value> initial_message;

    private:
        void send_uri_size()
        {
            size = uri.size();
            boost::asio::async_write(
                sock, boost::asio::buffer(&size, sizeof(size)),
                [this](const boost::system::error_code& ec, size_t)
            {
                if (!ec)
                {
                    send_uri();
                }
            });
        }

        void send_uri()
        {
            boost::asio::async_write(
                sock, boost::asio::buffer(uri.data(), uri.size()),
                [this](const boost::system::error_code& ec, size_t)
            {
                if (!ec)
                {
                    if (initial_message)
                    {
                        write(std::move(*initial_message));
                    }
                    else
                    {
                        read();
                    }
                }
            });
        }
    };

public:
    using WorkPin = std::unique_ptr<boost::asio::io_service::work>;

public:
    void post(
        std::string ip_addr,
        unsigned short port,
        Protocol protocol,  // how to reply to any server messages
        std::string uri,  // URI / URL / generic text identifier

        // if selected, post a message straight away without waiting
        // for server to initiate conversation
        // (warning: if neither client nor server provides an initial
        // message, nobody will talk to one another!)
        std::optional<boost::json::value> initial_message
    )
    {
        boost::asio::ip::tcp::socket sock(m_ioc);
        sock.connect(boost::asio::ip::tcp::endpoint(
            boost::asio::ip::address(boost::asio::ip::address_v4()),
            port
        ));
        m_connections.emplace_back(std::make_unique<Connection>(
            *this,
            std::move(sock),
            std::move(uri),
            std::move(protocol),
            std::move(initial_message)
        ));
    }

    // commit a thread to running client business
    void run()
    {
        m_ioc.run();
    }

    // close all active connections
    void stop()
    {
        m_ioc.stop();
    }

    // while the returned pin object is in scope, prevent threads from
    // returning from `run` when there is no more work left to do.
    WorkPin pin_work()
    {
        return std::make_unique<boost::asio::io_service::work>(m_ioservice);
    }

private:
    boost::asio::io_service m_ioc;
    std::vector<std::unique_ptr<Connection>> m_connections;
};


}  // namespace json_asio


#endif  // JSON_ASIO_SERVER_HPP
