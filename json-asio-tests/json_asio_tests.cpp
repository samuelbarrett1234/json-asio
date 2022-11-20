#include <functional>
#include <thread>
#include <mutex>
#include <chrono>
#include <condition_variable>
#include <json_asio_client.hpp>
#include <json_asio_server.hpp>
#include <json_asio_make_server.hpp>
#include <boost/test/unit_test.hpp>


using namespace json_asio;


struct EchoOnceProtocol
{
    EchoOnceProtocol(boost::json::value* p_out_received = nullptr,
        std::mutex* p_mtx = nullptr,
        std::condition_variable* p_cv = nullptr) :
        echoed(false),
        p_out_received(p_out_received),
        p_cv(p_cv),
        p_mtx(p_mtx)
    { }

    std::optional<boost::json::value> operator()(boost::json::value v)
    {
        if (echoed)
            return std::nullopt;

        {
            std::optional<std::unique_lock<std::mutex>> maybe_lock;
            if (p_mtx != nullptr)
                *maybe_lock = std::unique_lock<std::mutex>(*p_mtx);

            if (p_out_received)
                *p_out_received = v;
        }

        echoed = true;

        if (p_cv != nullptr)
            p_cv->notify_one();

        return v;
    }

    bool echoed;
    boost::json::value* p_out_received;
    std::mutex* p_mtx;
    std::condition_variable* p_cv;
};


std::pair<EchoOnceProtocol, std::optional<boost::json::value>> accept_and_return_uri(
    std::string uri, boost::json::value* p_out_received = nullptr)
{
    return std::make_pair(EchoOnceProtocol(p_out_received), boost::json::value_from(uri));
}


BOOST_AUTO_TEST_SUITE(JSON_ASIO_Tests)


BOOST_AUTO_TEST_CASE(EchoTest)
{
    std::condition_variable cv;
    std::mutex mtx;

    boost::json::value server_received, client_received;

    auto p_server = make_server<EchoOnceProtocol>(
        std::bind(&accept_and_return_uri, std::placeholders::_1, &server_received), 54321);
    auto p_client = std::make_unique<Client<EchoOnceProtocol>>();

    std::thread client_thread, server_thread;
    const auto start_time = std::chrono::steady_clock::now();
    const auto max_time = start_time + std::chrono::seconds(5);

    {
        std::unique_lock<std::mutex> lock(mtx);

        auto pin1 = p_server->pin_work();
        auto pin2 = p_client->pin_work();

        client_thread = std::thread([&p_client]() { p_client->run(); });
        server_thread = std::thread([&p_server]() { p_server->run(); });

        p_client->post("127.0.0.1", 54321, EchoOnceProtocol(&client_received, &mtx, &cv), "/test/", std::nullopt);

        // stop when done or when enough time has passed
        do
        {
            cv.wait_until(lock, max_time);
        }
        while (std::chrono::steady_clock::now() < max_time && client_received.is_null());

    }  // stop work pins

    p_server->stop();
    p_client->stop();

    client_thread.join();
    server_thread.join();

    BOOST_CHECK(boost::json::value_to<std::string>(client_received) == "/test/");  // initial echo from server
    BOOST_CHECK(boost::json::value_to<std::string>(server_received) == "/test/");  // client echo reply to server
}


BOOST_AUTO_TEST_SUITE_END();
 