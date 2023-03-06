#include "test_example_functions.h"

void AddDocument(SearchServer& server, int document_id, const std::string& document, 
                DocumentStatus status, const std::vector<int>& ratings) {
    server.AddDocument(document_id, document, status, ratings);
}