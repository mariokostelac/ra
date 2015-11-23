#include "Settings.hpp"
#include "Utils.hpp"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdio>
#include <cstring>

void Settings::load_settings(FILE *src) {
  char* buffer = new char[256];

  while (fgets(buffer, 256, src) != NULL) {
    // # is comment line
    if (buffer[0] == '#') {
      continue;
    }

    int len = strlen(buffer);
    int non_white_space_cnt = 0;
    for (int i = 0; i < len; ++i) {
      non_white_space_cnt += !isspace(buffer[i]);
    }
    if (non_white_space_cnt == 0) {
      continue;
    }

    // calculate positons
    char* key_end = strchr(buffer, ':');
    assert(key_end != NULL);

    int key_length = key_end - buffer;
    assert(key_length > 0);

    char* value_start = key_end + 1;
    int value_length = strlen(value_start) - 1;

    // lowercase the key
    for (int i = 0; i < key_length; ++i) {
      buffer[i] = tolower(buffer[i]);
    }

    string key(buffer, key_length);
    string value(key_end + 1, value_length);

    debug("settings.store %s -> %s\n", key.c_str(), value.c_str());
    store_[store_key(key)] = value;
  }

  delete[] buffer;
}

void Settings::dump_settings(FILE *dst) {
  for (auto kv : store_) {
    fprintf(dst, "%s: %s\n", kv.first.c_str(), kv.second.c_str());
  }
}

const bool Settings::setting_exists(const string key) {
  debug("settings.setting_exists %s\n", key.c_str());
  return store_.count(store_key(key)) > 0;
}

const string Settings::get_string(const string key) {
  debug("settings.get_string %s\n", key.c_str());
  return store_[store_key(key)];
}

const int Settings::get_int(const string key) {
  debug("settings.get_int %s\n", key.c_str());
  int result;
  sscanf(store_[store_key(key)].c_str(), " %d ", &result);
  return result;
}

const long long int Settings::get_long_int(const string key) {
  debug("settings.get_long_int %s\n", key.c_str());
  long long int result;
  sscanf(store_[store_key(key)].c_str(), " %lld ", &result);
  return result;
}

const double Settings::get_double(const string key) {
  debug("settings.get_double %s\n", key.c_str());
  double result;
  sscanf(store_[store_key(key)].c_str(), " %lf ", &result);
  return result;
}

const string Settings::store_key(const string user_key) {
  string result_key(user_key);
  std::transform(result_key.begin(), result_key.end(), result_key.begin(), ::tolower);
  return result_key;
}
