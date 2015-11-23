#pragma once

#include <map>
#include <string>

using std::map;
using std::string;

class Settings {
  public:

  void load_settings(FILE *src);
  void dump_settings(FILE *dst);
  const string get_string(const string key);
  const int get_int(const string key);
  const long long int get_long_int(const string key);
  const double get_double(const string key);
  const bool setting_exists(const string key);

  private:
  const string store_key(const string user_key);

  map<string, string> store_;
};
