// Copyright 2021 Your Name <your_email>

#ifndef INCLUDE_EXAMPLE_HPP_
#define INCLUDE_EXAMPLE_HPP_

#include <atomic>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <string>

struct URL{
  std::string url;
  size_t depth;
};

struct Page{
  std::string html;
  std::string protocol;
  std::string host;
  size_t depth;
};

void download(std::atomic<int> &f,
              const URL& url,
              std::queue<Page>& q_page,
              std::mutex& u_mutex);

void parse(const Page& p,
          std::atomic<int> &f,
          std::queue<URL>& q_url,
          std::queue<std::string>& fs,
          std::mutex& mutex,
          std::mutex& fs_mutex);

#endif // INCLUDE_EXAMPLE_HPP_
