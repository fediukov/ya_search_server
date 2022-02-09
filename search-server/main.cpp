#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <numeric>
#include <set>
#include <string>
#include <utility>
#include <vector>

using namespace std;

/* Подставьте вашу реализацию класса SearchServer сюда */
const int MAX_RESULT_DOCUMENT_COUNT = 5;

const double EPSILON = 1e-6;

vector<string> SplitIntoWords(const string& text) {
    vector<string> words;
    string word;
    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        }
        else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }

    return words;
}

struct Document {
    int id;
    double relevance;
    int rating;
};

enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};

class SearchServer {
public:
    void SetStopWords(const string& text) {
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }

    void AddDocument(int document_id, const string& document, DocumentStatus status, const vector<int>& ratings) {
        const vector<string> words = SplitIntoWordsNoStop(document);
        const double inv_word_count = 1.0 / words.size();
        for (const string& word : words) {
            word_to_document_freqs_[word][document_id] += inv_word_count;
        }
        documents_.emplace(document_id,
            DocumentData{
                ComputeAverageRating(ratings),
                status
            });
    }

    // findtopdocs 1/3 - check status
    vector<Document> FindTopDocuments(const string& raw_query, DocumentStatus status) const {
        return FindTopDocuments(raw_query,
                                [&status](int, const DocumentStatus& compared_status, int)
            { return compared_status == status; });
    }

    //  findtopdocs 2/3 - default value status = actual
    vector<Document> FindTopDocuments(const string& raw_query) const
    {
        return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
    }

    //  findtopdocs 3/3 - lambda function
    template <typename KeyMapper>
    vector<Document> FindTopDocuments(const string& raw_query, KeyMapper key_mapper) const {
        const Query query = ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(query, key_mapper);

        sort(matched_documents.begin(), matched_documents.end(),
            [](const Document& lhs, const Document& rhs) {
                if (abs(lhs.relevance - rhs.relevance) < EPSILON) {
                    return lhs.rating > rhs.rating;
                }
                else {
                    return lhs.relevance > rhs.relevance;
                }
            });
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
    }

    int GetDocumentCount() const {
        return documents_.size();
    }

    tuple<vector<string>, DocumentStatus> MatchDocument(const string& raw_query, int document_id) const {
        const Query query = ParseQuery(raw_query);
        vector<string> matched_words;
        for (const string& word : query.plus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            if (word_to_document_freqs_.at(word).count(document_id)) {
                matched_words.push_back(word);
            }
        }
        for (const string& word : query.minus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            if (word_to_document_freqs_.at(word).count(document_id)) {
                matched_words.clear();
                break;
            }
        }
        return { matched_words, documents_.at(document_id).status };
    }

private:
    struct DocumentData {
        int rating;
        DocumentStatus status;
    };

    set<string> stop_words_;
    map<string, map<int, double>> word_to_document_freqs_;
    map<int, DocumentData> documents_;

    bool IsStopWord(const string& word) const {
        return stop_words_.count(word) > 0;
    }

