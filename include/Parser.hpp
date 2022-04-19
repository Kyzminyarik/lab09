//
// Created by pvelp on 4/15/22.
//

#ifndef TEMPLATE_PARSER_HPP
#define TEMPLATE_PARSER_HPP
#include <atomic>
#include <header.hpp>
#include <iostream>

struct URL{
  std::string url;
  size_t depth;
};

class Parser{
 public:
  Parser() = delete;
  static void parse(size_t &f);
  static Queue<URL> queue_url;
  static Queue<std::string> fs;
//  static std::atomic<int> in_work;
};

#endif  // TEMPLATE_PARSER_HPP
