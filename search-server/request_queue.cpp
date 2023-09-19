#include "request_queue.h"


RequestQueue::RequestQueue(const SearchServer& server)
    : server_(server){}

std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentStatus status) {
    return AddFindRequest(raw_query, [status](int id, DocumentStatus doc_status, int rating){
        return status == doc_status;
    });
}

std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query) {
        return AddFindRequest(raw_query, DocumentStatus::ACTUAL);
}

int RequestQueue::GetNoResultRequests() const {
    return static_cast<int>(empty_result_.size());
}
