#include <header.hpp>
#include <boost/program_options.hpp>
#include <ThreadPool.hpp>
#include <fstream>
namespace po = boost::program_options;
Queue<URL> queue_url;
Queue<Page> queue_page;
Queue<std::string> fs;

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

  ThreadPool pool_downloader(network_threads);
  ThreadPool pool_parser(parser_threads);
  URL URl{url, depth};
  queue_url.push(std::move(URl));

  while (!queue_url.empty() || !queue_page.empty() ||
         queue_page.get_count() || queue_url.get_count()) {
    pool_downloader.enqueue([] { Downloader::download(); });
    pool_parser.enqueue([] { Parser::parse(); });
  }

  std::ofstream ofs{output};
  while (!fs.empty()) {
    std::string _tmp = fs.front();
    ofs << _tmp << std::endl;
    std::cout << _tmp << std::endl;
    fs.pop();
  }
  ofs.close();

return 0;
}