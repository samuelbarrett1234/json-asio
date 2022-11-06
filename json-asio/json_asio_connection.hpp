#ifndef JSON_ASIO_CONNECTION_HPP
#define JSON_ASIO_CONNECTION_HPP


#include <string>
#include <optional>
#include <boost/json.hpp>
#include <boost/asio.hpp>
#include <boost/multiprecision/cpp_int.hpp>


namespace json_asio
{
namespace detail
{


template<typename Protocol>
class Connection
{
public:
    Connection(boost::asio::ip::tcp::socket sock) :
        sock(std::move(sock))
    { }

protected:
    void write(boost::json::value msg)
    {
        const std::string msg_buf = boost::json::serialize(std::move(msg));
        buffer.resize(msg_buf.size());
        std::copy(msg_buf.begin(), msg_buf.end(), buffer.begin());
        size = buffer.size();

        boost::asio::async_write(
            sock, boost::asio::buffer(&size, sizeof(size)),
            [this](const boost::system::error_code& ec, size_t)
            {
                if (!ec)
                {
                    boost::asio::async_write(
                        sock, boost::asio::buffer(buffer),
                        [this](const boost::system::error_code& ec, size_t)
                        {
                            if (!ec)
                            {
                                buffer.clear();
                                read();
                            }
                        }
                    );
                }
            }
        );
    }

    void read()
    {
        boost::asio::async_read(
            sock, boost::asio::buffer(&size, sizeof(size)),
            [this](const boost::system::error_code& ec, size_t)
            {
                if (!ec)
                {
                    // TODO: implement size limit
                    buffer.resize(size);
                    boost::asio::async_read(
                        sock, boost::asio::buffer(buffer),
                        [this](const boost::system::error_code& ec, size_t)
                        {
                            if (!ec)
                            {
                                boost::json::value msg = boost::json::parse(
                                    boost::json::string_view(buffer.data(), buffer.size()),
                                    ec
                                );
                                buffer.clear();

                                if (!ec)
                                {
                                    write((*protocol)(std::move(msg)));
                                }
                            }
                        }
                    );
                }
            }
        );
    }

protected:
    boost::asio::ip::tcp::socket sock;
    std::vector<char> buffer;
    boost::multiprecision::uint32_t size;
};


}  // namespace detail
}  // namespace json_asio


#endif  // JSON_ASIO_CONNECTION_HPP
