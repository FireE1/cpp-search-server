#pragma once

#include "../Utility/document.h"
#include "search_server.cpp"

#include <algorithm>
#include <execution>
#include <string>
#include <vector>

std::vector<std::vector<Document>>
ProcessQueries(const SearchServer &search_server,
               const std::vector<std::string> &queries);

std::vector<Document>
ProcessQueriesJoined(const SearchServer &search_server,
                     const std::vector<std::string> &queries);
