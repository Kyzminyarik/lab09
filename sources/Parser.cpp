//
// Created by pvelp on 4/15/22.
//
#include <thread>
#include <Parser.hpp>
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

void search_for_links(GumboNode* node, Page p) {
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
      tmp = p.protocol + p.host + href->value;
    } else {
      tmp = href->value;
    }

    if (isImage(tmp)) {
      Parser::fs.push(std::move(tmp));
    } else {
      if (p.depth == 1) return;
      URL url{tmp, p.depth - 1};
      Parser::queue_url.push(std::move(url));
    }
  }
  GumboVector* children = &node->v.element.children;
  for (unsigned int i = 0; i < children->length; ++i) {
    search_for_links(static_cast<GumboNode*>(children->data[i]), p);
  }
}

void Parser::parse(size_t &f) {
//  set_logs();
  try{
    std::cout << "parser thread: " << std::this_thread::get_id() << std::endl;
    if (!Downloader::queue_page.empty()) {
      Page tmp = Downloader::queue_page.front();
      GumboOutput* output = gumbo_parse(tmp.html.c_str());
      search_for_links(output->root, tmp);
      gumbo_destroy_output(&kGumboDefaultOptions, output);
//      queue_page.pop();
      std::cout << "Parse page from: " << tmp.protocol+ "://" +tmp.host+"/" << std::endl;
      std::cout << queue_url.get_count() << " в очереди ссылок" << std::endl;
      f--;
     }
  } catch (...){}
}