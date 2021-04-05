#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <cstdlib>
#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>
#include "../include/cpp-base64/base64.cpp"

namespace beast = boost::beast;     // from <boost/beast.hpp>
namespace http = beast::http;       // from <boost/beast/http.hpp>
namespace net = boost::asio;        // from <boost/asio.hpp>
using tcp = net::ip::tcp;           // from <boost/asio/ip/tcp.hpp>
using namespace cv;

using boost::property_tree::ptree; 
using boost::property_tree::read_json; 
using boost::property_tree::write_json;

// Performs an HTTP GET and prints the response
int main(int argc, char** argv)
{
    try
    {
        // Check command line arguments.
        if(argc != 7 && argc != 8)
        {
            std::cerr <<
                "Usage: ./client <input_image_dir> <new_width> <new_height> <host> <port> <target> [<HTTP version: 1.0 or 1.1(default)>]\n" <<
                "Example:\n" <<
                "    http-client-sync www.example.com 80 /\n" <<
                "    http-client-sync www.example.com 80 / 1.0\n";
            return EXIT_FAILURE;
        }
        auto const image_dir = argv[1];
        auto const new_width = argv[2];
        auto const new_height = argv[3];
        auto const host = argv[4];
        auto const port = argv[5];
        auto const target = argv[6];
        int version = argc == 8 && !std::strcmp("1.0", argv[4]) ? 10 : 11;

        // Load image
        Mat image;
        Mat new_image;
        // int new_width;
        // int new_height;
        // new_width = 100;
        // new_height = 100;
        // String image_dir = "../testdata/gundamcat.png";
        
        image = imread( image_dir, 1 );

        // convert to base64
        std::vector<uchar> bufimg;
        cv::imencode(".jpg", image, bufimg);
        auto *enc_msg = reinterpret_cast<unsigned char*>(bufimg.data());
        std::string encoded = base64_encode(enc_msg, bufimg.size());

        // The io_context is required for all I/O
        net::io_context ioc;

        // These objects perform our I/O
        tcp::resolver resolver(ioc);
        beast::tcp_stream stream(ioc);

        // Look up the domain name
        auto const results = resolver.resolve(host, port);

        // Make the connection on the IP address we get from a lookup
        stream.connect(results);

        // Set up an HTTP GET request message
        // http::request<http::string_body> req{http::verb::get, target, version};
        // http::request<http::string_body> req{http::verb::post, target, version};
        // req.set(http::field::host, host);
        // req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        ptree root, info;
        root.put ("input_jpeg", encoded);
        root.put ( "desired_width", new_width);
        root.put ( "desired_height", new_height);
        

        std::ostringstream buf; 
        write_json (buf, root, false);
        std::string json = buf.str();
        // std::string json_length = json.length();


        http::request<http::dynamic_body> req{http::verb::post, target, version};
        req.set(http::field::host, host);
        req.set(http::field::content_type, "application/json");
        beast::ostream(req.body()) << json;

        std::cout << req << std::endl;

        // Send the HTTP request to the remote host
        http::write(stream, req);

        // This buffer is used for reading and must be persisted
        beast::flat_buffer buffer;

        // Declare a container to hold the response
        http::response<http::dynamic_body> res;

        // Receive the HTTP response
        http::read(stream, buffer, res);

        // Write the message to standard out
        std::cout << res << std::endl;

        // decode image from body
        auto res_body = res.body();
        printf("type : %s \n", typeid(res_body).name());

        // Gracefully close the socket
        beast::error_code ec;
        stream.socket().shutdown(tcp::socket::shutdown_both, ec);

        // not_connected happens sometimes
        // so don't bother reporting it.
        //
        if(ec && ec != beast::errc::not_connected)
            throw beast::system_error{ec};

        // If we get here then the connection is closed gracefully
    }
    catch(std::exception const& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}