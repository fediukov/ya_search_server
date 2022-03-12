#pragma once

#include <algorithm>
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
    int time_ = 0;
    const static int min_in_day_ = 1440;
    std::deque<bool> requests_;
    const SearchServer& search_server_;
};

template <typename DocumentPredicate>
std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate)
{
    ++time_;
    if (time_ > min_in_day_) {
        requests_.pop_front();
        --time_;
    }

    requests_.push_back(search_server_.FindTopDocuments(raw_query, document_predicate).size());

    return search_server_.FindTopDocuments(raw_query, document_predicate);
}
