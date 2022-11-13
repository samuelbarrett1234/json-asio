#include <functional>
#include <json_asio_client.hpp>
#include <json_asio_server.hpp>
#include <json_asio_make_server.hpp>
#include <boost/test/unit_test.hpp>


using namespace json_asio;


struct EchoOnceProtocol
{
    EchoOnceProtocol(boost::json::value* p_out_received = nullptr) :
        echoed(false),
        p_out_received(p_out_received)
    { }

    std::optional<boost::json::value> operator()(boost::json::value v)
    {
        if (echoed)
            return std::nullopt;

        if (p_out_received)
            *p_out_received = v;

        echoed = true;
        return v;
    }

    bool echoed;
    boost::json::value* p_out_received;
};


std::pair<EchoOnceProtocol, std::optional<boost::json::value>> accept_and_return_uri(
    std::string uri, boost::json::value* p_out_received = nullptr)
{
    return std::make_pair(EchoOnceProtocol(p_out_received), boost::json::value_from(uri));
}


BOOST_AUTO_TEST_SUITE(JSON_ASIO_Tests)


BOOST_AUTO_TEST_CASE(EchoTest, *boost::unit_test::timeout(5))
{
    boost::json::value server_received, client_received;

    auto p_server = make_server<EchoOnceProtocol>(
        std::bind(&accept_and_return_uri, std::placeholders::_1, &server_received), 54321);
    auto p_client = std::make_unique<Client<EchoOnceProtocol>>();

    p_client->post("127.0.0.1", 54321, EchoOnceProtocol(&client_received), "/test/", std::nullopt);

    p_server->run();  // should terminate quickly
    
    BOOST_CHECK(boost::json::value_to<std::string>(client_received) == "/test/");  // initial echo from server
    BOOST_CHECK(boost::json::value_to<std::string>(server_received) == "/test/");  // client echo reply to server
}


BOOST_AUTO_TEST_SUITE_END();
 