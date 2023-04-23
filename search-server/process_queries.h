#pragma once

#include "document.h"
#include "search_server.cpp"

#include <vector>
#include <string>
#include <algorithm>
#include <execution>

std::vector<std::vector<Document>> ProcessQueries(const SearchServer& search_server, 
                                            const std::vector<std::string>& queries);

std::vector<Document> ProcessQueriesJoined(const SearchServer& search_server,
                                    const std::vector<std::string>& queries);