    vector<string> SplitIntoWordsNoStop(const string& text) const {
        vector<string> words;
        for (const string& word : SplitIntoWords(text)) {
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        return words;
    }

    static int ComputeAverageRating(const vector<int>& ratings) {
        if (ratings.empty()) {
            return 0;
        }
        int rating_sum = accumulate(ratings.begin(), ratings.end(), 0);
        return rating_sum / static_cast<int>(ratings.size());
    }

    struct QueryWord {
        string data;
        bool is_minus;
        bool is_stop;
    };

    QueryWord ParseQueryWord(string text) const {
        bool is_minus = false;
        // Word shouldn't be empty
        if (text[0] == '-') {
            is_minus = true;
            text = text.substr(1);
        }
        return {
            text,
            is_minus,
            IsStopWord(text)
        };
    }

    struct Query {
        set<string> plus_words;
        set<string> minus_words;
    };

    Query ParseQuery(const string& text) const {
        Query query;
        for (const string& word : SplitIntoWords(text)) {
            const QueryWord query_word = ParseQueryWord(word);
            if (!query_word.is_stop) {
                if (query_word.is_minus) {
                    query.minus_words.insert(query_word.data);
                }
                else {
                    query.plus_words.insert(query_word.data);
                }
            }
        }
        return query;
    }

    // Existence required
    double ComputeWordInverseDocumentFreq(const string& word) const {
        return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
    }

    template <typename KeyMapper>
    vector<Document> FindAllDocuments(const Query& query, KeyMapper key_mapper) const {
        map<int, double> document_to_relevance;
        for (const string& word : query.plus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
            for (const auto [doc_id, relevance] : word_to_document_freqs_.at(word)) {
                if (key_mapper(doc_id, documents_.at(doc_id).status, documents_.at(doc_id).rating)) {
                    document_to_relevance[doc_id] += relevance * inverse_document_freq;
                }
            }
        }

        for (const string& word : query.minus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            for (const auto [doc_id, _] : word_to_document_freqs_.at(word)) {
                document_to_relevance.erase(doc_id);
            }
        }

        vector<Document> matched_documents;
        for (const auto [doc_id, relevance] : document_to_relevance) {
            matched_documents.push_back({
                doc_id,
                relevance,
                documents_.at(doc_id).rating
                });
        }
        return matched_documents;
    }
};

/* 
   Подставьте сюда вашу реализацию макросов 
   ASSERT, ASSERT_EQUAL, ASSERT_EQUAL_HINT, ASSERT_HINT и RUN_TEST
*/
template <typename T, typename U>
void AssertEqualImpl(const T& t, const U& u, const string& t_str, const string& u_str, const string& file,
                     const string& func, unsigned line, const string& hint) {
    if (t != u) {
        cout << boolalpha;
        cout << file << "("s << line << "): "s << func << ": "s;
        cout << "ASSERT_EQUAL("s << t_str << ", "s << u_str << ") failed: "s;
        cout << t << " != "s << u << "."s;
        if (!hint.empty()) {
            cout << " Hint: "s << hint;
        }
        cout << endl;
        abort();
    }
}

#define ASSERT_EQUAL(a, b) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_EQUAL_HINT(a, b, hint) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, (hint))

void AssertImpl(bool value, const string& expr_str, const string& file, const string& func, unsigned line,
                const string& hint) {
    if (!value) {
        cout << file << "("s << line << "): "s << func << ": "s;
        cout << "ASSERT("s << expr_str << ") failed."s;
        if (!hint.empty()) {
            cout << " Hint: "s << hint;
        }
        cout << endl;
        abort();
    }
}

#define ASSERT(expr) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_HINT(expr, hint) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, (hint))

template <typename F>
void RunTestImpl(const F& Func, const string& func_name) {
    Func();
    cerr << func_name << " OK" << endl;
}

#define RUN_TEST(func) RunTestImpl((func), #func)

// -------- Начало модульных тестов поисковой системы ----------

// Тест проверяет, что поисковая система исключает стоп-слова при добавлении документов
void TestExcludeStopWordsFromAddedDocumentContent() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = {1, 2, 3};
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("in"s);
        ASSERT_EQUAL(found_docs.size(), 1u);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);
    }

    {
        SearchServer server;
        server.SetStopWords("in the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT_HINT(server.FindTopDocuments("in"s).empty(), "Stop words must be excluded from documents"s);
    }
}

