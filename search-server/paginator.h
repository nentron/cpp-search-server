#pragma once

#include <iostream>
#include <stdexcept>
#include <vector>


template <typename Iterator>
class RangeIterator{
public:
    RangeIterator(Iterator begin_range, Iterator end_range)
        : begin_(begin_range), end_(end_range){}

    Iterator begin(){
        return begin_;
    }
    Iterator end(){
        return end_;
    }
    size_t size(){
        return distance(begin_, end_);
    }
private:
    Iterator begin_;
    Iterator end_;
};

template <typename Iterator>
class Paginator{
public:
    Paginator(Iterator begin_range, Iterator range_end, size_t length)
    : pages_(Paginate_(begin_range, range_end, length)){
    }
    inline auto begin() const {
        return pages_.begin();
    }
    inline auto end() const {
        return pages_.end();
    }

    Paginator& operator++(){
        next(pages_.begin());
        return *this;
    }

private:
    std::vector<RangeIterator<Iterator>> Paginate_(Iterator begin_range, Iterator range_end, size_t length){
        std::vector<RangeIterator<Iterator>> pages;
        int count_pages = distance(begin_range, range_end) / length;
        auto mid = begin_range;
        for (int i = 0; i < count_pages; ++i){
            advance(mid, length);
            pages.push_back(RangeIterator(begin_range, mid));
            begin_range = mid;
        }
        if (distance(begin_range, range_end) != 0){
            pages.push_back(RangeIterator(mid, range_end));
        }
        return pages;
    }
    std::vector<RangeIterator<Iterator>> pages_;
};

template <typename Container>
auto Paginate(const Container& c, size_t page_size) {
    return Paginator(begin(c), end(c), page_size);
}

template <typename Iterator>
std::ostream& operator<<(std::ostream& output, RangeIterator<Iterator> iter){
    for (auto it = iter.begin(); it != iter.end(); ++it){
        output << *it;
    }
    return output;
}
