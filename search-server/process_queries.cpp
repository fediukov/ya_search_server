#include "process_queries.h"

std::vector<std::vector<Document>> ProcessQueries(
    const SearchServer& search_server,
    const std::vector<std::string>& queries)
{
    //LOG_DURATION("ProcessQueries");
    std::vector<std::vector<Document>> result(queries.size());
    // trivial version
    /*for (const std::string& query : queries) {
        result.push_back(search_server.FindTopDocuments(query));
    }//*/
    // transform version
    std::transform(std::execution::par,
              queries.begin(), queries.end(),
              result.begin(),
              [&search_server](const std::string& query) {
                  return search_server.FindTopDocuments(query);
              });//*/
    return result;
}

std::vector<Document> ProcessQueriesJoined(
    const SearchServer& search_server,
    const std::vector<std::string>& queries)
{
    std::vector<Document> result;
    auto process_queries = ProcessQueries(search_server, queries);
    for (const auto& i : process_queries) { for (const auto& j : i) { result.push_back(j); } } // trivial way
    return result;
}

/*std::list<Document> ProcessQueriesJoinedList(
    const SearchServer& search_server,
    const std::vector<std::string>& queries)
{
    std::list<Document> result;
    auto process_queries = ProcessQueries(search_server, queries);
//    for (auto i : process_queries) { for (auto j : i) { std::cout << "{" << j.id << "} "; } } std::cout << std::endl;
    std::cout << "ProcessQueriesJoinedList started" << std::endl;
    for (const auto& i : process_queries) { for (const auto& j :i) { result.push_back(j); } } // trivial way
    for_each(std::execution::par,
        process_queries.begin(), process_queries.end(),
        [&result](const std::vector<Document>& v) {
            for (const auto& q : v) { result.push_back(q); }
        });//
    for (const auto& d : result) { std::cout << "{" << d.id << "} "; } std::cout << std::endl;
    std::cout << "ProcessQueriesJoinedList finished" << std::endl;
    return result;
}

std::deque<Document> ProcessQueriesJoinedDeque(
    const SearchServer& search_server,
    const std::vector<std::string>& queries)
{
    std::deque<Document> result;
    auto process_queries = ProcessQueries(search_server, queries);
//    for (auto i : process_queries) { for (auto j : i) { std::cout << "{" << j.id << "} "; } } std::cout << std::endl;
    std::cout << "ProcessQueriesJoinedDeque started" << std::endl;
    //for (const auto& i : process_queries) { for (const auto& j :i) { result.push_back(j); } } // trivial way
    for_each(std::execution::par,
        process_queries.begin(), process_queries.end(),
        [&result](const std::vector<Document>& v) {
            for (const auto& q : v) { result.push_back(q); }
        });//
    for (const auto& d : result) { std::cout << "{" << d.id << "} "; } std::cout << std::endl;
    std::cout << "ProcessQueriesJoinedDeque finished" << std::endl;
    return result;
}

std::vector<Document> ProcessQueriesJoinedVector(
    const SearchServer& search_server,
    const std::vector<std::string>& queries)
{
    std::vector<Document> result;
    auto process_queries = ProcessQueries(search_server, queries);
//    for (auto i : process_queries) { for (auto j : i) { std::cout << "{" << j.id << "} "; } } std::cout << std::endl;
    //std::cout << "ProcessQueriesJoinedVector started" << std::endl;
    for (const auto& i : process_queries) { for (const auto& j : i) { result.push_back(j); } } // trivial way
    for_each(std::execution::par,
        process_queries.begin(), process_queries.end(),
        [&result](const std::vector<Document>& v) {
            for (const auto& q : v) { result.push_back(q); }
        });//
    //for (const auto& d : result) { std::cout << "{" << d.id << "} "; } std::cout << std::endl;
    //std::cout << "ProcessQueriesJoinedVector finished" << std::endl;
    return result;
}//*/