//
// Copyright (c) 2017 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

//------------------------------------------------------------------------------
//
// Example: HTTP server, small
//
//------------------------------------------------------------------------------

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/asio.hpp>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <memory>
#include <string>
#include <opencv2/opencv.hpp>
#include "../include/cpp-base64/base64.cpp"

using namespace cv;

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

using boost::property_tree::ptree; 
using boost::property_tree::read_json; 
using boost::property_tree::write_json;


namespace my_program_state
{
    std::size_t
    request_count()
    {
        static std::size_t count = 0;
        return ++count;
    }

    std::time_t
    now()
    {
        return std::time(0);
    }
}

class http_connection : public std::enable_shared_from_this<http_connection>
{
public:
    http_connection(tcp::socket socket)
        : socket_(std::move(socket))
    {
    }

    // Initiate the asynchronous operations associated with the connection.
    void
    start()
    {
        read_request();
        check_deadline();
    }

private:
    // The socket for the currently connected client.
    tcp::socket socket_;

    // The buffer for performing reads.
    beast::flat_buffer buffer_{8192000000000};

    // The request message.
    http::request<http::dynamic_body> request_;

    // The response message.
    http::response<http::dynamic_body> response_;

    // The timer for putting a deadline on connection processing.
    net::steady_timer deadline_{
        socket_.get_executor(), std::chrono::seconds(1000)};

    // Asynchronously receive a complete request message.
    void
    read_request()
    {
        auto self = shared_from_this();

        http::async_read(
            socket_,
            buffer_,
            request_,
            [self](beast::error_code ec,
                std::size_t bytes_transferred)
            {
                boost::ignore_unused(bytes_transferred);
                if(!ec)
                    self->process_request();
            });
        // http::read(stream, buffer, res);
        // http::read(socket_, buffer_, request_);
    }

    // Determine what needs to be done with the request message.
    void
    process_request()
    {
        response_.version(request_.version());
        response_.keep_alive(false);

        switch(request_.method())
        {

        case http::verb::post:
            response_.result(http::status::ok);
            response_.set(http::field::server, "Beast");
            create_post_response();
            break;


        default:
            // We return responses indicating an error if
            // we do not recognize the request method.
            ptree root;
            root.put ("code", "400");
            root.put ( "message", "Invalid request method");
            
            std::ostringstream buf; 
            write_json (buf, root, false);
            std::string json = buf.str();

            response_.result(http::status::bad_request);
            response_.set(http::field::content_type, "application/json");
            beast::ostream(response_.body())
                << json;
            break;
        }

        write_response();
    }

    int 
    resize_image(Mat ori_image, Mat& new_image, int new_width, int new_height)
    {
        int ori_width = ori_image.cols;
        int ori_height = ori_image.rows;
        printf("ori_width : %d , ori_height : %d \n", ori_width, ori_height);
        printf("new_width : %d , new_height : %d \n", new_width, new_height);
        if (new_width <= 0 || new_height <= 0) {
            return 0;
        } else {
            resize(ori_image, new_image, Size(new_width,new_height));
            return 1;        
        }
        
    }

    void
    create_post_response()
    {
        if(request_.target() == "/resize_image")
        {
            printf("got : %s \n", request_.body().size());
            auto bodycontent = request_.body().data();
            printf("type : %s \n", typeid(bodycontent).name());
            std::cout << request_ << std::endl;

            Mat image;
            Mat new_image;
            int new_width;
            int new_height;

            if (request_.body().size() == NULL){
                std::cout << "Empty body, open image from file to simulate result" << std::endl;
                // dummy test for image
                new_width = 100;
                new_height = 100;
                String image_dir = "../testdata/gundamcat.png";
                
                image = imread( image_dir, 1 );
            } else {
                // not implemented yet
            }
            
            // resize
            resize_image(image, new_image, new_width, new_height);

            // convert to base64
            std::vector<uchar> bufimg;
            cv::imencode(".jpg", new_image, bufimg);
            auto *enc_msg = reinterpret_cast<unsigned char*>(bufimg.data());
            std::string encoded = base64_encode(enc_msg, bufimg.size());

            // namedWindow("Display Image", WINDOW_AUTOSIZE );
            // imshow("Display Image", new_image);
            // waitKey(0);

            ptree root;
            root.put ("code", "200");
            root.put ( "message", "success");
            root.put ( "output_jpeg", encoded);
            
            std::ostringstream buf; 
            write_json (buf, root, false);
            std::string json = buf.str();

            response_.set(http::field::content_type, "application/json");
            beast::ostream(response_.body()) << json;
        }
        else
        {
            ptree root;
            root.put ("code", "404");
            root.put ( "message", "File not found");
            
            std::ostringstream buf; 
            write_json (buf, root, false);
            std::string json = buf.str();

            response_.result(http::status::not_found);
            response_.set(http::field::content_type, "application/json");
            beast::ostream(response_.body()) << json;
        }
    }

    // Asynchronously transmit the response message.
    void
    write_response()
    {
        auto self = shared_from_this();

        response_.content_length(response_.body().size());

        http::async_write(
            socket_,
            response_,
            [self](beast::error_code ec, std::size_t)
            {
                self->socket_.shutdown(tcp::socket::shutdown_send, ec);
                self->deadline_.cancel();
            });
    }

    // Check whether we have spent enough time on this connection.
    void
    check_deadline()
    {
        auto self = shared_from_this();

        deadline_.async_wait(
            [self](beast::error_code ec)
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
void
http_server(tcp::acceptor& acceptor, tcp::socket& socket)
{
  acceptor.async_accept(socket,
      [&](beast::error_code ec)
      {
          if(!ec)
              std::make_shared<http_connection>(std::move(socket))->start();
          http_server(acceptor, socket);
      });
}

int
main(int argc, char* argv[])
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

        auto const address = net::ip::make_address(argv[1]);
        unsigned short port = static_cast<unsigned short>(std::atoi(argv[2]));

        net::io_context ioc{1};

        tcp::acceptor acceptor{ioc, {address, port}};
        tcp::socket socket{ioc};
        http_server(acceptor, socket);

        ioc.run();
    }
    catch(std::exception const& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}