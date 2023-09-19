#include "document.h"

std::ostream& operator<<(std::ostream& output, const Document& doc) {
    output << "{ document_id = "s << doc.id << ", relevance = "s
        << doc.relevance << ", rating = "s << doc.rating << " }"s;
    return output;
}
