#include "search_server.h"

void SearchServer::AddDocument(int document_id, const std::string& document, DocumentStatus status,
                    const std::vector<int>& ratings) {
    if ((document_id < 0) || (documents_.count(document_id) > 0)) {
        throw std::invalid_argument("Invalid document_id"s);
    }
    const auto words = SplitIntoWordsNoStop(document);

    const double inv_word_count = 1.0 / words.size();
    for (const std::string& word : words) {
        word_to_document_freqs_[word][document_id] += inv_word_count;
        id_to_document_freqs_[document_id][word] += inv_word_count;
        words_in_doc[document_id].insert(word);
    }
    documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status});
    document_ids_.insert(document_id);
}

std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query, DocumentStatus status) const {
        return FindTopDocuments(
            raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
                return document_status == status;
            });
    }

std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query) const {
    return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
}

int SearchServer::GetDocumentCount() const {
    return documents_.size();
}
/* Устарело
int SearchServer::GetDocumentId(int index) const {
    return document_ids_.at(index);
}
*/

std::set<int>::const_iterator SearchServer::begin() {
        return document_ids_.begin();
    }

std::set<int>::const_iterator SearchServer::end() {
    return document_ids_.end();
}

void SearchServer::RemoveDocument(int document_id) {
    if (document_ids_.count(document_id))
    {
        for (auto [word ,fr] : id_to_document_freqs_[document_id])
        {
            word_to_document_freqs_[word].erase(document_id);
        }
        documents_.erase(document_id);
        id_to_document_freqs_.erase(document_id);
        document_ids_.erase(document_id);
    }
}

void SearchServer::RemoveDuplicates(SearchServer& search_server) {
    std::set<int> to_remove;
    for (auto i = begin(); i != end(); ++i)
    {
        auto doc = words_in_doc[*i];
        for (auto k = i; k != end(); ++k)
        {
            auto doc1 = words_in_doc[*k];
            if (i != k && doc.size() == doc1.size() && doc == doc1)
            {
                to_remove.insert(*k);
            }
        }
    }
    for (int i : to_remove)
    {
        std::cout << "Found duplicate document id " << i << std::endl;
        RemoveDocument(i);
    }
}

const std::map<std::string, double>& SearchServer::GetWordFrequencies(int document_id) const {
    std::map<std::string, double> empty;
    return !id_to_document_freqs_.count(document_id) ? empty : id_to_document_freqs_.at(document_id);
}

std::tuple<std::vector<std::string>, DocumentStatus> SearchServer::MatchDocument(const std::string& raw_query,
                                                        int document_id) const {
    //LOG_DURATION("MatchDocument"s);
    
    const auto query = ParseQuery(raw_query);

    std::vector<std::string> matched_words;
    for (const std::string& word : query.plus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            matched_words.push_back(word);
        }
    }
    for (const std::string& word : query.minus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            matched_words.clear();
            break;
        }
    }
    return {matched_words, documents_.at(document_id).status};
}

bool SearchServer::IsStopWord(const std::string& word) const {
    return stop_words_.count(word) > 0;
}

std::vector<std::string> SearchServer::SplitIntoWordsNoStop(const std::string& text) const {
    std::vector<std::string> words;
    for (const std::string& word : SplitIntoWords(text)) {
        if (!IsValidWord(word)) {
            throw std::invalid_argument("Word "s + word + " is invalid"s);
        }
        if (!IsStopWord(word)) {
            words.push_back(word);
        }
    }
    return words;
}

SearchServer::QueryWord SearchServer::ParseQueryWord(const std::string& text) const {
    if (text.empty()) 
    {
        throw std::invalid_argument("Query word is empty"s);
    }
    std::string word = text;
    bool is_minus = false;
    if (word[0] == '-')
    {
        is_minus = true;
        word = word.substr(1);
    }
    if (word.empty() || word[0] == '-' || !IsValidWord(word)) 
    {
        throw std::invalid_argument("Query word "s + text + " is invalid"s);
    }

    return {word, is_minus, IsStopWord(word)};
}

SearchServer::Query SearchServer::ParseQuery(const std::string& text) const {
    Query result;
    for (const std::string& word : SplitIntoWords(text)) 
    {
        const auto query_word = ParseQueryWord(word);
        if (!query_word.is_stop) 
        {
            if (query_word.is_minus) 
            {
                result.minus_words.insert(query_word.data);
            } 
            else 
            {
                result.plus_words.insert(query_word.data);
            }
        }
    }
    return result;
}

double SearchServer::ComputeWordInverseDocumentFreq(const std::string& word) const {
    return std::log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
}