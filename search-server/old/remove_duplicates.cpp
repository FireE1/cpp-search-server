#include "remove_duplicates.h"

void RemoveDuplicates(SearchServer& search_server) {
    std::set<int> to_remove;
    std::set<std::set<std::string>> words;
    for (const int id : search_server)
    {
        std::map<std::string, double> words_and_fr = search_server.GetWordFrequencies(id);
        std::set<std::string> doc_words;
        for (auto [word, fr] : words_and_fr)
        {
            doc_words.insert(word);
        }
        if (words.count(doc_words))
        {
            to_remove.insert(id);
        }
        else
        {
            words.insert(doc_words);
        }
    }
    for (int id : to_remove)
    {
        std::cout << "Found duplicate document id " << id << std::endl;
        search_server.RemoveDocument(id);
    }
}