/*
Разместите код остальных тестов здесь
*/
void TestAddDocument()
{
    const int doc_id1 = 42;
    const string content1 = "cat in the city"s;
    const vector<int> ratings1 = {1, 2, 3};
    const int doc_id2 = 24;
    const string content2 = "fat out of city"s;
    const vector<int> ratings2 = {1, 2, 3, 6};
    {
        SearchServer server;
        const auto found_docs = server.FindTopDocuments("cat"s);
        ASSERT(server.FindTopDocuments("cat"s).empty());
    }
    
    {
        SearchServer server;
        server.AddDocument(doc_id1, content1, DocumentStatus::ACTUAL, ratings1);
        const auto found_docs = server.FindTopDocuments("cat"s);
        ASSERT_EQUAL(found_docs.size(), 1u);
        const Document& doc0 = found_docs[0];
        ASSERT(doc0.id == doc_id1);
    }
    
    {
        SearchServer server;
        server.AddDocument(doc_id1, content1, DocumentStatus::ACTUAL, ratings1);
        server.AddDocument(doc_id2, content2, DocumentStatus::ACTUAL, ratings2);
        const auto found_docs = server.FindTopDocuments("city"s);
        ASSERT_EQUAL(found_docs.size(), 2u);
        const Document& doc0 = found_docs[0];
        const Document& doc1 = found_docs[1];
        ASSERT(doc1.id == doc_id1 || doc1.id == doc_id2);
        ASSERT(doc0.id == doc_id1 || doc0.id == doc_id2);
    } 
}

void TestMinusWords()
{
    const int doc_id1 = 42;
    const string content1 = "cat in the city"s;
    const vector<int> ratings1 = {1, 2, 3};
    const int doc_id2 = 24;
    const string content2 = "fat of the city"s;
    const vector<int> ratings2 = {1, 2, 3, 6};
    const int doc_id3 = 33;
    const string content3 = "fat of the country"s;
    const vector<int> ratings3 = {1, 6};

    {
        SearchServer server;
        server.AddDocument(doc_id1, content1, DocumentStatus::ACTUAL, ratings1);
        server.AddDocument(doc_id2, content2, DocumentStatus::ACTUAL, ratings2);
        server.AddDocument(doc_id3, content3, DocumentStatus::ACTUAL, ratings3);
        ASSERT(server.FindTopDocuments("-fat"s).empty());
    }
    
    {
        SearchServer server;
        server.AddDocument(doc_id1, content1, DocumentStatus::ACTUAL, ratings1);
        server.AddDocument(doc_id2, content2, DocumentStatus::ACTUAL, ratings2);
        server.AddDocument(doc_id3, content3, DocumentStatus::ACTUAL, ratings3);
        const auto found_docs = server.FindTopDocuments("city -fat"s);
        ASSERT_EQUAL(found_docs.size(), 1u);
        const Document& doc0 = found_docs[0];
        ASSERT(doc0.id == doc_id1);
    }

    {
        SearchServer server;
        server.AddDocument(doc_id1, content1, DocumentStatus::ACTUAL, ratings1);
        server.AddDocument(doc_id2, content2, DocumentStatus::ACTUAL, ratings2);
        server.AddDocument(doc_id3, content3, DocumentStatus::ACTUAL, ratings3);
        const auto found_docs = server.FindTopDocuments("-city fat"s);
        ASSERT_EQUAL(found_docs.size(), 1u);
        const Document& doc0 = found_docs[0];
        ASSERT(doc0.id == doc_id3);
    }

    {
        SearchServer server;
        server.AddDocument(doc_id1, content1, DocumentStatus::ACTUAL, ratings1);
        server.AddDocument(doc_id2, content2, DocumentStatus::ACTUAL, ratings2);
        server.AddDocument(doc_id3, content3, DocumentStatus::ACTUAL, ratings3);
        const auto found_docs = server.FindTopDocuments("-city fat cat"s);
        ASSERT(found_docs.size() == 1);
        const Document& doc0 = found_docs[0];
        ASSERT(doc0.id == doc_id3);
    }

    {
        SearchServer server;
        server.AddDocument(doc_id1, content1, DocumentStatus::ACTUAL, ratings1);
        server.AddDocument(doc_id2, content2, DocumentStatus::ACTUAL, ratings2);
        server.AddDocument(doc_id3, content3, DocumentStatus::ACTUAL, ratings3);
        const auto found_docs = server.FindTopDocuments("city country -cat"s);
        ASSERT(found_docs.size() == 2);
        const Document& doc0 = found_docs[0];
        const Document& doc1 = found_docs[1];
        ASSERT(doc0.id == doc_id2 || doc0.id == doc_id3);
        ASSERT(doc1.id == doc_id2 || doc1.id == doc_id3);
    }
    
    {
        SearchServer server;
        server.AddDocument(doc_id1, content1, DocumentStatus::ACTUAL, ratings1);
        server.AddDocument(doc_id2, content2, DocumentStatus::ACTUAL, ratings2);
        server.AddDocument(doc_id3, content3, DocumentStatus::ACTUAL, ratings3);
        const auto found_docs = server.FindTopDocuments("city -dog"s);
        ASSERT(found_docs.size() == 2);
        const Document& doc0 = found_docs[0];
        const Document& doc1 = found_docs[1];
        ASSERT(doc0.id == doc_id1 || doc0.id == doc_id2);
        ASSERT(doc1.id == doc_id1 || doc1.id == doc_id2);
    }
}

