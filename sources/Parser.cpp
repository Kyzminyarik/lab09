// Copyright 2022 pvelps
#include <thread>
#include <algorithm>
#include <gumbo.h>
#include <regex>
#include <header.hpp>
#include "logging.hpp"

bool isImage(const std::string& href) {
  size_t lastDotPos = href.find_last_of('.');
  if (lastDotPos == std::string::npos) return false;
  std::string ext = href.substr(lastDotPos + 1, href.size() - 1);

  static const std::vector<std::string> imageExtensions = {
      "apng", "avif", "gif",  "jpg", "jpeg", "jfif", "pjpeg", "pjp",
      "png",  "svg",  "webp", "bmp", "ico",  "cur",  "tif",   "tiff"};

  return std::any_of(imageExtensions.begin(), imageExtensions.end(),
                     [&](const std::string& s) { return ext == s; });
}

void search_for_links(GumboNode* node, Page p,
                      std::queue<URL>& q_url,
                      std::queue<std::string>& fs,
                      std::mutex& mutex,
                      std::mutex& fs_mutex) {
  if (node->type != GUMBO_NODE_ELEMENT) {
    return;
  }
  GumboAttribute* href = nullptr;

  if (gumbo_get_attribute(&node->v.element.attributes, "href"))
    href = gumbo_get_attribute(&node->v.element.attributes, "href");
  else if (gumbo_get_attribute(&node->v.element.attributes, "content"))
    href = gumbo_get_attribute(&node->v.element.attributes, "content");
  else if (node->v.element.tag == GUMBO_TAG_IMAGE ||
           node->v.element.tag == GUMBO_TAG_IMG)
    href = gumbo_get_attribute(&node->v.element.attributes, "src");

  if (href) {
    std::regex rx(R"((^http[s]?://.*)|(/.*))");
    std::string tmp = href->value;
    if (!regex_match(tmp.begin(), tmp.end(), rx)) return;
    //EXAMPLE:
    //<href="/images/icons/pwa/apple/default.png?15">
    if (href->value[0] == '/') {
      tmp = p.protocol + "://" + p.host + href->value;
    } else {
      tmp = href->value;
    }

    if (isImage(tmp)) {
      fs_mutex.lock();
      fs.push(std::move(tmp));
      fs_mutex.unlock();
    } else {
      if (p.depth == 1) return;
      URL url{tmp, p.depth - 1};
      mutex.lock();
      q_url.push(std::move(url));
      mutex.unlock();
    }
  }
  GumboVector* children = &node->v.element.children;
  for (unsigned int i = 0; i < children->length; ++i) {
    search_for_links(static_cast<GumboNode*>(children->data[i]), p,
                     q_url, fs, mutex, fs_mutex);
  }
}

void parse(const Page& p,
           std::atomic<int> &f,
           std::queue<URL>& q_url,
           std::queue<std::string>& fs,
           std::mutex& mutex,
           std::mutex& fs_mutex) {

    GumboOutput* output = gumbo_parse(p.html.c_str());
    search_for_links(output->root, p, q_url, fs, mutex, fs_mutex);
    gumbo_destroy_output(&kGumboDefaultOptions, output);
    std::cout << q_url.size() << " в очереди ссылок" << std::endl;
    f--;
    std::cout << "f =  " << f << std::endl;
    BOOST_LOG_TRIVIAL(trace) << "Parse page form: "
                             << p.protocol + "://" + p.host;
  }
