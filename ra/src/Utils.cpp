/*!
 * @file Utils.cpp
 *
 * @brief Utils source file
 *
 * @author rvaser (robert.vaser@gmail.com)
 * @date Apr 25, 2015
 */

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#include "Utils.hpp"

void debug(const char* fmt, ...) {
#ifdef DEBUG
  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
#endif
}

Timer::Timer() :
    paused_(0), time_(0), timeval_() {
}

void Timer::start() {

    gettimeofday(&timeval_, nullptr);
    paused_ = 0;
}

void Timer::stop() {

    if (paused_) return;

    timeval stop;
    gettimeofday(&stop, nullptr);

    time_ += ((stop.tv_sec - timeval_.tv_sec) * 1000000L + stop.tv_usec) - timeval_.tv_usec;
    paused_ = 1;
}

void Timer::reset() {

    gettimeofday(&timeval_, nullptr);
    time_ = 0;
    paused_ = 0;
}

void Timer::print(const char* location, const char* message) const {
    fprintf(stderr, "[%s][%s]: %.5lf s\n", location, message, time_ / (double) 1000000);
}

std::string reverse_complement(const std::string& original) {
  std::string res;

  for (int i = original.size() - 1; i >= 0; --i) {

    char c = original[i];

    switch (c) {
      case 'A':
        c = 'T';
        break;
      case 'T':
        c = 'A';
        break;
      case 'C':
        c = 'G';
        break;
      case 'G':
        c = 'C';
        break;
      default:
        break;
    }

    res += c;
  }

  return res;
}
