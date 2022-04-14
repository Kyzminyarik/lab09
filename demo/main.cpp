#include <header.hpp>
#include <boost/program_options.hpp>

//#include <boost/beast/core.hpp>
//#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
//#include <boost/beast/version.hpp>
//#include <boost/asio/connect.hpp>
//#include <boost/asio/ip/tcp.hpp>
//#include <boost/asio/ssl/error.hpp>
//#include <boost/asio/ssl/stream.hpp>
//#include <boost/asio/ssl/detail/engine.hpp>
//#include <boost/asio/detail/config.hpp>
//#include <boost/asio/ssl/context.hpp>
//#include <boost/asio/ssl/detail/impl/engine.ipp>
#include "root_certificates.hpp"
//#include <cstdlib>
//#include <iostream>

namespace po = boost::program_options;
using tcp = boost::asio::ip::tcp;
namespace ssl = boost::asio::ssl;
namespace http = boost::beast::http;
namespace beast = boost::beast;

void parse_url(std::string url,
               std::string& protocol,
               std::string& host, std::string& target){
  size_t dd_pos = url.find_first_of(':');
  protocol = url.substr(0, dd_pos);
  size_t start = url.find('/', dd_pos + 3);
  if (start == std::string::npos) {
    host = url.substr(dd_pos + 3, url.size() - dd_pos - 2);
    target = "/";
  } else {
    host = url.substr(dd_pos + 3, start - dd_pos - 3);
    target = url.substr(start, url.size() - start + 1);
  }
}

std::string download_http_page(const std::string& host, const std::string& target){
  boost::asio::io_context ioc{};

  boost::asio::ip::tcp::resolver resolver(ioc);
  boost::beast::tcp_stream stream(ioc);

  auto const results = resolver.resolve(host, "80");

  stream.connect(results);
  stream.expires_after(std::chrono::seconds(3));

  boost::beast::http::request<boost::beast::http::string_body> req{
      boost::beast::http::verb::get, target, 11};
  req.set(boost::beast::http::field::host, host);
  req.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);

  boost::beast::http::write(stream, req);

  boost::beast::flat_buffer buffer;
  boost::beast::http::response<boost::beast::http::string_body> res;

  boost::beast::http::read(stream, buffer, res);

  boost::beast::error_code ec;
  stream.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);

  return res.body();
}


//std::string download_https_page(const std::string& host, const std::string& target){
//  boost::asio::io_context ioc{};
//
//  boost::asio::ssl::context ctx(boost::asio::ssl::context::tls_client);
//  ctx.set_default_verify_paths();
//  ctx.set_verify_mode(boost::asio::ssl::verify_peer);
//
//  boost::asio::ip::tcp::resolver resolver(ioc);
//  boost::beast::ssl_stream<boost::beast::tcp_stream> stream(ioc, ctx);
//
//  if (!SSL_set_tlsext_host_name(stream.native_handle(), host.data())) {
//    boost::beast::error_code ec{static_cast<int>(::ERR_get_error()),
//                                boost::asio::error::get_ssl_category()};
//    throw boost::beast::system_error{ec};
//  }
//
//  auto const results = resolver.resolve(host, "443");
//
//  boost::beast::get_lowest_layer(stream).connect(results);
//
//  stream.handshake(boost::asio::ssl::stream_base::client);
//
//  boost::beast::http::request<boost::beast::http::string_body> req{
//      boost::beast::http::verb::get, target, 11};
//  req.set(boost::beast::http::field::host, host);
//  req.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);
//
//  boost::beast::http::write(stream, req);
//
//  boost::beast::flat_buffer buffer;
//  boost::beast::http::response<boost::beast::http::string_body> res;
//
//  boost::beast::http::read(stream, buffer, res);
//
//  boost::beast::error_code ec;
//  stream.shutdown(ec);
//
//  return res.body();
//}

std::string download_https_page(const std::string& host, const std::string& target) {
  boost::asio::io_context ioc;

  // The SSL context is required, and holds certificates
  ssl::context ctx(ssl::context::tlsv12_client);

  // This holds the root certificate used for verification
  load_root_certificates(ctx);

  // Verify the remote server's certificate
  ctx.set_verify_mode(ssl::verify_peer);

  // These objects perform our I/O
  tcp::resolver resolver(ioc);
  beast::ssl_stream<beast::tcp_stream> stream(ioc, ctx);

  // Set SNI Hostname (many hosts need this to handshake successfully)
  if(!SSL_set_tlsext_host_name(stream.native_handle(), host.data()))
  {
    beast::error_code ec{static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category()};
    throw beast::system_error{ec};
  }

  // Look up the domain name
  auto const results = resolver.resolve(host, "443");

  // Make the connection on the IP address we get from a lookup
  beast::get_lowest_layer(stream).connect(results);

  // Perform the SSL handshake
  stream.handshake(ssl::stream_base::client);

  // Set up an HTTP GET request message
  http::request<http::string_body> req{http::verb::get, target, 11};
  req.set(http::field::host, host);
  req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

  // Send the HTTP request to the remote host
  http::write(stream, req);

  // This buffer is used for reading and must be persisted
  beast::flat_buffer buffer;

  // Declare a container to hold the response
  boost::beast::http::response<boost::beast::http::string_body> res;

  // Receive the HTTP response
  http::read(stream, buffer, res);
  return res.body();
}


int main(int argc, char* argv[]) {
  
  po::options_description desc("Option");
  desc.add_options()("help", "Help message")
                    ("url,u", po::value<std::string>(), "HTML page addr")
                    ("depth,d", po::value<size_t>(), "Search depth")
                    ("network_threads,n", po::value<size_t>(), "Producer threads number")
                    ("parser_threads,p", po::value<size_t>(), "Consumer threads number")
                    ("output,o", po::value<std::string>(), "Path to output file");
  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);

  if (vm.count("help")){
    std::cout << desc << "\n";
    return 0;
  }
  
  std::string url;
  if (vm.count("url")){
    if (vm.at("url").as<std::string>().empty()){
      throw std::runtime_error("empty url");
    } else {
      url = vm.at("url").as<std::string>();
    }
  }
  size_t depth;
  if (vm.count("depth")){
    depth = vm.at("depth").as<size_t>();
  } else {
    throw std::runtime_error("empty depth");
  }
size_t network_threads;
  if (vm.count("network_threads")){
    network_threads = vm.at("network_threads").as<size_t>();
  } else {
    throw std::runtime_error("empty network_threads");
  }
size_t parser_threads;
  if (vm.count("parser_threads")){
    parser_threads = vm.at("parser_threads").as<size_t>();
  } else {
    throw std::runtime_error("empty parser_threads");
  }
std::string output;
  if (vm.count("output")){
    if (vm.at("output").as<std::string>().empty()){
      throw std::runtime_error("empty output");
    } else {
      output = vm.at("output").as<std::string>();
    }
  }

  std::string protocol;
  std::string host;
  std::string target;
  parse_url(url, protocol, host, target);
//  std::cout << protocol << " " << host << " " << target << " " << std::endl;
  std::string res = download_http_page(host, target);
  std::cout << res << std::endl;

return 0;
}