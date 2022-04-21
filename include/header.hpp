// Copyright 2021 Your Name <your_email>

#ifndef INCLUDE_EXAMPLE_HPP_
#define INCLUDE_EXAMPLE_HPP_

#include <atomic>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <string>

#include "Downloader.hpp"
#include "Parser.hpp"
#include "mQueue.hpp"

// static struct URL{
//  std::string url;
//  size_t depth;
//};
//
//static Queue<URL> queue_url;
//static Queue<Page> queue_page;
//static Queue<std::string> fs;
//static size_t flag;
//extern std::atomic<int> p_w;
//extern std::atomic<int> d_w;
//static std::mutex mut;
//static std::mutex p_mut;
//static std::atomic<size_t> fl;

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
              std::shared_ptr<std::mutex>& u_mutex);

void parse(const Page& p,
          std::atomic<int> &f,
          std::queue<URL>& q_url,
          std::queue<std::string>& fs,
          std::shared_ptr<std::mutex>& mutex,
          std::shared_ptr<std::mutex>& fs_mutex);


//struct Page{
//  std::string html;
//  std::string protocol;
//  std::string host;
//  size_t depth;
//};
//
//struct URL{
//  std::string url;
//  size_t depth;
//};
#endif // INCLUDE_EXAMPLE_HPP_
