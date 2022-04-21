#include "header.hpp"
#include <boost/program_options.hpp>
#include <ThreadPool.hpp>
#include <fstream>
#include <atomic>

namespace po = boost::program_options;

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
  std::atomic<int> fl = 0;

  std::shared_ptr<std::mutex> u_mutex = std::make_shared<std::mutex>();
  std::shared_ptr<std::mutex> p_mutex = std::make_shared<std::mutex>();
  std::shared_ptr<std::mutex> f_mutex = std::make_shared<std::mutex>();

  std::queue<URL> q_url;
  std::queue<Page> q_page;
  std::queue<std::string> fs;

  ThreadPool pool_downloader(network_threads);
  ThreadPool pool_parser(parser_threads);

  URL URl{url, depth};
  q_url.push(std::move(URl));
//  download(fl, q_url, q_page, url_mutex, page_mutex);
//  std::cout << "flag: " << fl << std::endl;
//  parse(fl, q_url, q_page, fs, url_mutex, page_mutex, fs_mutex);
//  std::cout << "flag: " << fl << std::endl;

  while (!q_url.empty() || !q_page.empty() || fl != 0) {
    if (!q_url.empty()) {
      u_mutex->lock();
      URL data = q_url.front();
      q_url.pop();
      u_mutex->unlock();
      fl++;
      pool_downloader.enqueue(
          [&fl, data, &q_page, &p_mutex] { download(fl, data, q_page, p_mutex); });
    }
    if (!q_page.empty()) {
      p_mutex->lock();
      Page p = q_page.front();
      q_page.pop();
      p_mutex->unlock();
      pool_parser.enqueue([p, &fl, &q_url, &fs, &p_mutex, &f_mutex]{
        parse(p, fl, q_url, fs, p_mutex, f_mutex);
      });
    }
  }

  std::ofstream ofs{output};
  while (!fs.empty()) {
    std::string tmp = fs.front();
    fs.pop();
    ofs << tmp << std::endl;
    std::cout << "Parse picture: " << tmp << std::endl;
  }
  ofs.close();

return 0;
}