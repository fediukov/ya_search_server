#include "document.h"

using namespace std;

Document::Document(int id, double relevance, int rating)
    : id(id)
    , relevance(relevance)
    , rating(rating)
{
}

void PrintDocument(const Document& document) {
    //using namespace std::string_literals;
    cout << "{ "s
        << "document_id = "s << document.id << ", "s
        << "relevance = "s << document.relevance << ", "s
        << "rating = "s << document.rating << " }"s << endl;
}

void PrintMatchDocumentResult(int document_id, const vector<string_view>& words, DocumentStatus status) {
    //using namespace std::string_literals;
    cout << "{ "s
        << "document_id = "s << document_id << ", "s
        << "status = "s << static_cast<int>(status) << ", "s
        << "words ="s;
    for (string_view word : words) {
        cout << ' ' << word;
    }
    cout << "}"s << endl;
}

ostream& operator<<(ostream& out, const Document& document) {
    //using namespace std; 
    out << "{ "s
        << "document_id = "s << document.id << ", "s
        << "relevance = "s << document.relevance << ", "s
        << "rating = "s << document.rating << " }"s;
    return out;
}
