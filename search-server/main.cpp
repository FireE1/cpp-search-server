#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <map>
#include <cmath>
#include <numeric>
using namespace std::string_literals;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

std::string ReadLine() {
    std::string s;
    getline(std::cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result = 0;
    std::cin >> result;
    ReadLine();
    return result;
}

std::vector<std::string> SplitIntoWords(const std::string& text) {         //делим строку на слова
    std::vector<std::string> words;
    std::string word;
    for (const char c : text) 
    {
        if (c == ' ') 
        {
            if (!word.empty()) 
            {
                words.push_back(word);
                word.clear();
            }
        } else 
        {
            word += c;
        }
    }
    if (!word.empty()) 
    {
        words.push_back(word);
    }
    return words;
}

struct Document {
    int id;
    double relevance;
    int rating;
};

struct Query {
    std::set<std::string> plus_word;
    std::set<std::string> minus_word;
};

struct MinusNotStop {
    std::string word;
    bool minus;
    bool stop;
};

enum DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED
};

class SearchServer {
    public:
        void SetStopWords(const std::string& text) {             //собираем стоп слова в кучу
            for (const std::string& word : SplitIntoWords(text)) 
            {
                stop_words_.insert(word);
            }
        }

        void AddDocument(int document_id, const std::string& document, DocumentStatus status, const std::vector<int>& rates) {             //распределяем слова, в каких документах они, их TF и их рейтинг
            const std::vector<std::string> words = SplitIntoWordsNoStop(document);                                                                    //(количество конкретных слов ко всем словам)
            for (const std::string& word : words)
            {
                word_to_documents_[word][document_id] += 1.0 / words.size();
            }
            ++document_count_;
            document_rt_[document_id] = ComputeAverageRating(rates);
            doc_st_[document_id] = status;
        }

        std::vector<Document> FindTopDocuments(const std::string& raw_query, DocumentStatus status = DocumentStatus::ACTUAL) const {              //ищем нужный документ по запросу
            const Query query_words = ParseQuery(raw_query);
            auto matched_documents = FindAllDocuments(query_words, status);
            std::sort(matched_documents.begin(), matched_documents.end(), [](const Document& lhs, const Document& rhs) {
                if(lhs.relevance != rhs.relevance){return lhs.relevance > rhs.relevance;}
                if(abs(lhs.relevance - rhs.relevance) < 1e-6){return lhs.rating > rhs.rating;}
                return lhs.relevance > rhs.relevance;});
            if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) 
            {
                matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
            }
            return matched_documents;
        }

    private:

    std::map<std::string, std::map<int, double>> word_to_documents_;
    std::set<std::string> stop_words_;
    int document_count_ = 0;
    std::map<int, int> document_rt_;
    std::map<int, DocumentStatus> doc_st_;
        
        bool IsStopWord(const std::string& word) const {             //является ли слово - стоп словом
            return stop_words_.count(word) > 0;
        }

        MinusNotStop ParseQueryForMinus(std::string word) const {                //проверка слова на минус и проверка на стоп
            bool minus = false;
            if (word[0] == '-') 
            {
                minus = true;
                word = word.substr(1);
            }
            return {word, minus, IsStopWord(word)};
        }

        Query ParseQuery(const std::string& text) const {                //разбираем запрос на слова(плюс и минус)
            Query query_words;
            for (const std::string& word : SplitIntoWords(text)) 
            {
                const MinusNotStop poss_minus = ParseQueryForMinus(word);
                if (!poss_minus.stop)
                {
                    if (poss_minus.minus)
                    {
                    query_words.minus_word.insert(poss_minus.word);
                    }
                    else
                    {
                    query_words.plus_word.insert(poss_minus.word);
                    }
                }
            }
            return query_words;
        }
    
        std::vector<std::string> SplitIntoWordsNoStop(const std::string& text) const {             //отсекаем, от множества слов, стоп слова
            std::vector<std::string> words;
            for (const std::string& word : SplitIntoWords(text)) 
            {
                if (stop_words_.count(word) == 0) 
                {
                    words.push_back(word);
                }
            }
            return words;
        }

        double GetIDF(const std::string& iter_word) const {
            return log(document_count_ * 1.0 / word_to_documents_.at(iter_word).size());
        }

        std::vector<Document> FindAllDocuments(const Query& query_words, DocumentStatus status) const {             //выделяем id и релевантность нужных нам документов
            std::vector<Document> matched_documents;
            std::map<int, double> doc_to_relv;
            for (const std::string& pluses : query_words.plus_word)
            {
                if (!word_to_documents_.count(pluses))
                {
                    continue;
                }
                if (query_words.minus_word.count(pluses)) 
                {
                    continue;
                }
                double idf = GetIDF(pluses);
                for (const auto& id_tf : word_to_documents_.at(pluses))
                {
                    doc_to_relv[id_tf.first] += idf * id_tf.second;
                }
            }
            for (const std::string& minuses : query_words.minus_word)
            {
                for (const auto& word_and_id_in_doc : word_to_documents_)
                {
                    if (word_and_id_in_doc.first == minuses)
                    {
                        for (const auto& id_to_doc : word_and_id_in_doc.second)
                        {
                            doc_to_relv.erase(id_to_doc.first);
                        }
                    }
                }
            }
            for (const auto& relev : doc_to_relv)
            {
                    if (doc_st_.at(relev.first) == status)
                    {
                        matched_documents.push_back({relev.first, relev.second, document_rt_.at(relev.first)});
                    }
            }
            return matched_documents;
        }

        static int ComputeAverageRating(const std::vector<int>& ratings) {              //считает среднее ариф. рейтинга документа
            int rt_size = ratings.size();
            return std::accumulate(ratings.begin(), ratings.end(), 0) / rt_size;
        }
};

void PrintDocument(const Document& document) {
    std::cout << "{ "s
         << "document_id = "s << document.id << ", "s
         << "relevance = "s << document.relevance << ", "s
         << "rating = "s << document.rating
         << " }"s << std::endl;
}

int main() {
    SearchServer search_server;
    search_server.SetStopWords("и в на"s);
    search_server.AddDocument(0, "белый кот и модный ошейник"s,        DocumentStatus::ACTUAL, {8, -3});
    search_server.AddDocument(1, "пушистый кот пушистый хвост"s,       DocumentStatus::ACTUAL, {7, 2, 7});
    search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, {5, -12, 2, 1});
    for (const Document& document : search_server.FindTopDocuments("ухоженный кот"s)) {
        PrintDocument(document);
    }
}