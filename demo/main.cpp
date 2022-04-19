#include <header.hpp>
#include <boost/program_options.hpp>
#include <ThreadPool.hpp>
#include <fstream>
//#include <logging.hpp>
Queue<URL> Parser::queue_url;
Queue<Page> Downloader::queue_page;
Queue<std::string> Parser::fs;

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
//  set_logs();
//  Downloader::in_work = 0;
//  Parser::in_work = 0;
  size_t f = 0;
  ThreadPool pool_downloader(network_threads);
  ThreadPool pool_parser(parser_threads);
  URL URl{url, depth};
  Parser::queue_url.push(std::move(URl));
  Downloader::download(f);
  std::cout << "flag: " << f << std::endl;
  Parser::parse(f);
  std::cout << "flag: " << f << std::endl;

//  while (!Parser::queue_url.empty() || !Downloader::queue_page.empty() || f != 0){
//    pool_downloader.enqueue(Downloader::download, std::ref(f));
//    pool_parser.enqueue(Parser::parse, std::ref(f));
//  }

  std::ofstream ofs{output};
  while (!Parser::fs.empty()) {
    std::string tmp = Parser::fs.front();
    ofs << tmp << std::endl;
    std::cout << "Parse picture: " << tmp << std::endl;
//    Parser::fs.pop();
  }
  ofs.close();

return 0;
}