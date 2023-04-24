#include "search_server.h"

void SearchServer::AddDocument(int document_id, std::string_view document, DocumentStatus status,
                    const std::vector<int>& ratings) {
    if ((document_id < 0) || (documents_.count(document_id) > 0)) 
    {
        throw std::invalid_argument("Invalid document_id"s);
    }
    auto [get_data, _] = documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status, std::string(document)});
    const auto words = SplitIntoWordsNoStop(get_data->second.data);

    const double inv_word_count = 1.0 / words.size();
    for (std::string_view word : words)
    {
        word_to_document_freqs_[word][document_id] += inv_word_count;
        id_to_document_freqs_[document_id][word] += inv_word_count;
    }
    document_ids_.insert(document_id);
}

std::vector<Document> SearchServer::FindTopDocuments(std::string_view raw_query, DocumentStatus status) const {
        return FindTopDocuments(
            raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
                return document_status == status;
            });
    }

std::vector<Document> SearchServer::FindTopDocuments(std::string_view raw_query) const {
    return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
}

int SearchServer::GetDocumentCount() const {
    return documents_.size();
}

std::set<int>::const_iterator SearchServer::begin() {
        return document_ids_.begin();
    }

std::set<int>::const_iterator SearchServer::end() {
    return document_ids_.end();
}

void SearchServer::RemoveDocument(int document_id) {
    RemoveDocument(std::execution::seq, document_id);
}

void SearchServer::RemoveDocument(const std::execution::sequenced_policy&, int document_id) {
    if (document_ids_.count(document_id))
    {
        for_each(std::execution::seq, word_to_document_freqs_.begin(), word_to_document_freqs_.end(), 
                        [document_id](auto& i){i.second.erase(document_id);});
        documents_.erase(document_id);
        id_to_document_freqs_.erase(document_id);
        document_ids_.erase(document_id);
    }
}

void SearchServer::RemoveDocument(const std::execution::parallel_policy&, int document_id) {
    if (document_ids_.count(document_id))
    {
        for_each(std::execution::par, word_to_document_freqs_.begin(), word_to_document_freqs_.end(), 
                        [document_id](auto& i){i.second.erase(document_id);});
        documents_.erase(document_id);
        id_to_document_freqs_.erase(document_id);
        document_ids_.erase(document_id);
    }
}

const std::map<std::string_view, double>& SearchServer::GetWordFrequencies(int document_id) const {
    std::map<std::string_view, double> empty;
    return !id_to_document_freqs_.count(document_id) ? empty : id_to_document_freqs_.at(document_id);
}

std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(std::string_view raw_query,
                                                        int document_id) const {
    const auto query = ParseQuery(raw_query, true);

    std::vector<std::string_view> matched_words;
    for (std::string_view word : query.minus_words)
    {
        if (word_to_document_freqs_.count(word) == 0) 
        {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) 
        {
            return {matched_words, documents_.at(document_id).status};
        }
    }
    for (std::string_view word : query.plus_words)
    {
        if (word_to_document_freqs_.count(word) == 0) 
        {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) 
        {
            matched_words.push_back(word);
        }
    }
    
    return {matched_words, documents_.at(document_id).status};
}

std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(const std::execution::sequenced_policy&,
                                                                    std::string_view raw_query, int document_id) const {
    return MatchDocument(raw_query, document_id);
}

std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(const std::execution::parallel_policy&,
                                                                    std::string_view raw_query, int document_id) const {
    const auto query = ParseQuery(raw_query, false);

    if (std::any_of(std::execution::par, query.minus_words.begin(), query.minus_words.end(),
            [document_id, this](std::string_view minus)
            {const auto& find = word_to_document_freqs_.find(minus); 
            return (find != word_to_document_freqs_.end()) && find->second.count(document_id);}))
    {
        std::vector<std::string_view> empty;
        return {empty, documents_.at(document_id).status};
    }
    std::vector<std::string_view> matched_words(query.plus_words.size());
    auto iter = std::copy_if(std::execution::par, query.plus_words.begin(), query.plus_words.end(), matched_words.begin(),
            [document_id, this](std::string_view plus)
            {const auto& find = word_to_document_freqs_.find(plus); 
            return (find != word_to_document_freqs_.end()) && find->second.count(document_id);});
    std::sort(std::execution::par, matched_words.begin(), iter);
    matched_words.erase(std::unique(std::execution::par, matched_words.begin(), iter), matched_words.end());
    return {matched_words, documents_.at(document_id).status};
}

bool SearchServer::IsStopWord(std::string_view word) const {
    return stop_words_.count(word) > 0;
}

std::vector<std::string_view> SearchServer::SplitIntoWordsNoStop(std::string_view text) const {
    std::vector<std::string_view> words;
    for (std::string_view& word : SplitIntoWords(text))
    {
        if (!IsValidWord(word))
        {
            throw std::invalid_argument("Word "s + std::string(word) + " is invalid"s);
        }
        if (!IsStopWord(word))
        {
            words.push_back(word);
        }
    }
    return words;
}

SearchServer::QueryWord SearchServer::ParseQueryWord(std::string_view text) const {
    if (text.empty()) 
    {
        throw std::invalid_argument("Query word is empty"s);
    }
    bool is_minus = false;
    if (text[0] == '-')
    {
        is_minus = true;
        text = text.substr(1);
    }
    if (text.empty() || text[0] == '-' || !IsValidWord(text))
    {
        throw std::invalid_argument("Query word "s + std::string(text) + " is invalid"s);
    }

    return {text, is_minus, IsStopWord(text)};
}

SearchServer::Query SearchServer::ParseQuery(std::string_view text, bool seq) const {
    Query result;
    for (std::string_view word : SplitIntoWords(text))
    {
        const auto query_word = ParseQueryWord(word);
        if (!query_word.is_stop)
        {
            if (query_word.is_minus)
            {
                result.minus_words.push_back(query_word.data);
            } 
            else 
            {
                result.plus_words.push_back(query_word.data);
            }
        }
    }

    if(seq)
    {
        std::sort(std::execution::seq, result.minus_words.begin(), result.minus_words.end());
        std::sort(std::execution::seq, result.plus_words.begin(),  result.plus_words.end());
        result.minus_words.erase(std::unique(std::execution::seq, result.minus_words.begin(), result.minus_words.end()), result.minus_words.end());
        result.plus_words.erase (std::unique(std::execution::seq, result.plus_words.begin(),  result.plus_words.end()),  result.plus_words.end());
    }

    return result;
}

double SearchServer::ComputeWordInverseDocumentFreq(std::string_view& word) const {
    return std::log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
}