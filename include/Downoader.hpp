//
// Created by pvelp on 4/15/22.
//

#ifndef TEMPLATE_DOWNOADER_HPP
#define TEMPLATE_DOWNOADER_HPP
#include <iostream>
#include <string>
#include <atomic>
#include "Parser.hpp"

struct Page{
  std::string html;
  std::string protocol;
  std::string host;
  size_t depth;
};

class Downloader{
 public:
  Downloader() = delete;
  static void download(size_t& f);
  static Queue<Page> queue_page;

//  static std::atomic<int> in_work;
 private:
  static std::string downloadHttpPage(const std::string& host, const std::string& target);
  static std::string downloadHttpsPage(const std::string& host, const std::string& target);
};

#endif  // TEMPLATE_DOWNOADER_HPP
