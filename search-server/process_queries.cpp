#include "process_queries.h"

std::vector<std::vector<Document>> ProcessQueries(const SearchServer& search_server, 
                                            const std::vector<std::string>& queries){
    std::vector<std::vector<Document>> to_ret(queries.size());
    std::transform(std::execution::par, queries.begin(), queries.end(), to_ret.begin(), 
            [&search_server](std::string qs){return search_server.FindTopDocuments(qs);});
    return to_ret;
}

std::vector<Document> ProcessQueriesJoined(const SearchServer& search_server,
                                    const std::vector<std::string>& queries){
    std::vector<Document> to_ret;
    for (std::vector<Document> docs : ProcessQueries(search_server, queries)) {
        to_ret.insert(to_ret.end(), docs.begin(), docs.end());
    }
    return to_ret;
}