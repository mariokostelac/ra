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
  const string get_or_store_string(const string key, const string value);

  const int get_int(const string key);
  const int get_or_store_int(const string key, const int value);

  const long long int get_long_int(const string key);
  const long long int get_or_store_long_int(const string key, const long long int value);

  const double get_double(const string key);
  const double get_or_store_double(const string key, const double value);

  const bool setting_exists(const string key);
  const bool setting_exists(const string key, const bool value);

  private:
  void put(const string key, const string value);
  const string get(const string key);
  const string storage_key(const string key);

  map<string, string> store_;
};
