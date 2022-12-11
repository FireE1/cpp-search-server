#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <map>
#include <cmath>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result = 0;
    cin >> result;
    ReadLine();
    return result;
}

vector<string> SplitIntoWords(const string& text) {         //делим строку на слова
    vector<string> words;
    string word;
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
};

struct Query {
    set<string> plus_word;
    set<string> minus_word;
};

struct MinusNotStop {
    string word;
    bool minus;
    bool stop;
};

class SearchServer {
    public:
        void SetStopWords(const string& text) {             //собираем стоп слова в кучу
            for (const string& word : SplitIntoWords(text)) 
            {
                stop_words_.insert(word);
            }
        }

        void AddDocument(int document_id, const string& document) {             //распределяем слова, в каких документах они и их TF
            const vector<string> words = SplitIntoWordsNoStop(document);                                    //(количество конкретных слов ко всем словам)
            for (const string& word : words)
            {
                word_to_documents_[word][document_id] += 1.0 / words.size();
            }
            ++document_count_;
        }

        vector<Document> FindTopDocuments(const string& raw_query) const {              //ищем нужный документ по запросу
            const Query query_words = ParseQuery(raw_query);
            auto matched_documents = FindAllDocuments(query_words);

            sort(matched_documents.begin(), matched_documents.end(), [](const Document& lhs, const Document& rhs) {
                return lhs.relevance > rhs.relevance;});
            if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) 
            {
                matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
            }
            return matched_documents;
        }
    private:

    map<string, map<int, double>> word_to_documents_;
    set<string> stop_words_;
    int document_count_ = 0;

        bool IsStopWord(const string& word) const {             //является ли слово - стоп словом
            return stop_words_.count(word) > 0;
        }

        MinusNotStop ParseQueryForMinus(string word) const {                //проверка слова на минус и проверка на стоп
            bool minus = false;
            if (word[0] == '-') 
            {
                minus = true;
                word = word.substr(1);
            }
            return {word, minus, IsStopWord(word)};
        }

        Query ParseQuery(const string& text) const {                //разбираем запрос на слова(плюс и минус)
            Query query_words;
            for (const string& word : SplitIntoWords(text)) 
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
    
        vector<string> SplitIntoWordsNoStop(const string& text) const {             //отсекаем, от множества слов, стоп слова
            vector<string> words;
            for (const string& word : SplitIntoWords(text)) 
            {
                if (stop_words_.count(word) == 0) 
                {
                    words.push_back(word);
                }
            }
            return words;
        }

        double GetIDF(const string& iter_word) const {
            return log(document_count_ * 1.0 / word_to_documents_.at(iter_word).size());
        }

        vector<Document> FindAllDocuments(const Query& query_words) const {             //выделяем id и релевантность нужных нам документов
            vector<Document> matched_documents;
            map<int, double> doc_to_relv;
            for (const string& pluses : query_words.plus_word)
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
            for (const string& minuses : query_words.minus_word)
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
                matched_documents.push_back({relev.first, relev.second});
            }
            return matched_documents;
        }
};

SearchServer CreateSearchServer() {             //собираем сервер
    SearchServer search_server;
    search_server.SetStopWords(ReadLine());

    const int document_count = ReadLineWithNumber();
    for (int document_id = 0; document_id < document_count; ++document_id) 
    {
        search_server.AddDocument(document_id, ReadLine());
    }
    return search_server;
}

int main() {                //задаем запрос и выделяем нужные документы по id и релевантности
    SearchServer search_server = CreateSearchServer();
    const string query = ReadLine();
    for (const auto& [document_id, relevance] : search_server.FindTopDocuments(query)) 
    {
        cout << "{ document_id = "s << document_id << ", relevance = "s << relevance << " }"s << endl;
    }
}