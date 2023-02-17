#pragma once

#include "document.h"

#include <iostream>
#include <string>

using namespace std::string_literals;

template <typename Iterator>
struct IteratorRange {
    Iterator begin;
    Iterator end;
    size_t range = 0;
};


template <typename Iterator>
class Paginator {
    public:

    Paginator(Iterator begin, Iterator end, size_t range) {
        for (Iterator i = begin; i != end; i += range)
        {
            if (distance(i, end) > range)
            {
                page.push_back({i, i + range, range});
            }
            else
            {
                page.push_back({i, i + distance(i, end), range});
                break;
            }
        }
    }

    auto begin() const {
        return page.begin();
    }

    auto end() const {
        return page.end();
    }

    private:

    std::vector<IteratorRange<Iterator>> page;
}; 


std::ostream& operator <<(std::ostream& out, const Document& doc) {
    return out << "{ document_id = "s << doc.id << ", relevance = "s
        << doc.relevance << ", rating = "s << doc.rating << " }"s;
}

template <typename Iterator>
std::ostream& operator <<(std::ostream& out, IteratorRange<Iterator> range) {
    for (Iterator it = range.begin; it != range.end; ++it) 
    {
        out << *it;
    }
    return out;
}

template <typename Container>
auto Paginate(const Container& c, size_t page_size) {
    return Paginator(begin(c), end(c), page_size);
}