void TestMatchDocument()
{
    const int doc_id1 = 42;
    const string content1 = "cat in the city"s;
    const vector<int> ratings1 = {1, 2, 3};

    {
        SearchServer server;
        server.AddDocument(doc_id1, content1, DocumentStatus::ACTUAL, ratings1);
        const auto found_docs = server.MatchDocument("cat"s, 42);
        const auto words = get<0>(found_docs);
        const auto status = get<1>(found_docs);
        ASSERT(words.size() == 1);
        ASSERT(count(words.begin(), words.end(), "bat"s) == 0);
        ASSERT(count(words.begin(), words.end(), "city"s) == 0);
        ASSERT(count(words.begin(), words.end(), "cat"s) == 1);
        ASSERT(status == DocumentStatus::ACTUAL);
    }

    {
        SearchServer server;
        server.AddDocument(doc_id1, content1, DocumentStatus::ACTUAL, ratings1);
        const auto found_docs = server.MatchDocument("-bat"s, 42);
        const auto words = get<0>(found_docs);
        ASSERT(words.size() == 0);
    }
    
    {
        SearchServer server;
        server.AddDocument(doc_id1, content1, DocumentStatus::ACTUAL, ratings1);
        const auto found_docs = server.MatchDocument("cat -bat"s, 42);
        const auto words = get<0>(found_docs);
        const auto status = get<1>(found_docs);
        ASSERT(words.size() == 1);
        ASSERT(count(words.begin(), words.end(), "bat"s) == 0);
        ASSERT(count(words.begin(), words.end(), "city"s) == 0);
        ASSERT(count(words.begin(), words.end(), "cat"s) == 1);
        ASSERT(status == DocumentStatus::ACTUAL);
    }
    
    {
        SearchServer server;
        server.AddDocument(doc_id1, content1, DocumentStatus::ACTUAL, ratings1);
        const auto found_docs = server.MatchDocument("cat -city"s, 42);
        const auto words = get<0>(found_docs);
        ASSERT(words.size() == 0);
        ASSERT(count(words.begin(), words.end(), "bat"s) == 0);
        ASSERT(count(words.begin(), words.end(), "city"s) == 0);
        ASSERT(count(words.begin(), words.end(), "cat"s) == 0);
    }

    {
        SearchServer server;
        server.AddDocument(doc_id1, content1, DocumentStatus::ACTUAL, ratings1);
        const auto found_docs = server.MatchDocument("cat in -bat"s, 42);
        const auto words = get<0>(found_docs);
        const auto status = get<1>(found_docs);
        ASSERT(words.size() == 2);
        ASSERT(count(words.begin(), words.end(), "bat"s) == 0);
        ASSERT(count(words.begin(), words.end(), "city"s) == 0);
        ASSERT(count(words.begin(), words.end(), "in"s) == 1);
        ASSERT(count(words.begin(), words.end(), "cat"s) == 1);
        ASSERT(status == DocumentStatus::ACTUAL);
    }

    {
        SearchServer server;
        server.AddDocument(doc_id1, content1, DocumentStatus::ACTUAL, ratings1);
        const auto found_docs = server.MatchDocument("cat in -city"s, 42);
        const auto words = get<0>(found_docs);
        const auto status = get<1>(found_docs);
        ASSERT(words.size() == 0);
        ASSERT(count(words.begin(), words.end(), "bat"s) == 0);
        ASSERT(count(words.begin(), words.end(), "city"s) == 0);
        ASSERT(count(words.begin(), words.end(), "in"s) == 0);
        ASSERT(count(words.begin(), words.end(), "cat"s) == 0);
        ASSERT(status == DocumentStatus::ACTUAL);
    }

    {
        SearchServer server;
        server.AddDocument(doc_id1, content1, DocumentStatus::ACTUAL, ratings1);
        const auto found_docs = server.MatchDocument("the city in cat"s, 42);
        const auto words = get<0>(found_docs);
        const auto status = get<1>(found_docs);
        ASSERT(words.size() == 4);
        ASSERT(count(words.begin(), words.end(), "bat"s) == 0);
        ASSERT(count(words.begin(), words.end(), "city"s) == 1);
        ASSERT(count(words.begin(), words.end(), "in"s) == 1);
        ASSERT(count(words.begin(), words.end(), "cat"s) == 1);
        ASSERT(count(words.begin(), words.end(), "the"s) == 1);
        ASSERT(status == DocumentStatus::ACTUAL);
    }

    {
        SearchServer server;
        server.AddDocument(doc_id1, content1, DocumentStatus::ACTUAL, ratings1);
        const auto found_docs = server.MatchDocument("the city in cat -bat"s, 42);
        const auto words = get<0>(found_docs);
        const auto status = get<1>(found_docs);
        ASSERT(words.size() == 4);
        ASSERT(count(words.begin(), words.end(), "bat"s) == 0);
        ASSERT(count(words.begin(), words.end(), "city"s) == 1);
        ASSERT(count(words.begin(), words.end(), "in"s) == 1);
        ASSERT(count(words.begin(), words.end(), "cat"s) == 1);
        ASSERT(count(words.begin(), words.end(), "the"s) == 1);
        ASSERT(status == DocumentStatus::ACTUAL);
    }
    
    {
        SearchServer server;
        server.AddDocument(doc_id1, content1, DocumentStatus::ACTUAL, ratings1);
        const auto found_docs = server.MatchDocument("the city in cat -in"s, 42);
        const auto words = get<0>(found_docs);
        ASSERT(words.size() == 0);
        ASSERT(count(words.begin(), words.end(), "bat"s) == 0);
        ASSERT(count(words.begin(), words.end(), "city"s) == 0);
        ASSERT(count(words.begin(), words.end(), "in"s) == 0);
        ASSERT(count(words.begin(), words.end(), "cat"s) == 0);
        ASSERT(count(words.begin(), words.end(), "the"s) == 0);
    }
}

