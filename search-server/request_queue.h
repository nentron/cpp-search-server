#pragma once

#include "document.h"
#include "search_server.h"

#include <deque>
#include <iostream>
#include <string>
#include <vector>


class RequestQueue {
public:
    explicit RequestQueue(const SearchServer& search_server);

    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus status);

    std::vector<Document> AddFindRequest(const std::string& raw_query);

    int GetNoResultRequests() const;

    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate);


private:
    const SearchServer& server_;
    struct QueryResult {
        std::string query;
        std::vector<Document> docs;
    };
    std::deque<QueryResult> requests_;
    std::deque<QueryResult> empty_result_;
    const static int min_in_day_ = 1440;
};


template <typename DocumentPredicate>
std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate){
    std::vector<Document> result = server_.FindTopDocuments(raw_query, document_predicate);
    if (result.empty()){
        empty_result_.push_back({raw_query, result});
    } else {
        requests_.push_back({raw_query, result});
    }

    if ((requests_.size() + empty_result_.size()) > min_in_day_){
        if (!empty_result_.empty()){
            empty_result_.pop_front();
        } else {
            requests_.pop_front();
        }
    }
    return result;
}