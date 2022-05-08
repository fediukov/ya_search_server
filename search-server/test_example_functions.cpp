#include "test_example_functions.h"

using namespace std::literals;

void AddDocument(SearchServer& search_server, int document_id, const std::string& document, DocumentStatus status,
    const std::vector<int>& ratings) {
    using namespace std;
    try {
        search_server.AddDocument(document_id, document, status, ratings);
    }
    catch (const invalid_argument& e) {
        cout << "Ошибка добавления документа "s << document_id << ": "s << e.what() << endl;
    }
}

void FindTopDocuments(const SearchServer& search_server, const std::string& raw_query) {
    using namespace std;
    cout << "Результаты поиска по запросу: "s << raw_query << endl;
    try {
        for (const Document& document : search_server.FindTopDocuments(raw_query)) {
            PrintDocument(document);
        }
    }
    catch (const invalid_argument& e) {
        cout << "Ошибка поиска: "s << e.what() << endl;
    }
}

void MatchDocuments(const SearchServer& search_server, const std::string& query) {
    using namespace std;
    try {
        cout << "Матчинг документов по запросу: "s << query << endl;
        for (auto document_id : search_server)
        {
            const auto [words, status] = search_server.MatchDocument(query, document_id);
            PrintMatchDocumentResult(document_id, words, status);
        }
    }
    catch (const invalid_argument& e) {
        cout << "Ошибка матчинга документов на запрос "s << query << ": "s << e.what() << endl;
    }
}

