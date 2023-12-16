#include "string_processing.h"

std::vector<std::string_view> SplitIntoWords(std::string_view text) {
  std::vector<std::string_view> words;
  size_t first = text.find_first_not_of(' ');
  size_t last = text.npos;
  for (size_t space = text.find(' ', first); first != last;
       first = text.find_first_not_of(' ', space),
              space = text.find(' ', first)) {
    words.push_back(space == last ? text.substr(first)
                                  : text.substr(first, space - first));
  }
  return words;
}
