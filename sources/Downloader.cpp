// Copyright 2021 Your Name <your_email>

#include <Downoader.hpp>
#include <header.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include "root_certificates.hpp"
#include <regex>
#include <thread>
#include <chrono>
//#include "logging.hpp"

using tcp = boost::asio::ip::tcp;
namespace ssl = boost::asio::ssl;
namespace http = boost::beast::http;
namespace beast = boost::beast;

//std::atomic<int> Downloader::in_work = 0;

//Queue<URL> queue_url;
//Queue<Page> queue_page;

void parse_url(std::string url,
               std::string& protocol, std::string& host, std::string& target){
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


std::string Downloader::downloadHttpPage(const std::string &host,
                                         const std::string &target) {
  boost::asio::io_context ioc{};

  tcp::resolver resolver(ioc);
  beast::tcp_stream stream(ioc);

  auto const results = resolver.resolve(host, "80");

  stream.connect(results);
  stream.expires_after(std::chrono::seconds(3));

  http::request<boost::beast::http::string_body> req{
      http::verb::get, target, 11};
  req.set(http::field::host, host);
  req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

  http::write(stream, req);

  beast::flat_buffer buffer;
  http::response<boost::beast::http::string_body> res;

  http::read(stream, buffer, res);

  beast::error_code ec;
  stream.socket().shutdown(tcp::socket::shutdown_both, ec);

  return res.body();
}

std::string Downloader::downloadHttpsPage(const std::string &host, const std::string &target) {
  boost::asio::io_context ioc{};
  ssl::context ctx(ssl::context::tls_client);
//  ctx.set_default_verify_paths();
//  ctx.add_verify_path("/etc/ssl/certs/");
//  ctx.set_verify_mode(boost::asio::ssl::verify_peer);
  load_root_certificates(ctx);

  tcp::resolver resolver(ioc);
  beast::ssl_stream<beast::tcp_stream> stream(ioc, ctx);

  if (!SSL_set_tlsext_host_name(stream.native_handle(), host.data())) {
    beast::error_code ec{static_cast<int>(::ERR_get_error()),
                         boost::asio::error::get_ssl_category()};
    throw beast::system_error{ec};
  }

  auto const results = resolver.resolve(host, "443");

  beast::get_lowest_layer(stream).connect(results);

  stream.handshake(ssl::stream_base::client);

  http::request<boost::beast::http::string_body> req{
      http::verb::get, target, 11};
  req.set(http::field::host, host);
  req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

  http::write(stream, req);

  beast::flat_buffer buffer;
  beast::http::response<boost::beast::http::string_body> res;

  http::read(stream, buffer, res);

  beast::error_code ec;
  stream.shutdown(ec);

  return res.body();
}

void Downloader::download(size_t &f){
  f++;
  std::cout << "download thread: " << std::this_thread::get_id() << std::endl;
  if (!Parser::queue_url.empty()) {
    URL tmp = Parser::queue_url.front();
    std::regex rx(R"(^http[s]?://.*)");
    if (!regex_match(tmp.url.begin(), tmp.url.end(), rx)) {
      Parser::queue_url.pop();
      return;
    }
    std::string protocol;
    std::string host;
    std::string target;
    std::string html;
    parse_url(tmp.url, protocol, host, target);
    if (protocol == "http") {
      html = Downloader::downloadHttpPage(host, target);
    }
    if (protocol == "https") {
      html = Downloader::downloadHttpsPage(host, target);
    }
//    queue_url.pop();
    Page page{html, protocol, host, tmp.depth};
    queue_page.push(std::move(page));
    std::cout << "Download page from: " << tmp.url << std::endl;
    std::cout << queue_page.get_count() << " в очереди страниц" << std::endl;
//    std::cout << page.html << std::endl;
  }
}

