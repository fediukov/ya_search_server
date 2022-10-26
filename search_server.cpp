#include "search_server.h"

SearchServer::SearchServer(const std::string_view stop_words_text)
    : SearchServer(SplitIntoWords(stop_words_text))
{
}

SearchServer::SearchServer(const std::string& stop_words_text)
    : SearchServer(SplitIntoWords(stop_words_text))
{
}//*/

void SearchServer::AddDocument(int document_id, const std::string_view document, DocumentStatus status, const std::vector<int>& ratings) {

    if ((document_id < 0) ||
        (documents_.count(document_id) > 0)) {
        throw std::invalid_argument("Invalid document_id");
    }

    const auto words = SplitIntoWordsNoStop(document);
    const double inv_word_count = 1.0 / words.size();

    for (const auto word : words) {
        auto it = words_.insert(std::string(word));
        std::string_view word_sv = *(it.first);
        word_to_document_freqs_[word_sv][document_id] += inv_word_count;
        document_to_word_freqs_[document_id][word_sv] += inv_word_count;
    }

    documents_.emplace(document_id, DocumentData{ ComputeAverageRating(ratings), status });

    document_ids_.insert(document_id);
}

std::vector<Document> SearchServer::FindTopDocuments(const std::string_view raw_query, DocumentStatus status) const {
    return FindTopDocuments(std::execution::seq, raw_query, status);
}

std::vector<Document> SearchServer::FindTopDocuments(const std::string_view raw_query) const {
    return FindTopDocuments(std::execution::seq, raw_query);
}

int SearchServer::GetDocumentCount() const {
    return documents_.size();
}

std::set<int>::const_iterator SearchServer::begin() const {
    return document_ids_.begin();
}

std::set<int>::const_iterator SearchServer::end() const {
    return document_ids_.end();
}

const std::map<std::string_view, double>&
SearchServer::GetWordFrequencies(int document_id) const {
    static std::map<std::string_view, double> empty_map;
    auto it = document_to_word_freqs_.find(document_id);
    if (it != document_to_word_freqs_.end())
    {
        return ((*it).second);
    }
    else
    {
        return empty_map;
    }
}

void SearchServer::RemoveDocument(int document_id) {
    RemoveDocument(std::execution::seq, document_id);
}

// execution sequenced_policy
void SearchServer::RemoveDocument(const std::execution::sequenced_policy& policy, int document_id) {
    // remove from document_ids_
    document_ids_.erase(document_id);
    // remove from documents_
    documents_.erase(document_id);
    // remove from word_to_document_freqs_
    for_each(policy, word_to_document_freqs_.begin(),
        word_to_document_freqs_.end(),
        [document_id](auto& p)
        { p.second.erase(document_id); }
    );//*/
    // remove from document_to_word_freqs_
    document_to_word_freqs_.erase(document_id);
}

// execution parallel_policy
void SearchServer::RemoveDocument(const std::execution::parallel_policy& policy, int document_id) {
    // remove from document_ids_
    document_ids_.erase(document_id);
    // remove from documents_
    documents_.erase(document_id);
    // remove from word_to_document_freqs_
    const std::map<std::string_view, double>& m = GetWordFrequencies(document_id);
    std::vector<std::string_view> v(m.size());
    transform(policy, m.begin(), m.end(), v.begin(),
        [](auto& p) { return p.first; });
    for_each(policy, v.begin(), v.end(),
        [this, document_id](auto& word)
        { word_to_document_freqs_[word].erase(document_id); }
    );
    // remove from document_to_word_freqs_
    document_to_word_freqs_.erase(document_id);
}

std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(const std::string_view raw_query, int document_id) const {
    return MatchDocument(std::execution::seq, raw_query, document_id);

}

// execution sequenced_policy
std::tuple<std::vector<std::string_view>, DocumentStatus>
SearchServer::MatchDocument(const std::execution::sequenced_policy& policy, const std::string_view raw_query, int document_id) const {
    if ((document_id < 0) || (documents_.count(document_id) == 0)) {
        throw std::invalid_argument("document_id out of range");
    }

    const auto query = SearchServer::ParseQuery(std::execution::seq, raw_query);
    std::vector<std::string_view> matched_words;
    const std::map<std::string_view, double>& word_freq = GetWordFrequencies(document_id);

    bool is_minus = any_of(//policy,
        query.minus_words.begin(), query.minus_words.end(),
        [&word_freq](const auto& word) { return word_freq.count(word); }
    );
    if (is_minus)
    {
        return { {}, documents_.at(document_id).status };
    }

    copy_if(//policy,
        query.plus_words.begin(), query.plus_words.end(),
        std::back_inserter(matched_words),
        [&word_freq, &matched_words](std::string_view word) {
            return word_freq.count(word);
        });

    sort(//std::execution::seq,
        matched_words.begin(), matched_words.end());
    auto last = unique(matched_words.begin(), matched_words.end());
    matched_words.erase(last, matched_words.end()); //*/

    return { matched_words, documents_.at(document_id).status };
}