void TestSortRelevance()
{
    const int doc_id1 = 42;
    const string content1 = "cat in the city"s;
    const vector<int> ratings1 = {1, 2, 3};
    const int doc_id2 = 24;
    const string content2 = "fat of the city"s;
    const vector<int> ratings2 = {1, 2, 3, 6};
    const int doc_id3 = 33;
    const string content3 = "fat of the country"s;
    const vector<int> ratings3 = {1, 6};

    {
        SearchServer server;
        server.AddDocument(doc_id1, content1, DocumentStatus::ACTUAL, ratings1);
        server.AddDocument(doc_id2, content2, DocumentStatus::ACTUAL, ratings2);
        server.AddDocument(doc_id3, content3, DocumentStatus::ACTUAL, ratings3);
        const auto found_docs = server.FindTopDocuments("the"s);
        ASSERT(found_docs.size() == 3);
        const Document& doc0 = found_docs[0];
        const Document& doc1 = found_docs[1];
        const Document& doc2 = found_docs[2];
        ASSERT(doc0.relevance == 0);
        ASSERT(doc1.relevance == 0);
        ASSERT(doc2.relevance == 0);
        ASSERT(doc0.id == doc_id1 || doc0.id == doc_id2 || doc0.id == doc_id3);
        ASSERT(doc1.id == doc_id1 || doc1.id == doc_id2 || doc1.id == doc_id3);
        ASSERT(doc2.id == doc_id1 || doc2.id == doc_id2 || doc2.id == doc_id3);
    }
    
    {
        SearchServer server;
        server.AddDocument(doc_id1, content1, DocumentStatus::ACTUAL, ratings1);
        server.AddDocument(doc_id2, content2, DocumentStatus::ACTUAL, ratings2);
        server.AddDocument(doc_id3, content3, DocumentStatus::ACTUAL, ratings3);
        const auto found_docs = server.FindTopDocuments("the cat"s);
        ASSERT(found_docs.size() == 3);
        const Document& doc0 = found_docs[0];
        const Document& doc1 = found_docs[1];
        const Document& doc2 = found_docs[2];
        ASSERT(abs(doc0.relevance - 0.274653) < EPSILON);
        ASSERT(doc1.relevance == 0);
        ASSERT(doc2.relevance == 0);
        ASSERT(doc0.id == doc_id1);
        ASSERT(doc1.id == doc_id2 || doc1.id == doc_id3);
        ASSERT(doc2.id == doc_id3 || doc2.id == doc_id2);
    }

    {
        SearchServer server;
        server.AddDocument(doc_id1, content1, DocumentStatus::ACTUAL, ratings1);
        server.AddDocument(doc_id2, content2, DocumentStatus::ACTUAL, ratings2);
        server.AddDocument(doc_id3, content3, DocumentStatus::ACTUAL, ratings3);
        const auto found_docs = server.FindTopDocuments("the fat country"s);
        ASSERT(found_docs.size() == 3);
        const Document& doc0 = found_docs[0];
        const Document& doc1 = found_docs[1];
        const Document& doc2 = found_docs[2];
        ASSERT(abs(doc0.relevance - 0.376019) < EPSILON);
        ASSERT(abs(doc1.relevance - 0.101366) < EPSILON);
        ASSERT(doc2.relevance == 0);
        ASSERT(doc0.id == doc_id3);
        ASSERT(doc1.id == doc_id2);
        ASSERT(doc2.id == doc_id1);
    }
    
    {
        SearchServer server;
        server.AddDocument(doc_id1, content1, DocumentStatus::ACTUAL, ratings1);
        server.AddDocument(doc_id2, content2, DocumentStatus::ACTUAL, ratings2);
        server.AddDocument(doc_id3, content3, DocumentStatus::ACTUAL, ratings3);
        const auto found_docs = server.FindTopDocuments("the fat country bat city fat"s);
        ASSERT(found_docs.size() == 3);
        const Document& doc0 = found_docs[0];
        const Document& doc1 = found_docs[1];
        const Document& doc2 = found_docs[2];
        ASSERT(abs(doc0.relevance - 0.376019) < EPSILON);
        ASSERT(abs(doc1.relevance - 0.202733) < EPSILON);
        ASSERT(abs(doc2.relevance - 0.101366) < EPSILON);
        ASSERT(doc0.id == doc_id3);
        ASSERT(doc1.id == doc_id2);
        ASSERT(doc2.id == doc_id1);
    }
}

