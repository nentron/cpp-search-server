//Вставьте сюда своё решение из урока «Очередь запросов» темы «Стек, очередь, дек».‎
#pragma once

struct Document {
    Document() = default;
    Document(int doc_id, double doc_relevance, int doc_rating)
    : id(doc_id)
    , relevance(doc_relevance)
    , rating(doc_rating){}

    int id = 0;
    double relevance = 0.0;
    int rating = 0;
};

enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};