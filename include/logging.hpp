//
// Created by pvelp on 4/16/22.
//

#ifndef TEMPLATE_LOGGING_HPP
#define TEMPLATE_LOGGING_HPP
#include <iostream>
#include <boost/log/exceptions.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup.hpp>
#include <boost/log/core.hpp>
namespace logs = boost::log;
namespace keywords = boost::log::keywords;

void set_logs();

#endif  // TEMPLATE_LOGGING_HPP