void TestRatingCalculation()
{
    const int doc_id1 = 42;
    const string content1 = "cat in the city"s;
    const vector<int> ratings1 = {};
    const int doc_id2 = 24;
    const string content2 = "fat of the city"s;
    const vector<int> ratings2 = {0, 0, 0, 0};
    const int doc_id3 = 33;
    const string content3 = "fat of the country"s;
    const vector<int> ratings3 = {0, 1, 0, 0};
    const int doc_id4 = 3;
    const string content4 = "bat of the country"s;
    const vector<int> ratings4 = {1, 1, 0, 1};
    const int doc_id5 = 4;
    const string content5 = "bat of the city"s;
    const vector<int> ratings5 = {1, 1, 0, 1};

    {
        SearchServer server;
        server.AddDocument(doc_id1, content1, DocumentStatus::ACTUAL, ratings1);
        server.AddDocument(doc_id2, content2, DocumentStatus::ACTUAL, ratings2);
        server.AddDocument(doc_id3, content3, DocumentStatus::ACTUAL, ratings3);
        server.AddDocument(doc_id4, content4, DocumentStatus::ACTUAL, ratings4);
        server.AddDocument(doc_id5, content5, DocumentStatus::ACTUAL, ratings5);
        const auto found_docs = server.FindTopDocuments("the"s);
        ASSERT(found_docs.size() == 5);
        const Document& doc0 = found_docs[0];
        const Document& doc1 = found_docs[1];
        const Document& doc2 = found_docs[2];
        const Document& doc3 = found_docs[3];
        const Document& doc4 = found_docs[4];
        ASSERT(doc0.rating == 0);
        ASSERT(doc1.rating == 0);
        ASSERT(doc2.rating == 0);
        ASSERT(doc3.rating == 0);
        ASSERT(doc4.rating == 0);
    }
    
    const int doc_id6 = 42;
    const string content6 = "cat in the city one"s;
    const vector<int> ratings6 = {1, 2, 3};
    const int doc_id7 = 24;
    const string content7 = "fat of the city one"s;
    const vector<int> ratings7 = {1, 2, -3, 0};
    const int doc_id8 = 33;
    const string content8 = "fat of the country one"s;
    const vector<int> ratings8 = {6, 6, 5, -5};
    const int doc_id9 = 3;
    const string content9 = "bat of the country one"s;
    const vector<int> ratings9 = {-6, -6, -5, 5};
    const int doc_id99 = 4;
    const string content99 = "the alabama town"s;
    const vector<int> ratings99 = {5, 1, -7, -3, -1};

    {
        SearchServer server;
        server.AddDocument(doc_id6, content6, DocumentStatus::ACTUAL, ratings6);
        server.AddDocument(doc_id7, content7, DocumentStatus::ACTUAL, ratings7);
        server.AddDocument(doc_id8, content8, DocumentStatus::ACTUAL, ratings8);
        server.AddDocument(doc_id9, content9, DocumentStatus::ACTUAL, ratings9);
        server.AddDocument(doc_id99, content99, DocumentStatus::ACTUAL, ratings99);
        const auto found_docs = server.FindTopDocuments("cat in the city one fat"s);
        ASSERT(found_docs.size() == 5);
        const Document& doc0 = found_docs[0];
        const Document& doc1 = found_docs[1];
        const Document& doc2 = found_docs[2];
        const Document& doc3 = found_docs[3];
        const Document& doc4 = found_docs[4];
        ASSERT(doc0.rating == 2);
        ASSERT(doc1.rating == 0);
        ASSERT(doc2.rating == 3);
        ASSERT(doc3.rating == -3);
        ASSERT(doc4.rating == -1);
    }
}

