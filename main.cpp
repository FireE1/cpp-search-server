#include "search-server/search_server.h"
#include "search-server/process_queries.h"

#include <string>
#include <iostream>
#include <execution>

void PrintDocument(const Document& document) {
    std::cout << "{ "s
         << "document_id = "s << document.id << ", "s
         << "relevance = "s << document.relevance << ", "s
         << "rating = "s << document.rating << " }"s << std::endl;
}

int main() {
    SearchServer search_server("and with"s);
    int id = 0;
    for (
        const std::string& text : {
            "white cat and yellow hat"s,
            "curly cat curly tail"s,
            "nasty dog with big eyes"s,
            "nasty pigeon john"s,
        }
    ) {
        search_server.AddDocument(++id, text, DocumentStatus::ACTUAL, {1, 2});
    }
    std::cout << "ACTUAL by default:"s << std::endl;
    // последовательная версия
    for (const Document& document : search_server.FindTopDocuments("curly nasty cat"s)) {
        PrintDocument(document);
    }
    std::cout << "BANNED:"s << std::endl;
    // последовательная версия
    for (const Document& document : search_server.FindTopDocuments(std::execution::seq, "curly nasty cat"s, DocumentStatus::BANNED)) {
        PrintDocument(document);
    }
    std::cout << "Even ids:"s << std::endl;
    // параллельная версия
    for (const Document& document : search_server.FindTopDocuments(std::execution::par, "curly nasty cat"s, [](int document_id, DocumentStatus status, int rating) { return document_id % 2 == 0; })) {
        PrintDocument(document);
    }
    return 0;
}