void TestExcludeDocumentsWithMinusWords () {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = {1, 2, 3};
    {    
    SearchServer server;
    server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("in"s);
        ASSERT_EQUAL(found_docs.size(), 1);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);
    }
    
    {    
    SearchServer server;
    server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("cat -in"s);
        ASSERT_EQUAL(found_docs.size(), 0);
    }
    
    {
        SearchServer server;
    server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT(server.FindTopDocuments("-in"s).empty());
    }
   }

void TestOnDocumentAdd () {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = {1, 2, 3};
    {
    SearchServer server;
        const auto found_docs = server.FindTopDocuments("cat"s);
        ASSERT(found_docs.empty());
    server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("cat"s);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);
    }
}

void TestOnFuncMatch () {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = {1, 2, 3};
    {
    SearchServer server;
    server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto match_no_m = server.MatchDocument("cat city"s, 42);
        const auto match_m = server.MatchDocument("cat -city"s, 42);
        ASSERT_EQUAL(match_no_m, ({"cat"s, "city"s}, DocumentStatus::ACTUAL));
        ASSERT(match_m.empty());
    }
}

void TestOnStatusFilt () {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = {1, 2, 3};
    
    const int doc_id_2 = 48;
    const string content_2 = "dog on the city"s;
    const vector<int> ratings_2 = {4, 5, 6};
    {
    server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
    server.AddDocument(doc_id_2, content_2, DocumentStatus::REMOVED, ratings_2);
        const auto found_docs = server.FindTopDocuments("the city"s, DocumentStatus::ACTUAL);
        ASSERT_EQUAL(found_docs.size(), 1);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);
    }
}

void TestOnPredicatFilt () {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = {1, 2, 3};
    
    const int doc_id_2 = 48;
    const string content_2 = "dog on the city"s;
    const vector<int> ratings_2 = {4, 5, 6};
    {
    server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
    server.AddDocument(doc_id_2, content_2, DocumentStatus::REMOVED, ratings_2);
        const auto found_docs = server.FindTopDocuments("the city"s, 42);
        ASSERT_EQUAL(found_docs.size(), 1);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);
    }
}

void TestMatchingDocumentsWithMinusAndWithOut () {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = {1, 2, 3};
    
    const int doc_id_2 = 48;
    const string content_2 = "dog on the city"s;
    const vector<int> ratings_2 = {4, 5, 6};
    {
    SearchServer server;
    server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
    server.AddDocument(doc_id_2, content_2, DocumentStatus::ACTUAL, ratings_2);
        const auto found_docs = server.FindTopDocuments("-dog city"s);
        ASSERT_EQUAL(found_docs.size(), 1);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);
    }
    
    {
    SearchServer server;
    server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
    server.AddDocument(doc_id_2, content_2, DocumentStatus::ACTUAL, ratings_2);
        const auto found_docs = server.FindTopDocuments("-cat city"s);
        ASSERT_EQUAL(found_docs.size(), 1);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id_2);
    }
}
void TestRatingOnDocuments () {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = {1, 2, 3};
    
    const int doc_id_2 = 48;
    const string content_2 = "dog on the city"s;
    const vector<int> ratings_2 = {4, 5, 6};
    {
    SearchServer server;   
    server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
    server.AddDocument(doc_id_2, content_2, DocumentStatus::ACTUAL, ratings_2);
        const auto found_docs = server.FindTopDocuments("on city"s);
        ASSERT_EQUAL(found_docs.size(), 2);
        const Document& doc0 = found_docs[0];
        const Document& doc2 = found_docs[1];
        ASSERT_EQUAL(doc0.rating, (accumulate(ratings_2.begin(), ratings_2.end(), 0) / static_cast<int>(ratings_2.size())));
        ASSERT_EQUAL(doc2.rating, (accumulate(ratings.begin(), ratings.end(), 0) / static_cast<int>(ratings.size())));
    }
}

double Relevance (vector<string> content, vector<string> q, size_t docs_num) {
    double relev = 0.0;
    int TF_words_match = 0;
    set<string> IDF_words;
    for (const string& i : content)
    {
        for (const string& u : q)
        {
            if (i == u)
            {
                ++IDF_words_match;
                TF_words.insert(u);
            }
        }
    }
    relev = (log((docs_num * 1.0) / (IDF_words * 1.0))) * ((TF_words_match * 1.0) / (content.size() * 1.0));
    return relev;
}

void TestRelevanceOnDocuments () {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = {1, 2, 3};
    
    const int doc_id_2 = 48;
    const string content_2 = "dog on the city"s;
    const vector<int> ratings_2 = {4, 5, 6};

    const string q = "on city"s;

    const double inv_word_count = 1.0 / words.size();
    log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
    SplitIntoWords()
    {
    SearchServer server;   
    server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
    server.AddDocument(doc_id_2, content_2, DocumentStatus::ACTUAL, ratings_2);
        const auto found_docs = server.FindTopDocuments(q);
        const Document& doc0 = found_docs[0];
        const Document& doc2 = found_docs[1];
        const double doc0_rel = Relevance(SplitIntoWords(content_2), SplitIntoWords(q), found_docs.size());
        const double doc2_rel = Relevance(SplitIntoWords(content), SplitIntoWords(q), found_docs.size());
        ASSERT_EQUAL(doc0.relevance, doc0_rel);
        ASSERT_EQUAL(doc2.relevance, doc2_rel);
        ASSERT(doc0.relevance > doc2.relevance);
    }
}

void TestSortingOnDocuments () {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = {1, 2, 3};
    
    const int doc_id_2 = 48;
    const string content_2 = "dog on the city"s;
    const vector<int> ratings_2 = {4, 5, 6};
    {
    SearchServer server;   
    server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
    server.AddDocument(doc_id_2, content_2, DocumentStatus::ACTUAL, ratings_2);
        const auto found_docs = server.FindTopDocuments("on city"s);
        const Document& doc0 = found_docs[0];
        const Document& doc2 = found_docs[1];
        ASSERT_EQUAL(doc0.id, doc_id_2);
        ASSERT_EQUAL(doc2.id, doc_id);
    }
}

void TestSearchServer() {
    RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
    RUN_TEST(TestExcludeDocumentsWithMinusWords);
    RUN_TEST(TestMatchingDocumentsWithMinusAndWithOut);
    RUN_TEST(TestRatingOnDocuments);
    RUN_TEST(TestRatingOnDocuments);
    RUN_TEST(TestOnDocumentAdd);
    RUN_TEST(TestOnFuncMatch);
    RUN_TEST(TestOnStatusFilt);
    RUN_TEST(TestOnPredicatFilt);
    RUN_TEST(TestSortingOnDocuments);
}