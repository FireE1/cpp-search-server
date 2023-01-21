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
    server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("cat"s);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);
    }
    {
    SearchServer server;
    server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("in"s);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);
    }
    {
    SearchServer server;
    server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("the"s);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);
    }
    {
    SearchServer server;
    server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("city"s);
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

void TestOnPredictFilt () {
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
        ASSERT_EQUAL(doc0.rating, 5);
        ASSERT_EQUAL(doc2.rating, 2);
    }
}

void TestRelevanceOnDocuments () {
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
        ASSERT(doc0.relevance > doc2.relevance);
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
    RUN_TEST(TestOnPredictFilt);
}