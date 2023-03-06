#pragma once

#include "search_server.h"
#include "test_example_functions.cpp"

void AddDocument(SearchServer& server, int document_id, const std::string& document, 
                DocumentStatus status, const std::vector<int>& ratings);