void TestStatusOfFindDocuments()
{
    const int doc_id6 = 42;
    const string content6 = "cat in the city one"s;
    const vector<int> ratings6 = {1, 2, 3};
    const int doc_id7 = 24;
    const string content7 = "fat of the city one"s;
    const vector<int> ratings7 = {1, 2, -3, 0};
    const int doc_id8 = 33;
    const string content8 = "fat of the country one"s;
    const vector<int> ratings8 = {6, 6, 5, -5};
    const int doc_id9 = 3;
    const string content9 = "bat of the country one"s;
    const vector<int> ratings9 = {-6, -6, -5, 5};
    const int doc_id99 = 4;
    const string content99 = "the alabama town"s;
    const vector<int> ratings99 = {5, 1, -7, -3, -1};

    {
        SearchServer server;
        server.AddDocument(doc_id6, content6, DocumentStatus::ACTUAL, ratings6);
        server.AddDocument(doc_id7, content7, DocumentStatus::IRRELEVANT, ratings7);
        server.AddDocument(doc_id8, content8, DocumentStatus::BANNED, ratings8);
        server.AddDocument(doc_id9, content9, DocumentStatus::REMOVED, ratings9);
        server.AddDocument(doc_id99, content99, DocumentStatus::ACTUAL, ratings99);
        const auto found_docs0 = server.FindTopDocuments("cat in the city one fat"s, DocumentStatus::ACTUAL);
        ASSERT(found_docs0.size() == 2);
        const Document& doc0 = found_docs0[0];
        const Document& doc1 = found_docs0[1];
        ASSERT(doc0.id == doc_id6);
        ASSERT(doc1.id == doc_id99);
        
        const auto found_docs1 = server.FindTopDocuments("cat in the city one fat"s, DocumentStatus::IRRELEVANT);
        ASSERT(found_docs1.size() == 1);
        const Document& doc2 = found_docs1[0];
        ASSERT(doc2.id == doc_id7);

        const auto found_docs2 = server.FindTopDocuments("cat in the city one fat"s, DocumentStatus::BANNED);
        ASSERT(found_docs2.size() == 1);
        const Document& doc3 = found_docs2[0];
        ASSERT(doc3.id == doc_id8);

        const auto found_docs3 = server.FindTopDocuments("cat in the city one fat"s, DocumentStatus::REMOVED);
        ASSERT(found_docs3.size() == 1);
        const Document& doc4 = found_docs3[0];
        ASSERT(doc4.id == doc_id9);
    }
}

