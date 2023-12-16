#pragma once

#include <functional>
#include <set>
#include <string>
#include <string_view>
#include <vector>

using namespace std::string_literals;

std::vector<std::string_view> SplitIntoWords(std::string_view text);

template <typename StringContainer>
std::set<std::string, std::less<>>
MakeUniqueNonEmptyStrings(const StringContainer &strings) {
  std::set<std::string, std::less<>> non_empty_strings;
  std::string to_str;
  for (auto &str : strings) {
    to_str = str;
    if (!to_str.empty()) {
      non_empty_strings.insert(to_str);
    }
  }
  return non_empty_strings;
}
