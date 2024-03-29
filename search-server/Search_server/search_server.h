#pragma once

#include "../Utility/concurrent_map.h"
#include "../Utility/document.h"
#include "../Utility/string_processing.h"

#include <algorithm>
#include <cmath>
#include <execution>
#include <iostream>
#include <map>
#include <numeric>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

constexpr int MAX_RESULT_DOCUMENT_COUNT = 5;
constexpr double EPSILON = 1e-6;

class SearchServer {
public:
  // Коснструкторы
  template <typename StringContainer>
  SearchServer(const StringContainer &stop_words);
  SearchServer(std::string_view &stop_words_text)
      : SearchServer(SplitIntoWords(stop_words_text)) {}
  SearchServer(const std::string &stop_words_text)
      : SearchServer(SplitIntoWords(stop_words_text)) {}

  // Добавление документа
  void AddDocument(int document_id, std::string_view document,
                   DocumentStatus status, const std::vector<int> &ratings);

  // Поиск Подходящих документов
  template <typename DocumentPredicate>
  std::vector<Document>
  FindTopDocuments(std::string_view raw_query,
                   DocumentPredicate document_predicate) const;
  std::vector<Document> FindTopDocuments(std::string_view raw_query,
                                         DocumentStatus status) const;
  std::vector<Document> FindTopDocuments(std::string_view raw_query) const;

  template <typename DocumentPredicate, typename Policy>
  std::vector<Document>
  FindTopDocuments(Policy policy, std::string_view raw_query,
                   DocumentPredicate document_predicate) const;
  template <typename Policy>
  std::vector<Document> FindTopDocuments(Policy policy,
                                         std::string_view raw_query,
                                         DocumentStatus status) const;
  template <typename Policy>
  std::vector<Document> FindTopDocuments(Policy policy,
                                         std::string_view raw_query) const;

  // Количество документов в памяти
  int GetDocumentCount() const;

  // Итерирование по id документов
  std::set<int>::const_iterator begin();
  std::set<int>::const_iterator end();

  // Удаление документа
  void RemoveDocument(int document_id);
  void RemoveDocument(const std::execution::sequenced_policy &,
                      int document_id);
  void RemoveDocument(const std::execution::parallel_policy &, int document_id);

  const std::map<std::string_view, double> &
  GetWordFrequencies(int document_id) const;

  // Получаем документ по запросу
  std::tuple<std::vector<std::string_view>, DocumentStatus>
  MatchDocument(std::string_view raw_query, int document_id) const;
  std::tuple<std::vector<std::string_view>, DocumentStatus>
  MatchDocument(const std::execution::sequenced_policy &,
                std::string_view raw_query, int document_id) const;
  std::tuple<std::vector<std::string_view>, DocumentStatus>
  MatchDocument(const std::execution::parallel_policy &,
                std::string_view raw_query, int document_id) const;

private:
  struct DocumentData {
    int rating;
    DocumentStatus status;
    std::string data;
  };
  const std::set<std::string, std::less<>> stop_words_;
  std::map<std::string_view, std::map<int, double>> word_to_document_freqs_;
  std::map<int, DocumentData> documents_;
  std::set<int> document_ids_;
  std::map<int, std::map<std::string_view, double>> id_to_document_freqs_;

  // Проверка на стоп-слово
  bool IsStopWord(std::string_view word) const;

  static bool IsValidWord(std::string_view word) {
    return std::none_of(word.begin(), word.end(),
                        [](char c) { return c >= '\0' && c < ' '; });
  }

  // Получаем слова, что не стоп-слова
  std::vector<std::string_view>
  SplitIntoWordsNoStop(std::string_view text) const;

  static int ComputeAverageRating(const std::vector<int> &ratings) {
    if (ratings.empty()) {
      return 0;
    }
    int rating_sum = std::accumulate(ratings.begin(), ratings.end(), 0);
    return rating_sum / static_cast<int>(ratings.size());
  }

  struct QueryWord {
    std::string_view data;
    bool is_minus;
    bool is_stop;
  };

  QueryWord ParseQueryWord(std::string_view text) const;

  struct Query {
    std::vector<std::string_view> plus_words;
    std::vector<std::string_view> minus_words;
  };

  Query ParseQuery(std::string_view text, bool seq) const;

  double ComputeWordInverseDocumentFreq(std::string_view &word) const;

  template <typename DocumentPredicate>
  std::vector<Document>
  FindAllDocuments(const Query &query,
                   DocumentPredicate document_predicate) const;

  template <typename DocumentPredicate, typename Policy>
  std::vector<Document>
  FindAllDocuments(Policy policy, const Query &query,
                   DocumentPredicate document_predicate) const;
};

template <typename StringContainer>
SearchServer::SearchServer(const StringContainer &stop_words)
    : stop_words_(MakeUniqueNonEmptyStrings(stop_words)) {
  if (!all_of(stop_words_.begin(), stop_words_.end(), IsValidWord)) {
    throw std::invalid_argument("Some of stop words are invalid"s);
  }
}

