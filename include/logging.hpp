// Copyright 2022 pvelp

#ifndef INCLUDE_LOGGING_HPP_
#define INCLUDE_LOGGING_HPP_
#include <iostream>
#include <boost/log/exceptions.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup.hpp>
#include <boost/log/core.hpp>
namespace logs = boost::log;
namespace keywords = boost::log::keywords;

void set_logs();

#endif  // INCLUDE_LOGGING_HPP_
