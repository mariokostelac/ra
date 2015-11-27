#include "Settings.hpp"
#include "Utils.hpp"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <cstdlib>

string to_string(const int num) {
  return std::to_string(num);
}

string to_string(const long long int num) {
  return std::to_string(num);
}

string to_string(const double num) {
  return std::to_string(num);
}

int to_int(const string s) {
  return atoi(s.c_str());
}

long long int to_llint(const string s) {
  return atol(s.c_str());
}

double to_double(const string s) {
  return atof(s.c_str());
}

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
    put(key, value);
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
  return store_.count(storage_key(key)) > 0;
}

const string Settings::get_string(const string key) {
  debug("settings.get_string %s\n", key.c_str());
  return get(key);
}

const string Settings::get_or_store_string(const string key, const string value) {
  debug("settings.get_or_store_string %s %s\n", key.c_str(), value.c_str());
  if (setting_exists(key)) {
    return get(key);
  }

  put(key, value);
  return get(key);
}

const int Settings::get_int(const string key) {
  debug("settings.get_int %s\n", key.c_str());
  return to_int(get(key));
}

const int Settings::get_or_store_int(const string key, const int value) {
  debug("settings.get_or_store_int %s %d\n", key.c_str(), value);
  if (setting_exists(key)) {
    return to_int(get(key));
  }

  put(key, to_string(value));
  return value;
}

const long long int Settings::get_long_int(const string key) {
  debug("settings.get_long_int %s\n", key.c_str());
  return to_llint(get(key));
}

const long long int Settings::get_or_store_long_int(const string key, const long long int value) {
  debug("settings.get_or_store_long_int %s %lld\n", key.c_str(), value);
  if (setting_exists(key)) {
    return to_llint(get(key));
  }

  put(key, to_string(value));
  return value;
}

const double Settings::get_double(const string key) {
  debug("settings.get_double %s\n", key.c_str());
  return to_double(get(key));
}

const double Settings::get_or_store_double(const string key, const double value) {
  debug("settings.get_or_store_double %s\n", key.c_str(), value);
  if (setting_exists(key)) {
    return to_double(get(key));
  }

  put(key, to_string(value));
  return value;
}

void Settings::put(const string key, const string value) {
  store_[storage_key(key)] = value;
}

const string Settings::get(const string key) {
  return store_[storage_key(key)];
}

const string Settings::storage_key(const string key) {
  string storage_key(key);
  std::transform(storage_key.begin(), storage_key.end(), storage_key.begin(), ::tolower);
  return storage_key;
}
