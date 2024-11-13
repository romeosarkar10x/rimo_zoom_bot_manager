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

namespace my_program_state
{
    std::size_t requestcount()
    {
        static std::size_t count = 0;
        return ++count;
    }

    std::time_t now()
    {
        return std::time(0);
    }
} // namespace my_program_state

class http_connection : public std::enable_shared_from_this<http_connection>
{
public:
    http_connection(tcp::socket socket) :
        socket_(std::move(socket))
    {}

    // Initiate the asynchronous operations associated with the connection.
    void start()
    {
        read_request();
        check_deadline();
    }

private:
    // The socket for the currently connected client.
    tcp::socket socket_;

    // The buffer for performing reads.
    boost::beast::flat_buffer buffer_ { 8192 };

    // The request message.
    http::request<http::dynamic_body> request;

    // The response message.
    http::response<http::dynamic_body> response;

    // The timer for putting a deadline on connection processing.
    boost::asio::basic_waitable_timer<std::chrono::steady_clock> deadline_ { socket_.get_executor(),
                                                                             std::chrono::seconds(60) };

    // Asynchronously receive a complete request message.
    void read_request()
    {
        auto self = shared_from_this();

        http::async_read(socket_, buffer_, request,
                         [self](boost::beast::error_code ec, std::size_t bytes_transferred)
                         {
                             boost::ignore_unused(bytes_transferred);
                             if(!ec)
                                 self->process_request();
                         });
    }

    // Determine what needs to be done with the request message.
    void process_request()
    {
        response.version(request.version());
        response.keep_alive(false);

        switch(request.method())
        {
            case http::verb::get:
                response.result(http::status::ok);
                response.set(http::field::server, "Beast");
                create_response();
                break;

            default:
                // We return responses indicating an error if
                // we do not recognize the request method.
                response.result(http::status::bad_request);
                response.set(http::field::content_type, "text/plain");
                boost::beast::ostream(response.body())
                    << "Invalid request-method '" << std::string(request.method_string()) << "'";
                break;
        }

        write_response();
    }

    // Construct a response message based on the program state.
    void create_response()
    {
        if(request.target() == "/count")
        {
            response.set(http::field::content_type, "text/html");
            boost::beast::ostream(response.body())
                << "<html>\n"
                << "<head><title>Request count</title></head>\n"
                << "<body>\n"
                << "<h1>Request count</h1>\n"
                << "<p>There have been " << my_program_state::requestcount() << " requests so far.</p>\n"
                << "</body>\n"
                << "</html>\n";
        }
        else if(request.target() == "/time")
        {
            response.set(http::field::content_type, "text/html");
            boost::beast::ostream(response.body())
                << "<html>\n"
                << "<head><title>Current time</title></head>\n"
                << "<body>\n"
                << "<h1>Current time</h1>\n"
                << "<p>The current time is " << my_program_state::now() << " seconds since the epoch.</p>\n"
                << "</body>\n"
                << "</html>\n";
        }
        else
        {
            response.result(http::status::not_found);
            response.set(http::field::content_type, "text/plain");
            boost::beast::ostream(response.body()) << "File not found\r\n";
        }
    }

    // Asynchronously transmit the response message.
    void write_response()
    {
        auto self = shared_from_this();

        // response.set(http::field::content_length, response.body().size());
        std::string response_size = std::to_string(response.body().size());
        response.set(http::field::content_length, response_size);

        http::async_write(socket_, response,
                          [self](boost::beast::error_code ec, std::size_t)
                          {
                              self->socket_.shutdown(tcp::socket::shutdown_send, ec);
                              self->deadline_.cancel();
                          });
    }

    // Check whether we have spent enough time on this connection.
    void check_deadline()
    {
        auto self = shared_from_this();

        deadline_.async_wait(
            [self](boost::beast::error_code ec)
            {
                if(!ec)
                {
                    // Close socket to cancel any outstanding operation.
                    self->socket_.close(ec);
                }
            });
    }
};

// "Loop" forever accepting new connections.
void http_server(tcp::acceptor& acceptor, tcp::socket& socket)
{
    acceptor.async_accept(socket,
                          [&](boost::beast::error_code ec)
                          {
                              if(!ec)
                                  std::make_shared<http_connection>(std::move(socket))->start();
                              http_server(acceptor, socket);
                          });
}

int main(int argc, char* argv[])
{
    try
    {
        // Check command line arguments.
        if(argc != 3)
        {
            std::cerr << "Usage: " << argv[0] << " <address> <port>\n";
            std::cerr << "  For IPv4, try:\n";
            std::cerr << "    receiver 0.0.0.0 80\n";
            std::cerr << "  For IPv6, try:\n";
            std::cerr << "    receiver 0::0 80\n";
            return EXIT_FAILURE;
        }

        const auto address = boost::asio::ip::make_address(argv[1]);
        unsigned short port = static_cast<unsigned short>(std::atoi(argv[2]));

        boost::asio::io_context ioc { 1 };

        tcp::acceptor acceptor {
            ioc, { address, port }
        };
        tcp::socket socket { ioc };
        http_server(acceptor, socket);

        ioc.run();
    }
    catch(const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