// execution parallel_policy
std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(const std::execution::parallel_policy& policy, const std::string_view raw_query, int document_id) const {
    if ((document_id < 0) || (documents_.count(document_id) == 0)) {
        throw std::invalid_argument("document_id out of range");
    }

    const auto query = ParseQuery(std::execution::par, raw_query);
    std::vector<std::string_view> matched_words;
    matched_words.reserve(query.plus_words.size());

    const std::map<std::string_view, double>& word_freq = GetWordFrequencies(document_id);

    bool is_minus = any_of(//policy,
        query.minus_words.begin(), query.minus_words.end(),
        [&word_freq](const auto& word) { return word_freq.count(word); }
    );
    if (is_minus)
    {
        return { {}, documents_.at(document_id).status };
    }

    copy_if(//policy,
        query.plus_words.begin(), query.plus_words.end(),
        std::back_inserter(matched_words),
        [&word_freq, &matched_words](std::string_view word) {
            return word_freq.count(word);
        });

    sort(//std::execution::par,
        matched_words.begin(), matched_words.end());
    auto last = unique(matched_words.begin(), matched_words.end());
    matched_words.erase(last, matched_words.end()); //*/

    return { matched_words, documents_.at(document_id).status };
}

bool SearchServer::IsStopWord(const std::string_view word) const {
    return stop_words_.count(word) > 0;
}

bool SearchServer::IsValidWord(const std::string_view word) {
    return std::none_of(word.begin(), word.end(),
        [](char c) {
            return c >= '\0' && c < ' ';
        });
}

std::vector<std::string_view> SearchServer::SplitIntoWordsNoStop(const std::string_view text) const {
    std::vector<std::string_view> words;
    for (const auto word : SplitIntoWords(text)) {
        if (!IsValidWord(word)) {
            throw std::invalid_argument(
                "Word " + std::string(word) + " is invalid");
        }
        if (!IsStopWord(word) && word.size() > 0) {
            words.push_back(word);
        }
    }
    return words;
}

int SearchServer::ComputeAverageRating(const std::vector<int>& ratings) {
    if (ratings.empty()) {
        return 0;
    }
    return std::accumulate(ratings.begin(), ratings.end(), 0) / static_cast<int>(ratings.size());
}

double SearchServer::ComputeWordInverseDocumentFreq(const std::string_view word) const {
    return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
}

SearchServer::QueryWord SearchServer::ParseQueryWord(const std::string_view text) const {
    if (text.empty()) {
        throw std::invalid_argument("Query word is empty");
    }

    std::string_view word = text;
    bool is_minus = false;
    if (word[0] == '-') {
        is_minus = true;
        word = word.substr(1);
    }
    if (word.empty() || word[0] == '-' || !IsValidWord(word)) {
        throw std::invalid_argument("Query word " + std::string(text) + " is invalid");
    }
    return { word, is_minus, IsStopWord(word) };
}

SearchServer::Query SearchServer::ParseQuery(const std::execution::sequenced_policy& policy, const std::string_view text) const {
    Query result;
    for (const auto word : SplitIntoWords(text)) {
        const auto query_word = ParseQueryWord(word);
        if (!query_word.is_stop) {
            if (query_word.is_minus) {
                result.minus_words.push_back(query_word.data); // 
            }
            else {
                result.plus_words.push_back(query_word.data); //
            }
        }
    }

    // sort -> unique -> erase is here for this version ParseQuery
    sort(result.plus_words.begin(), result.plus_words.end());
    std::vector<std::string_view>::const_iterator last = unique(result.plus_words.begin(), result.plus_words.end());
    result.plus_words.erase(last, result.plus_words.end());
    sort(result.minus_words.begin(), result.minus_words.end());
    last = unique(result.minus_words.begin(), result.minus_words.end());
    result.minus_words.erase(last, result.minus_words.end());

    return result;
}

SearchServer::Query SearchServer::ParseQuery(const std::execution::parallel_policy& policy, const std::string_view text) const {
    Query result;
    for (const std::string_view word : SplitIntoWords(text)) {
        const auto query_word = ParseQueryWord(word);
        if (!query_word.is_stop) {
            if (query_word.is_minus) {
                result.minus_words.push_back(query_word.data);
            }
            else {
                result.plus_words.push_back(query_word.data);
            }
        }
    }

    // sort -> unique -> erase is in MatchDocument for this version ParseQuery

    return result;
}