/* -------------------------- Test01 -------------------------- */
void Test01()
{
    using namespace std;
    {
        SearchServer search_server("and in at"s);
        RequestQueue request_queue(search_server);

        search_server.AddDocument(1, "curly cat curly tail"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
        search_server.AddDocument(2, "curly dog and fancy collar"s, DocumentStatus::ACTUAL, { 1, 2, 3 });
        search_server.AddDocument(3, "big cat fancy collar "s, DocumentStatus::ACTUAL, { 1, 2, 8 });
        search_server.AddDocument(4, "big dog sparrow Eugene"s, DocumentStatus::ACTUAL, { 1, 3, 2 });
        search_server.AddDocument(5, "big dog sparrow Vasiliy"s, DocumentStatus::ACTUAL, { 1, 1, 1 });

        // 1439 запросов с нулевым результатом
        for (int i = 0; i < 1439; ++i) {
            request_queue.AddFindRequest("empty request"s);
        }
        // все еще 1439 запросов с нулевым результатом
        request_queue.AddFindRequest("curly dog"s);
        // новые сутки, первый запрос удален, 1438 запросов с нулевым результатом
        request_queue.AddFindRequest("big collar"s);
        // первый запрос удален, 1437 запросов с нулевым результатом
        request_queue.AddFindRequest("sparrow"s);
        cout << "Total empty requests: "s << request_queue.GetNoResultRequests() << endl;
        // проверка матчинга документов
        MatchDocuments(search_server, "yellow big cat");
        // проверка работы функции GetWordFrequencies в search_server
        int id_of_checking_freq = 3;
        map<string_view, double> freqs = search_server.GetWordFrequencies(id_of_checking_freq);
        cout << "Checking frequencies of document " << id_of_checking_freq << ":" << endl;
        for (const auto& freq : freqs)
        {
            cout << "{" << freq.first << " - " << freq.second << "}" << endl;
        }
        // проверка удаления документа
        search_server.RemoveDocument(id_of_checking_freq);
        MatchDocuments(search_server, "yellow big cat");
    }

    {
        // проверка дубликатов
        SearchServer search_server("and with"s);

        AddDocument(search_server, 1, "funny pet and nasty rat"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
        AddDocument(search_server, 2, "funny pet with curly hair"s, DocumentStatus::ACTUAL, { 1, 2 });

        // дубликат документа 2, будет удалён
        AddDocument(search_server, 3, "funny pet with curly hair"s, DocumentStatus::ACTUAL, { 1, 2 });

        // отличие только в стоп-словах, считаем дубликатом
        AddDocument(search_server, 4, "funny pet and curly hair"s, DocumentStatus::ACTUAL, { 1, 2 });

        // множество слов такое же, считаем дубликатом документа 1
        AddDocument(search_server, 5, "funny funny pet and nasty nasty rat"s, DocumentStatus::ACTUAL, { 1, 2 });

        // добавились новые слова, дубликатом не является
        AddDocument(search_server, 6, "funny pet and not very nasty rat"s, DocumentStatus::ACTUAL, { 1, 2 });

        // множество слов такое же, как в id 6, несмотря на другой порядок, считаем дубликатом
        AddDocument(search_server, 7, "very nasty rat and not very funny pet"s, DocumentStatus::ACTUAL, { 1, 2 });

        // есть не все слова, не является дубликатом
        AddDocument(search_server, 8, "pet with rat and rat and rat"s, DocumentStatus::ACTUAL, { 1, 2 });

/*        // слова из разных документов, не является дубликатом
        AddDocument(search_server, 9, "nasty rat with curly hair"s, DocumentStatus::ACTUAL, { 1, 2 });

        cout << "Before duplicates removed: "s << search_server.GetDocumentCount() << endl;
        RemoveDuplicates(search_server);
        cout << "After duplicates removed: "s << search_server.GetDocumentCount() << endl;//*/
    }
    cout << "Test 01 finished" << endl;
}

/* -------------------------- Test02 -------------------------- */
void Test02()
{
    using namespace std;
    
    SearchServer search_server("and with"s);

    int id = 0;
    for (
        const string& text : {
            "funny pet and nasty rat"s,
            "funny pet with curly hair"s,
            "funny pet and not very nasty rat"s,
            "pet with rat and rat and rat"s,
            "nasty rat with curly hair"s,
        }
    ) {
        search_server.AddDocument(++id, text, DocumentStatus::ACTUAL, {1, 2});
    }

    const vector<string> queries = {
        "nasty rat -not"s,
        "not very funny nasty pet"s,
        "curly hair"s
    };
    id = 0;
    for (const auto& documents : ProcessQueries(search_server, queries))
    {
        cout << documents.size() << " documents for query ["s << queries[id++] << "]"s << endl;
    }//*/
    cout << "Test 02 finished" << endl;
}

/* ------------------- BanchMark for Test03 -------------------- */
/*
std::string GenerateWord(std::mt19937& generator, int max_length) {
    using namespace std;
    const int length = uniform_int_distribution(1, max_length)(generator);
    string word;
    word.reserve(length);
    for (int i = 0; i < length; ++i) {
        word.push_back(uniform_int_distribution('a', 'z')(generator));
    }
    return word;
}

std::vector<std::string> GenerateDictionary(std::mt19937& generator, int word_count, int max_length) {
    using namespace std;
    vector<string> words;
    words.reserve(word_count);
    for (int i = 0; i < word_count; ++i) {
        words.push_back(GenerateWord(generator, max_length));
    }
    sort(words.begin(), words.end());
    words.erase(unique(words.begin(), words.end()), words.end());
    return words;
}

std::string GenerateQuery(std::mt19937& generator, const std::vector<std::string>& dictionary, int max_word_count) {
    using namespace std;
    const int word_count = uniform_int_distribution(1, max_word_count)(generator);
    string query;
    for (int i = 0; i < word_count; ++i) {
        if (!query.empty()) {
            query.push_back(' ');
        }
        query += dictionary[uniform_int_distribution<int>(0, dictionary.size() - 1)(generator)];
    }
    return query;
}

std::vector<std::string> GenerateQueries(std::mt19937& generator, const std::vector<std::string>& dictionary, int query_count, int max_word_count) {
    using namespace std;
    vector<string> queries;
    queries.reserve(query_count);
    for (int i = 0; i < query_count; ++i) {
        queries.push_back(GenerateQuery(generator, dictionary, max_word_count));
    }
    return queries;
}//*/

/*#define TEST(processor) Test(#processor, processor, search_server, queries) //*/

/* ------------------------- Test03 ------------------------- */
void Test03()
{
    // using BanchMark for Test03
    /*using namespace std;
    
    mt19937 generator;
    const auto dictionary = GenerateDictionary(generator, 100, 25);
    const auto documents = GenerateQueries(generator, dictionary, 200, 10);

    SearchServer search_server(dictionary[0]);
    for (size_t i = 0; i < documents.size(); ++i) {
        search_server.AddDocument(i, documents[i], DocumentStatus::ACTUAL, {1, 2, 3});
    }

    const auto queries = GenerateQueries(generator, dictionary, 2'000, 7);
    cout << "Test 03 started" << endl;
    TEST(ProcessQueries);
    cout << "Test 03 finished" << endl;//*/
}

/* ------------------------- Test04 ------------------------- */
void Test04()
{
    using namespace std;
    
    SearchServer search_server("and with"s);

    int id = 0;
    for (
        const string& text : {
            "funny pet and nasty rat"s,
            "funny pet with curly hair"s,
            "funny pet and not very nasty rat"s,
            "pet with rat and rat and rat"s,
            "nasty rat with curly hair"s,
            "fat pet and nasty rat"s,
            "fat pet with curly hair"s,
            "fat pet and not very nasty rat"s,
            "pet with fat and fat rat"s,
            "nasty fat rat with curly hair"s,
            "bat pet and nasty rat"s,
/*            "bat pet with curly hair"s,
            "bat pet and not very nasty rat"s,
            "pet with bat and fat cat"s,
            "nasty bat with curly eyes"s,
            "cat and black bat"s,
            "cat with black hair"s,
            "cat and black eyes"s,
            "pet with cat and white eyes"s,
            "nasty cat with white eyes"s,
            "dog and white bat"s,
            "dog with blue hair"s,
            "dog and red eyes"s,
            "pet with dog and blue eyes"s,
            "nasty dog with red hair"s,//*/
        }
    ) {
        search_server.AddDocument(++id, text, DocumentStatus::ACTUAL, {1, 2});
    }

    const vector<string> queries = {
        "nasty rat -not"s,
        "not very funny nasty pet"s,
        "curly hair"s,
        "bat black",
        "white rat",
/*        "blue cat",
        "red eyes dog",
        "dog hair black", //*/
    };
    for (const auto& vec : ProcessQueries(search_server, queries)) {
        for (const auto& doc : vec) { cout << "{" << doc.id << "} "; }
    } cout << endl;//*/

/*    for (const Document& document : ProcessQueriesJoinedList(search_server, queries)) {
        //cout << "Document "s << document.id << " matched with relevance "s << document.relevance << endl;
    }//*/
    for (const Document& document : ProcessQueriesJoined(search_server, queries)) {
        cout << "{"s << document.id << "} "s;
    }//*/
    cout << endl;
 /*   for (const Document& document : ProcessQueriesJoinedDeque(search_server, queries)) {
        //cout << "Document "s << document.id << " matched with relevance "s << document.relevance << endl;
    }//*/
 /*   for (const Document& document : ProcessQueriesJoined(search_server, queries)) {
        //cout << "Document "s << document.id << " matched with relevance "s << document.relevance << endl;
    }//*/
    cout << "Test 04 finished" << endl;
}

/* ------------------------- Test05 ------------------------- */
void Test05()
{
    // using BanchMark for Test03
    /*using namespace std;
    
    mt19937 generator;
    const auto dictionary = GenerateDictionary(generator, 100, 25);
    const auto documents = GenerateQueries(generator, dictionary, 200, 10);

    SearchServer search_server(dictionary[0]);
    for (size_t i = 0; i < documents.size(); ++i) {
        search_server.AddDocument(i, documents[i], DocumentStatus::ACTUAL, {1, 2, 3});
    }

    const auto queries = GenerateQueries(generator, dictionary, 2'000, 7);
    
    cout << "Test 05 started" << endl;
    TEST(ProcessQueriesJoined);
    cout << "Test 05 finished" << endl;//*/
}

/* ------------------------- Test06 ------------------------- */
void Test06()
{
    using namespace std;
    
    SearchServer search_server("and with"s);

    int id = 0;
    for (
        const string& text : {
            "funny pet and nasty rat"s,
            "funny pet with curly hair"s,
            "funny pet and not very nasty rat"s,
            "pet with rat and rat and rat"s,
            "nasty rat with curly hair"s,
        }
    ) {
        search_server.AddDocument(++id, text, DocumentStatus::ACTUAL, {1, 2});
    }

    const string query = "curly and funny"s;

    auto report = [&search_server, &query] {
        cout << search_server.GetDocumentCount() << " documents total, "s << search_server.FindTopDocuments(query).size() << " documents for query ["s << query << "]"s << endl;
    };

    report();
    cout << "Test 06 is going" << endl;
    // однопоточная версия
    search_server.RemoveDocument(5);
    report();
    cout << "Test 06 is going" << endl;
    // однопоточная версия
    search_server.RemoveDocument(execution::seq, 1);
    report();
    cout << "Test 06 is going" << endl;
    // многопоточная версия
    search_server.RemoveDocument(execution::par, 2);
//    report();
    cout << "Test 06 finished" << endl;
}

/* ------------------- BanchMark for Test07 -------------------- */
/*std::string GenerateWord(std::mt19937& generator, int max_length) {
    using namespace std;
    const int length = uniform_int_distribution(1, max_length)(generator);
    string word;
    word.reserve(length);
    for (int i = 0; i < length; ++i) {
        word.push_back(uniform_int_distribution('a', 'z')(generator));
    }
    return word;
}

std::vector<std::string> GenerateDictionary(std::mt19937& generator, int word_count, int max_length) {
    using namespace std;
    vector<string> words;
    words.reserve(word_count);
    for (int i = 0; i < word_count; ++i) {
        words.push_back(GenerateWord(generator, max_length));
    }
    sort(words.begin(), words.end());
    words.erase(unique(words.begin(), words.end()), words.end());
    return words;
}

std::string GenerateQuery(std::mt19937& generator, const std::vector<std::string>& dictionary, int max_word_count) {
    using namespace std;
    const int word_count = uniform_int_distribution(1, max_word_count)(generator);
    string query;
    for (int i = 0; i < word_count; ++i) {
        if (!query.empty()) {
            query.push_back(' ');
        }
        query += dictionary[uniform_int_distribution<int>(0, dictionary.size() - 1)(generator)];
    }
    return query;
}

std::vector<std::string> GenerateQueries(std::mt19937& generator, const std::vector<std::string>& dictionary, int query_count, int max_word_count) {
    using namespace std;
    vector<string> queries;
    queries.reserve(query_count);
    for (int i = 0; i < query_count; ++i) {
        queries.push_back(GenerateQuery(generator, dictionary, max_word_count));
    }
    return queries;
}

#define TEST(mode) Test(#mode, search_server, execution::mode) //*/

/* ------------------------- Test07 ------------------------- */
void Test07()
{
    /*using namespace std;
    
    mt19937 generator;

    const auto dictionary = GenerateDictionary(generator, 10'000, 25);
    const auto documents = GenerateQueries(generator, dictionary, 10'000, 100);

    {
        SearchServer search_server(dictionary[0]);
        for (size_t i = 0; i < documents.size(); ++i) {
            search_server.AddDocument(i, documents[i], DocumentStatus::ACTUAL, {1, 2, 3});
        }

        TEST(seq);
    }
    {
        SearchServer search_server(dictionary[0]);
        for (size_t i = 0; i < documents.size(); ++i) {
            search_server.AddDocument(i, documents[i], DocumentStatus::ACTUAL, {1, 2, 3});
        }

        TEST(par);
    }//*/
}

/* ------------------------- Test08 ------------------------- */
void Test08()
{
    using namespace std;
    
    SearchServer search_server("and with"s);

    int id = 0;
    for (
        const string& text : {
            "funny pet and nasty rat"s,
            "funny pet with curly hair"s,
            "funny pet and not very nasty rat"s,
            "pet with rat and rat and rat"s,
            "nasty rat with curly hair"s,
        }
    ) {
        search_server.AddDocument(++id, text, DocumentStatus::ACTUAL, {1, 2});
        cout << id << ": " << text << endl;
    }

    const string query = "curly and funny -not"s;
    cout << "query: " << query << endl;

    {
        const auto [words, status] = search_server.MatchDocument(query, 1);
        cout << words.size() << " words for document 1 :"s; for (auto w : words) { cout << w << " "; } cout << endl;
        // 1 words for document 1
    }

    {
        const auto [words, status] = search_server.MatchDocument(execution::seq, query, 2);
        cout << words.size() << " words for document 2 :"s; for (auto w : words) { cout << w << " "; } cout << endl;
        // 2 words for document 2
    }

    {
        const auto [words, status] = search_server.MatchDocument(execution::par, query, 3);
        cout << words.size() << " words for document 3 :"s; for (auto w : words) { cout << w << " "; } cout << endl;
        // 0 words for document 3
    }    
    cout << "Test 08 finished" << endl;
}

/* ------------------- BanchMark for Test09 -------------------- */
std::string GenerateWord(std::mt19937& generator, int max_length) {
    using namespace std;
    const int length = uniform_int_distribution<>(1, max_length)(generator);
    string word;
    word.reserve(length);
    for (int i = 0; i < length; ++i) {
        word.push_back(uniform_int_distribution<>('a', 'z')(generator));
    }
    return word;
}

std::vector<std::string> GenerateDictionary(std::mt19937& generator, int word_count, int max_length) {
    using namespace std;
    vector<string> words;
    words.reserve(word_count);
    for (int i = 0; i < word_count; ++i) {
        words.push_back(GenerateWord(generator, max_length));
    }
    sort(words.begin(), words.end());
    words.erase(unique(words.begin(), words.end()), words.end());
    return words;
}

std::string GenerateQuery(std::mt19937& generator, const std::vector<std::string>& dictionary, int word_count, double minus_prob = 0) {
    using namespace std;
    string query;
    for (int i = 0; i < word_count; ++i) {
        if (!query.empty()) {
            query.push_back(' ');
        }
        if (uniform_real_distribution<>(0, 1)(generator) < minus_prob) {
            query.push_back('-');
        }
        query += dictionary[uniform_int_distribution<int>(0, dictionary.size() - 1)(generator)];
    }
    return query;
}

std::vector<std::string> GenerateQueries(std::mt19937& generator, const std::vector<std::string>& dictionary, int query_count, int max_word_count) {
    using namespace std;
    vector<string> queries;
    queries.reserve(query_count);
    for (int i = 0; i < query_count; ++i) {
        queries.push_back(GenerateQuery(generator, dictionary, max_word_count));
    }
    return queries;
}//*/

//#define TEST(policy) Test(#policy, search_server, query, execution::policy)//*/

/* ------------------------- Test09 ------------------------- */
void Test09()
{
    /*using namespace std;
    mt19937 generator;

    const auto dictionary = GenerateDictionary(generator, 200, 10);
    const auto documents = GenerateQueries(generator, dictionary, 2'000, 70);

    const string query = GenerateQuery(generator, dictionary, 200, 0.1);

    SearchServer search_server(dictionary[0]);
    for (size_t i = 0; i < documents.size(); ++i) {
        search_server.AddDocument(i, documents[i], DocumentStatus::ACTUAL, {1, 2, 3});
    }

    TEST(seq);
    TEST(par);    
    cout << "Test 09 finished" << endl;//*/
}

/* ------------------------- Test10 ------------------------- */
void Test10()
{
    using namespace std;
    
    SearchServer search_server("and with"s);

    int id = 0;
    for (
        const string& text : {
            "white cat and yellow hat"s,
            "curly cat curly tail"s,
            "nasty dog with big eyes"s,
            "nasty pigeon john"s,
        }
        ) {
        search_server.AddDocument(++id, text, DocumentStatus::ACTUAL, { 1, 2 });
    }

    cout << "ACTUAL by default:"s << endl;
    // последовательная версия
    for (const Document& document : search_server.FindTopDocuments("curly nasty cat"s)) {
        PrintDocument(document);
    }
    cout << "BANNED:"s << endl;
    // последовательная версия
    for (const Document& document : search_server.FindTopDocuments(/*execution::seq, */"curly nasty cat"s, DocumentStatus::BANNED)) {
        PrintDocument(document);
    }

    cout << "Even ids:"s << endl;
    // параллельная версия
    for (const Document& document : search_server.FindTopDocuments(/*execution::par, */"curly nasty cat"s, [](int document_id, DocumentStatus status, int rating) { return document_id % 2 == 0; })) {
        PrintDocument(document);
    }
}

/* ------------------- BanchMark for Test11 -------------------- */
/*std::string GenerateWord(std::mt19937& generator, int max_length) {
    using namespace std;
    const int length = uniform_int_distribution<>(1, max_length)(generator);
    string word;
    word.reserve(length);
    for (int i = 0; i < length; ++i) {
        word.push_back(uniform_int_distribution<>('a', 'z')(generator));
    }
    return word;
}

std::vector<std::string> GenerateDictionary(std::mt19937& generator, int word_count, int max_length) {
    using namespace std;
    vector<string> words;
    words.reserve(word_count);
    for (int i = 0; i < word_count; ++i) {
        words.push_back(GenerateWord(generator, max_length));
    }
    words.erase(unique(words.begin(), words.end()), words.end());
    return words;
}

std::string GenerateQuery(std::mt19937& generator, const std::vector<std::string>& dictionary, int word_count, double minus_prob = 0) {
    using namespace std;
    string query;
    for (int i = 0; i < word_count; ++i) {
        if (!query.empty()) {
            query.push_back(' ');
        }
        if (uniform_real_distribution<>(0, 1)(generator) < minus_prob) {
            query.push_back('-');
        }
        query += dictionary[uniform_int_distribution<int>(0, dictionary.size() - 1)(generator)];
    }
    return query;
}

std::vector<std::string> GenerateQueries(std::mt19937& generator, const std::vector<std::string>& dictionary, int query_count, int max_word_count) {
    using namespace std;
    vector<string> queries;
    queries.reserve(query_count);
    for (int i = 0; i < query_count; ++i) {
        queries.push_back(GenerateQuery(generator, dictionary, max_word_count));
    }
    return queries;
}//*/

#define TEST(policy) Test(#policy, search_server, queries, execution::policy)//*/

/* ------------------------- Test11 ------------------------- */
void Test11()
{
    using namespace std;

    mt19937 generator;

    const auto dictionary = GenerateDictionary(generator, 1000, 10);
    const auto documents = GenerateQueries(generator, dictionary, 10'000, 70);

    SearchServer search_server(dictionary[0]);
    for (size_t i = 0; i < documents.size(); ++i) {
        search_server.AddDocument(i, documents[i], DocumentStatus::ACTUAL, { 1, 2, 3 });
    }

    const auto queries = GenerateQueries(generator, dictionary, 100, 70);

    TEST(seq);
    TEST(par);//*/
}
