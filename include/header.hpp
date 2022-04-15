// Copyright 2021 Your Name <your_email>

#ifndef INCLUDE_EXAMPLE_HPP_
#define INCLUDE_EXAMPLE_HPP_

#include <string>
#include <iostream>
#include <mQueue.hpp>
#include <Downoader.hpp>
#include <Parser.hpp>

struct Page{
  std::string html;
  std::string protocol;
  std::string host;
  size_t depth;
};

struct URL{
  std::string url;
  size_t depth;
};

extern Queue<URL> queue_url;
extern Queue<Page> queue_page;
extern Queue<std::string> fs;

#endif // INCLUDE_EXAMPLE_HPP_
