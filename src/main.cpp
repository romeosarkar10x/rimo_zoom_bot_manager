#include <iostream>

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio.hpp>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <memory>

using tcp = boost::asio::ip::tcp;    // from <boost/asio.hpp>
namespace http = boost::beast::http; // from <boost/beast/http.hpp>

std::size_t request_count()
{
    static std::size_t count = 0;
    return ++count;
}

std::time_t now()
{
    return std::time(0);
}

struct http_connection : public std::enable_shared_from_this<http_connection>
{
    http_connection(tcp::socket socket) :
        m_socket(std::move(socket))
    {}

    // Initiate the asynchronous operations associated with the connection.
    void start()
    {
        read_request();
        check_deadline();
    }

private:
    // The socket for the currently connected client.
    tcp::socket m_socket;

    // The buffer for performing reads.
    boost::beast::flat_buffer m_buffer { 8192 };

    // The request message.
    http::request<http::dynamic_body> m_request;

    // The response message.
    http::response<http::dynamic_body> m_response;

    // The timer for putting a deadline on connection processing.
    boost::asio::basic_waitable_timer<std::chrono::steady_clock> m_deadline { m_socket.get_executor(),
                                                                              std::chrono::seconds(60) };

    // Asynchronously receive a complete request message.
    void read_request()
    {
        auto self = shared_from_this();

        http::async_read(m_socket, m_buffer, m_request,
                         [self](boost::beast::error_code error_code, std::size_t bytes_transferred)
                         {
                             boost::ignore_unused(bytes_transferred);
                             if(!error_code)
                                 self->process_request();
                         });
    }

    // Determine what needs to be done with the request message.
    void process_request()
    {
        m_response.version(m_request.version());
        m_response.keep_alive(false);

        switch(m_request.method())
        {
            case http::verb::get:
                m_response.result(http::status::ok);
                // response.set(http::field::server, "Beast");
                create_get_response();
                break;

            case http::verb::post:
                create_post_response();
            default:
                // We return responses indicating an error if
                // we do not recognize the request method.
                m_response.result(http::status::bad_request);
                m_response.set(http::field::content_type, "text/plain");
                boost::beast::ostream(m_response.body())
                    << "Invalid request-method '" << std::string(m_request.method_string()) << "'";
                break;
        }

        write_response();
    }

    // Construct a response message based on the program state.
    void create_get_response()
    {
        if(m_request.target() == "/count")
        {
            m_response.set(http::field::content_type, "text/html");
            boost::beast::ostream(m_response.body())
                << "<html>\n"
                << "<head><title>Request count</title></head>\n"
                << "<body>\n"
                << "<h1>Request count</h1>\n"
                << "<p>There have been " << request_count() << " requests so far.</p>\n"
                << "</body>\n"
                << "</html>\n";
        }
        else if(m_request.target() == "/time")
        {
            m_response.set(http::field::content_type, "text/html");
            boost::beast::ostream(m_response.body())
                << "<html>\n"
                << "<head><title>Current time</title></head>\n"
                << "<body>\n"
                << "<h1>Current time</h1>\n"
                << "<p>The current time is " << now() << " seconds since the epoch.</p>\n"
                << "</body>\n"
                << "</html>\n";
        }
        else
        {
            m_response.result(http::status::not_found);
            m_response.set(http::field::content_type, "text/plain");
            boost::beast::ostream(m_response.body()) << "invalid path\r\n";
        }
    }

    void create_post_response()
    {
        boost::beast::ostream(m_response.body()) << "{\"devesh\":\"gadha\"}";
        m_response.result(http::status::ok);
    }

    // Asynchronously transmit the response message.
    void write_response()
    {
        auto self = shared_from_this();

        // response.set(http::field::content_length, response.body().size());
        std::string response_size = std::to_string(m_response.body().size());
        m_response.set(http::field::content_length, response_size);

        http::async_write(m_socket, m_response,
                          [self](boost::beast::error_code error_code, std::size_t)
                          {
                              auto return_ec = self->m_socket.shutdown(tcp::socket::shutdown_send, error_code);
                              self->m_deadline.cancel();
                          });
    }

    // Check whether we have spent enough time on this connection.
    void check_deadline()
    {
        auto self = shared_from_this();

        m_deadline.async_wait(
            [self](boost::beast::error_code error_code)
            {
                if(!error_code)
                {
                    // Close socket to cancel any outstanding operation.
                    auto return_ec = self->m_socket.close(error_code);
                }
            });
    }
};

// "Loop" forever accepting new connections.
void http_server(tcp::acceptor& acceptor, tcp::socket& socket)
{
    acceptor.async_accept(socket,
                          [&](boost::beast::error_code error_code)
                          {
                              if(!error_code)
                                  std::make_shared<http_connection>(std::move(socket))->start();
                              http_server(acceptor, socket);
                          });
}

int main()
{
    try
    {
        const boost::asio::ip::address address = boost::asio::ip::make_address("0.0.0.0");
        unsigned short port = static_cast<unsigned short>(80U);

        boost::asio::io_context io_context { 1 };

        tcp::acceptor acceptor {
            io_context, { address, port }
        };
        tcp::socket socket { io_context };
        http_server(acceptor, socket);

        io_context.run();
    }
    catch(const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