void TestCheckingRelevance()
{
    const int doc_id1 = 42;
    const string content1 = "cat in the city"s;
    const vector<int> ratings1 = {};
    const int doc_id2 = 24;
    const string content2 = "fat of the city"s;
    const vector<int> ratings2 = {0, 0, 0, 0};
    const int doc_id3 = 33;
    const string content3 = "fat of the country"s;
    const vector<int> ratings3 = {0, 1, 0, 0};
    const int doc_id4 = 3;
    const string content4 = "bat of the country"s;
    const vector<int> ratings4 = {1, 1, 0, 1};
    const int doc_id5 = 4;
    const string content5 = "bat of the city"s;
    const vector<int> ratings5 = {1, 1, 0, 1};

    {
        SearchServer server;
        server.AddDocument(doc_id1, content1, DocumentStatus::ACTUAL, ratings1);
        server.AddDocument(doc_id2, content2, DocumentStatus::ACTUAL, ratings2);
        server.AddDocument(doc_id3, content3, DocumentStatus::ACTUAL, ratings3);
        server.AddDocument(doc_id4, content4, DocumentStatus::ACTUAL, ratings4);
        server.AddDocument(doc_id5, content5, DocumentStatus::ACTUAL, ratings5);
        const auto found_docs = server.FindTopDocuments("cat in the city one fat"s);
        ASSERT(found_docs.size() == 5);
        const Document& doc0 = found_docs[0];
        const Document& doc1 = found_docs[1];
        const Document& doc2 = found_docs[2];
        const Document& doc3 = found_docs[3];
        const Document& doc4 = found_docs[4];
        ASSERT(abs(doc0.relevance - 0.932425) < EPSILON);
        ASSERT(doc0.id == doc_id1);
        ASSERT(abs(doc1.relevance - 0.356779) < EPSILON);
        ASSERT(doc1.id == doc_id2);
        ASSERT(abs(doc2.relevance - 0.229073) < EPSILON);
        ASSERT(doc2.id == doc_id3);
        ASSERT(abs(doc3.relevance - 0.127706) < EPSILON);
        ASSERT(doc3.id == doc_id5);
        ASSERT(doc4.relevance == 0);
        ASSERT(doc4.id == doc_id4);
    }
}

// Функция TestSearchServer является точкой входа для запуска тестов
void TestSearchServer() {
    RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
    RUN_TEST(TestAddDocument);
    RUN_TEST(TestMinusWords);
    RUN_TEST(TestMatchDocument);
    RUN_TEST(TestSortRelevance);
    RUN_TEST(TestRatingCalculation);
    RUN_TEST(TestStatusOfFindDocuments);
    RUN_TEST(TestCheckingRelevance);
}

// --------- Окончание модульных тестов поисковой системы -----------

int main() {
    TestSearchServer();
    // Если вы видите эту строку, значит все тесты прошли успешно
    cout << "Search server testing finished"s << endl;
}