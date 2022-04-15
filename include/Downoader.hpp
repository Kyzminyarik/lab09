//
// Created by pvelp on 4/15/22.
//

#ifndef TEMPLATE_DOWNOADER_HPP
#define TEMPLATE_DOWNOADER_HPP
#include <iostream>
#include <string>

class Downloader{
 public:
  Downloader() = delete;
  static void download();
 private:
  static std::string downloadHttpPage(const std::string& host, const std::string& target);
  static std::string downloadHttpsPage(const std::string& host, const std::string& target);
};

#endif  // TEMPLATE_DOWNOADER_HPP
