#include "process_queries.h"

std::vector<std::vector<Document>> ProcessQueries(
    const SearchServer& search_server,
    const std::vector<std::string>& queries)
{
    //LOG_DURATION("ProcessQueries");
    std::vector<std::vector<Document>> result(queries.size());
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

