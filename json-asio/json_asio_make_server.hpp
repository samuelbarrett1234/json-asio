#ifndef JSON_ASIO_MAKE_SERVER_HPP
#define JSON_ASIO_MAKE_SERVER_HPP


#include "json_asio_server.hpp"


namespace json_asio
{


// a server which delegates acceptance to a given function.
// here, the acceptance function type `FunctionT` must be callable
// with an `std::string` and must return
// `std::pair<Protocol, std::optional<boost::json::value>>>`
// with the same interpretation as that from the `Server`.
template<typename Protocol, typename FunctionT>
class AutoServer
{
public:
    AutoServer(FunctionT _on_accepted) :
        m_on_accepted(std::move(_on_accepted))
    { }

protected:
    std::pair<Protocol, std::optional<boost::json::value>> on_accepted(std::string uri)
    {
        return m_on_accepted(std::move(uri));
    }

private:
    FunctionT m_on_accepted;
};


template<typename Protocol, typename FunctionT>
std::unique_ptr<Server<Protocol, AutoServer<Protocol, FunctionT>>> make_server(
    FunctionT on_accepted, unsigned short port)
{
    return std::make_unique<Server<Protocol, AutoServer<Protocol, FunctionT>>>(
        port, std::move(on_accepted));
}


}  // namespace json_asio


#endif  // JSON_ASIO_MAKE_SERVER_HPP
