#pragma once

#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "paginator.h"
#include "process_queries.h"
#include "read_input_functions.h"
//#include "remove_duplicates.h"
#include "request_queue.h"
#include "search_server.h"

void AddDocument(SearchServer& search_server, int document_id, const std::string& document, DocumentStatus status, const std::vector<int>& ratings);
void FindTopDocuments(const SearchServer& search_server, const std::string& raw_query);
void MatchDocuments(const SearchServer& search_server, const std::string& query);

// BanchMark for Test03,05
/*std::string GenerateWord(std::mt19937& generator, int max_length);
std::vector<std::string> GenerateDictionary(std::mt19937& generator, int word_count, int max_length);
std::string GenerateQuery(std::mt19937& generator, const std::vector<std::string>& dictionary, int max_word_count);
std::vector<std::string> GenerateQueries(std::mt19937& generator, const std::vector<std::string>& dictionary, int query_count, int max_word_count);
template <typename QueriesProcessor>
void Test(std::string_view mark, QueriesProcessor processor, const SearchServer& search_server, const std::vector<std::string>& queries) {
    LOG_DURATION(mark);
    const auto documents_lists = processor(search_server, queries);
}//*/

// BanchMark for Test07
/*std::string GenerateWord(std::mt19937& generator, int max_length);
std::vector<std::string> GenerateDictionary(std::mt19937& generator, int word_count, int max_length);
std::string GenerateQuery(std::mt19937& generator, const std::vector<std::string>& dictionary, int max_word_count);
std::vector<std::string> GenerateQueries(std::mt19937& generator, const std::vector<std::string>& dictionary, int query_count, int max_word_count);    
template <typename ExecutionPolicy>
void Test(std::string_view mark, SearchServer search_server, ExecutionPolicy&& policy) {
    using namespace std;
    LOG_DURATION(mark);
    const int document_count = search_server.GetDocumentCount();
    for (int id = 0; id < document_count; ++id) {
        search_server.RemoveDocument(policy, id);
    }
    cout << search_server.GetDocumentCount() << endl;
}//*/

// BanchMark for Test09
std::string GenerateWord(std::mt19937& generator, int max_length);
std::vector<std::string> GenerateDictionary(std::mt19937& generator, int word_count, int max_length);
std::string GenerateQuery(std::mt19937& generator, const std::vector<std::string>& dictionary, int word_count, double minus_prob);
std::vector<std::string> GenerateQueries(std::mt19937& generator, const std::vector<std::string>& dictionary, int query_count, int max_word_count);
/*template <typename ExecutionPolicy>
void Test(std::string_view mark, SearchServer search_server, const std::string& query, ExecutionPolicy&& policy) {
    using namespace std;
    LOG_DURATION(mark);
    const int document_count = search_server.GetDocumentCount();
    int word_count = 0;
    for (int id = 0; id < document_count; ++id) {
        const auto [words, status] = search_server.MatchDocument(policy, query, id);
        word_count += words.size();
    }
    cout << word_count << endl;
}//*/

// BanchMark for Test11
/*std::string GenerateWord(std::mt19937& generator, int max_length);
std::vector<std::string> GenerateDictionary(std::mt19937& generator, int word_count, int max_length);
std::string GenerateQuery(std::mt19937& generator, const std::vector<std::string>& dictionary, int word_count, double minus_prob = 0);
std::vector<std::string> GenerateQueries(std::mt19937& generator, const std::vector<std::string>& dictionary, int query_count, int max_word_count);//*/
template <typename ExecutionPolicy>
void Test(std::string_view mark, const SearchServer& search_server, const std::vector<std::string>& queries, ExecutionPolicy&& policy) {
    using namespace std;
    LOG_DURATION(mark);
    double total_relevance = 0;
    for (const string_view query : queries) {
        for (const auto& document : search_server.FindTopDocuments(policy, query)) {
            total_relevance += document.relevance;
        }
    }
    cout << total_relevance << endl;
}//*/

void Test01(); // checking old search server
void Test02(); // checking ProcessQueries with execution
void Test03(); // + random7
void Test04(); // checking ProcessQueriesJoined
void Test05(); // + random
void Test06(); // making parallel SearchServer::RemoveDocumnt
void Test07(); // + random
void Test08(); // making parallel SearchServer::MatchDocument
void Test09(); // + random
void Test10(); // making parallel SearchServer::FindTopDocument
void Test11(); // + random

