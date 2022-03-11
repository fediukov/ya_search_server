#pragma once

#include <iostream>
#include <string>
#include <vector>

using namespace std;

enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};

struct Document {
    Document() = default;

    Document(int id, double relevance, int rating)
        : id(id)
        , relevance(relevance)
        , rating(rating) {
    }

    int id = 0;
    double relevance = 0.0;
    int rating = 0;
};

void PrintDocument(const Document& document);
void PrintMatchDocumentResult(int document_id, const vector<string>& words, DocumentStatus status);

ostream& operator<<(ostream& out, const Document& document);
/*{
    using namespace std;
    out << "{ "s
        << "document_id = "s << document.id << ", "s
        << "relevance = "s << document.relevance << ", "s
        << "rating = "s << document.rating << " }"s;
    return out;
}*/

