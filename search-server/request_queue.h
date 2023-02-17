#pragma once

#include "search_server.h"

#include <deque>

class RequestQueue {
public:
    explicit RequestQueue(const SearchServer& search_server)
            :search_server_(search_server) {}

    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate) {
        std::vector<Document> queue = search_server_.FindTopDocuments(raw_query, document_predicate);
        QueryResult result;
        if (queue.empty())
        {
            ++no_res_q_num;
            result.no_result = queue;
            requests_.push_back(result);
        }
        else
        {
            result.wth_result = queue;
            requests_.push_back(result);
        }
        if (requests_.size() > min_in_day_)
        {
            requests_.pop_front();
            if (!queue.empty() || no_res_q_num > min_in_day_)
            {
                --no_res_q_num;
            }
        }
        return queue;
    }

    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus status) {
        return AddFindRequest(
            raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
                return document_status == status;
            });
    }

    std::vector<Document> AddFindRequest(const std::string& raw_query) {
        return AddFindRequest(raw_query, DocumentStatus::ACTUAL);
    }

    int GetNoResultRequests() const {
        return no_res_q_num;
    }

private:
    struct QueryResult {
        std::vector<Document> no_result;
        std::vector<Document> wth_result;
    };

    std::deque<QueryResult> requests_;
    const static int min_in_day_ = 1440;
    const SearchServer& search_server_;
    int no_res_q_num = 0;
}; 