template <typename DocumentPredicate>
std::vector<Document>
SearchServer::FindTopDocuments(std::string_view raw_query,
                               DocumentPredicate document_predicate) const {
  const auto query = ParseQuery(raw_query, true);

  auto matched_documents = FindAllDocuments(query, document_predicate);

  sort(matched_documents.begin(), matched_documents.end(),
       [](const Document &lhs, const Document &rhs) {
         if (std::abs(lhs.relevance - rhs.relevance) < EPSILON) {
           return lhs.rating > rhs.rating;
         } else {
           return lhs.relevance > rhs.relevance;
         }
       });
  if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
    matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
  }

  return matched_documents;
}

template <typename DocumentPredicate, typename Policy>
std::vector<Document>
SearchServer::FindTopDocuments(Policy policy, std::string_view raw_query,
                               DocumentPredicate document_predicate) const {
  const auto query = ParseQuery(raw_query, true);

  auto matched_documents = FindAllDocuments(policy, query, document_predicate);

  sort(policy, matched_documents.begin(), matched_documents.end(),
       [](const Document &lhs, const Document &rhs) {
         if (std::abs(lhs.relevance - rhs.relevance) < EPSILON) {
           return lhs.rating > rhs.rating;
         } else {
           return lhs.relevance > rhs.relevance;
         }
       });

  if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
    matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
  }

  return matched_documents;
}

template <typename Policy>
std::vector<Document>
SearchServer::FindTopDocuments(Policy policy, std::string_view raw_query,
                               DocumentStatus status) const {
  return FindTopDocuments(
      policy, raw_query,
      [status](int document_id, DocumentStatus document_status, int rating) {
        return document_status == status;
      });
}

template <typename Policy>
std::vector<Document>
SearchServer::FindTopDocuments(Policy policy,
                               std::string_view raw_query) const {
  return FindTopDocuments(policy, raw_query, DocumentStatus::ACTUAL);
}

template <typename DocumentPredicate>
std::vector<Document>
SearchServer::FindAllDocuments(const Query &query,
                               DocumentPredicate document_predicate) const {
  std::map<int, double> document_to_relevance;
  for (std::string_view word : query.plus_words) {
    if (word_to_document_freqs_.count(word) == 0) {
      continue;
    }
    const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
    for (const auto [document_id, term_freq] :
         word_to_document_freqs_.at(word)) {
      const auto &document_data = documents_.at(document_id);
      if (document_predicate(document_id, document_data.status,
                             document_data.rating)) {
        document_to_relevance[document_id] += term_freq * inverse_document_freq;
      }
    }
  }

  for (std::string_view word : query.minus_words) {
    if (word_to_document_freqs_.count(word) == 0) {
      continue;
    }
    for (const auto [document_id, _] : word_to_document_freqs_.at(word)) {
      document_to_relevance.erase(document_id);
    }
  }

  std::vector<Document> matched_documents;
  for (const auto [document_id, relevance] : document_to_relevance) {
    matched_documents.push_back(
        {document_id, relevance, documents_.at(document_id).rating});
  }
  return matched_documents;
}

template <typename DocumentPredicate, typename Policy>
std::vector<Document>
SearchServer::FindAllDocuments(Policy policy, const Query &query,
                               DocumentPredicate document_predicate) const {
  const size_t par_for_con_map = 100;
  ConcurrentMap<int, double> document_to_relevance(par_for_con_map);

  std::for_each(policy, query.plus_words.begin(), query.plus_words.end(),
                [this, &document_predicate,
                 &document_to_relevance](std::string_view word) {
                  if (word_to_document_freqs_.count(word) == 0) {
                    return;
                  }
                  const double inverse_document_freq =
                      ComputeWordInverseDocumentFreq(word);
                  for (const auto [document_id, term_freq] :
                       word_to_document_freqs_.at(word)) {
                    const auto &document_data = documents_.at(document_id);
                    if (document_predicate(document_id, document_data.status,
                                           document_data.rating)) {
                      document_to_relevance[document_id].ref_to_value +=
                          term_freq * inverse_document_freq;
                    }
                  }
                });

  std::for_each(policy, query.minus_words.begin(), query.minus_words.end(),
                [&](std::string_view word) {
                  if (word_to_document_freqs_.count(word) == 0) {
                    return;
                  }
                  for (const auto [document_id, _] :
                       word_to_document_freqs_.at(word)) {
                    document_to_relevance.erase(document_id);
                  }
                });

  const auto &document_to_relevance_whole =
      document_to_relevance.BuildOrdinaryMap();

  std::vector<Document> matched_documents;
  for (const auto [document_id, relevance] : document_to_relevance_whole) {
    matched_documents.push_back(
        {document_id, relevance, documents_.at(document_id).rating});
  }
  return matched_documents;
}
