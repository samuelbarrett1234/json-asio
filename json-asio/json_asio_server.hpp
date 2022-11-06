#ifndef JSON_ASIO_SERVER_HPP
#define JSON_ASIO_SERVER_HPP


#include <optional>
#include <memory>
#include <boost/json.hpp>
#include <boost/asio.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include "json_asio_connection.hpp"


namespace json_asio
{


// an object for maintaining persistent connections with
// clients, allowing you to automatically accept new
// connections.
// `Base` must have the following function:
// `std::pair<Protocol, std::optional<boost::json::value>>> on_accepted(std::string uri)
template<typename Protocol, typename Base>
class Server :
    protected Base
{
private:
    class Connection :
        public detail::Connection<Protocol>
    {
    public:
        Connection(Server<Protocol, Base>& server, boost::asio::ip::tcp::socket sock) :
            detail::Connection<Protocol>(std::move(sock)),
            server(server)
        {
            get_uri_size();
        }

    private:
        Server<Protocol, Base>& server;
        std::string uri;
        std::optional<Protocol> protocol;

    private:
        void get_uri_size()
        {
            boost::asio::async_read(
                sock, boost::asio::buffer(&size, sizeof(size)),
                [this](const boost::system::error_code& ec, size_t bytes_transferred)
            {
                if (!ec)
                {
                    get_uri();
                }
            });
        }

        void get_uri()
        {
            // TODO: implement size limit
            uri.resize(size);
            boost::asio::async_read(
                sock, boost::asio::buffer(uri.data(), uri.size()),
                [this](const boost::system::error_code& ec, size_t bytes_transferred)
            {
                if (!ec)
                {
                    auto [p, initial_message] = server.on_accepted(std::move(uri_buffer));
                    protocol = std::move(p);
                    uri.clear();

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
    Server(unsigned short port) :
        m_acceptor(boost::asio::ip::tcp::endpoint(tcp::v4(), port))
    {
        accept();
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
    void accept()
    {
        m_acceptor.async_accept(
            [this](const boost::system::error_code& ec, boost::asio::ip::tcp::socket sock)
            {
                if (!ec)
                {
                    // asynchronously handle new connection
                    m_connections.emplace_back(std::make_unique<Connection>(*this, std::move(sock)));
                }

                // regardless of success or failure, get ready to accept incoming connections
                accept();
            }
        );
    }

private:
    boost::asio::io_service m_ioc;
    boost::asio::ip::tcp::acceptor m_acceptor;
    std::vector<std::unique_ptr<Connection>> m_connections;
};


}  // namespace json_asio


#endif  // JSON_ASIO_SERVER_HPP
