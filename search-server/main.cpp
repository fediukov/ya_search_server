// search_server_s4_t2_solution.cpp
#include <iostream>
#include <string>
#include <vector>

#include "paginator.h"
#include "read_input_functions.h"
#include "remove_duplicates.h"
#include "request_queue.h"
#include "search_server.h"

void AddDocument(SearchServer& search_server, int document_id, const std::string& document, DocumentStatus status,
    const std::vector<int>& ratings) {
    using namespace std;
    try {
        search_server.AddDocument(document_id, document, status, ratings);
    }
    catch (const invalid_argument& e) {
        cout << "Ошибка добавления документа "s << document_id << ": "s << e.what() << endl;
    }
}

void FindTopDocuments(const SearchServer& search_server, const std::string& raw_query) {
    using namespace std;
    cout << "Результаты поиска по запросу: "s << raw_query << endl;
    try {
        for (const Document& document : search_server.FindTopDocuments(raw_query)) {
            PrintDocument(document);
        }
    }
    catch (const invalid_argument& e) {
        cout << "Ошибка поиска: "s << e.what() << endl;
    }
}

void MatchDocuments(const SearchServer& search_server, const std::string& query) {
    using namespace std;
    try {
        cout << "Матчинг документов по запросу: "s << query << endl;
        for (auto document_id : search_server)
        {
            const auto [words, status] = search_server.MatchDocument(query, document_id);
            PrintMatchDocumentResult(document_id, words, status);
        }
        /* // старая реализация
        const int document_count = search_server.GetDocumentCount();
        for (int index = 0; index < document_count; ++index) {
            const int document_id = search_server.GetDocumentId(index);
            const auto [words, status] = search_server.MatchDocument(query, document_id);
            PrintMatchDocumentResult(document_id, words, status);
        }
        */
    }
    catch (const invalid_argument& e) {
        cout << "Ошибка матчинга документов на запрос "s << query << ": "s << e.what() << endl;
    }
}

// /* -------------------- main -------------------- */ //
int main() {
    using namespace std;
    {
        SearchServer search_server("and in at"s);
        RequestQueue request_queue(search_server);

        search_server.AddDocument(1, "curly cat curly tail"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
        search_server.AddDocument(2, "curly dog and fancy collar"s, DocumentStatus::ACTUAL, { 1, 2, 3 });
        search_server.AddDocument(3, "big cat fancy collar "s, DocumentStatus::ACTUAL, { 1, 2, 8 });
        search_server.AddDocument(4, "big dog sparrow Eugene"s, DocumentStatus::ACTUAL, { 1, 3, 2 });
        search_server.AddDocument(5, "big dog sparrow Vasiliy"s, DocumentStatus::ACTUAL, { 1, 1, 1 });

        // 1439 запросов с нулевым результатом
        for (int i = 0; i < 1439; ++i) {
            request_queue.AddFindRequest("empty request"s);
        }
        // все еще 1439 запросов с нулевым результатом
        request_queue.AddFindRequest("curly dog"s);
        // новые сутки, первый запрос удален, 1438 запросов с нулевым результатом
        request_queue.AddFindRequest("big collar"s);
        // первый запрос удален, 1437 запросов с нулевым результатом
        request_queue.AddFindRequest("sparrow"s);
        cout << "Total empty requests: "s << request_queue.GetNoResultRequests() << endl;
        // проверка матчинга документов
        MatchDocuments(search_server, "yellow big cat");
        // проверка работы функции GetWordFrequencies в search_server
        int id_of_checking_freq = 3;
        map<string, double> freqs = search_server.GetWordFrequencies(id_of_checking_freq);
        cout << "Checking frequencies of document " << id_of_checking_freq << ":" << endl;
        for (const auto& freq : freqs)
        {
            cout << "{" << freq.first << " - " << freq.second << "}" << endl;
        }
        // проверка удаления документа
        search_server.RemoveDocument(id_of_checking_freq);
        MatchDocuments(search_server, "yellow big cat");
    }

    {
        // проверка дубликатов
        SearchServer search_server("and with"s);

        AddDocument(search_server, 1, "funny pet and nasty rat"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
        AddDocument(search_server, 2, "funny pet with curly hair"s, DocumentStatus::ACTUAL, { 1, 2 });

        // дубликат документа 2, будет удалён
        AddDocument(search_server, 3, "funny pet with curly hair"s, DocumentStatus::ACTUAL, { 1, 2 });

        // отличие только в стоп-словах, считаем дубликатом
        AddDocument(search_server, 4, "funny pet and curly hair"s, DocumentStatus::ACTUAL, { 1, 2 });

        // множество слов такое же, считаем дубликатом документа 1
        AddDocument(search_server, 5, "funny funny pet and nasty nasty rat"s, DocumentStatus::ACTUAL, { 1, 2 });

        // добавились новые слова, дубликатом не является
        AddDocument(search_server, 6, "funny pet and not very nasty rat"s, DocumentStatus::ACTUAL, { 1, 2 });

        // множество слов такое же, как в id 6, несмотря на другой порядок, считаем дубликатом
        AddDocument(search_server, 7, "very nasty rat and not very funny pet"s, DocumentStatus::ACTUAL, { 1, 2 });

        // есть не все слова, не является дубликатом
        AddDocument(search_server, 8, "pet with rat and rat and rat"s, DocumentStatus::ACTUAL, { 1, 2 });

        // слова из разных документов, не является дубликатом
        AddDocument(search_server, 9, "nasty rat with curly hair"s, DocumentStatus::ACTUAL, { 1, 2 });

        cout << "Before duplicates removed: "s << search_server.GetDocumentCount() << endl;
        RemoveDuplicates(search_server);
        cout << "After duplicates removed: "s << search_server.GetDocumentCount() << endl;
    }

    return 0;
}