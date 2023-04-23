#pragma once

#include "search_server.h"

#include <string>
#include <vector>

void AddDocument(SearchServer& server, int document_id, const std::string& document, 
                DocumentStatus status, const std::vector<int>& ratings);