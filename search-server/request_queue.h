#pragma once

#include <deque>
#include <string>
#include <vector>
#include "search_server.h"

class RequestQueue {
public:
    RequestQueue() = default;
    explicit RequestQueue(const SearchServer& search_server);

    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate);

    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus status);
    std::vector<Document> AddFindRequest(const std::string& raw_query);
    int GetNoResultRequests() const;
private:
    struct QueryResult {
        std::vector<Document> search_results_;
        bool is_not_empty_;
    };
    int time_ = 0;
    const static int min_in_day_ = 1440;
    std::deque<QueryResult> requests_;
    const SearchServer& search_server_;
};

RequestQueue::RequestQueue(const SearchServer& search_server)
    : search_server_(search_server)
{
}

template <typename DocumentPredicate>
std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate)
{
    ++time_;
    if (time_ > min_in_day_) {
        requests_.pop_front();
        --time_;
    }

    QueryResult query_result;
    if (search_server_.FindTopDocuments(raw_query, document_predicate).size() != 0)
    {
        query_result.is_not_empty_ = true;
    }
    else
    {
        query_result.is_not_empty_ = false;
    }
    query_result.search_results_ = search_server_.FindTopDocuments(raw_query, document_predicate);
    requests_.push_back(query_result);

    return requests_.back().search_results_;
}

std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentStatus status) {
    return AddFindRequest(raw_query, [status]([[maybe_unused]] int document_id, DocumentStatus document_status, [[maybe_unused]] int rating) { return document_status == status; });
}

std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query) {
    return AddFindRequest(raw_query, DocumentStatus::ACTUAL);
}

int RequestQueue::GetNoResultRequests() const {
    int count = 0;
    for (const auto& request : requests_)
    {
        if (!request.is_not_empty_)
        {
            ++count;
        }
    }
    return count;
}
