#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <set>
#include <string>
#include <utility>
#include <map>
#include <numeric>
#include <vector>

using namespace std;


const int MAX_RESULT_DOCUMENT_COUNT = 5;
const int ONE = 1;
const double DOUBLE_ONE = 1.0;
const int ZERO = 0;
const char SPACE = ' ';
const char MINUS = '-';
const double ELIPSON = 1e-6;


string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result;
    cin >> result;
    ReadLine();
    return result;
}

vector<string> SplitIntoWords(const string& text) {
    vector<string> words;
    string word;
    for (const char c : text) {
        if (c == SPACE) {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        } else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }

    return words;
}


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


template <typename StringContainer>
set<string> MakeUniqueNonEmptyStrings(const StringContainer& strings) {
    set<string> non_empty_strings;
    for (const string& str : strings) {
        if (!str.empty()) {
            non_empty_strings.insert(str);
        }
    }
    return non_empty_strings;
}

enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};

class SearchServer {
public:
    template <typename StringContainer>
    explicit SearchServer(const StringContainer& stop_words)
        : stop_words_(MakeUniqueNonEmptyStrings(stop_words)) {
        if (!IsValidStopWords()){
            throw invalid_argument("Stop Words must not contain special symbols"s);
        }
    }

    explicit SearchServer(const string& stop_words_text)
        : SearchServer(SplitIntoWords(stop_words_text))
    {
    }

    inline static constexpr int INVALID_DOCUMENT_ID = -1;

    void AddDocument(int document_id, const string& document, DocumentStatus status,
                     const vector<int>& ratings) {
        if (documents_.count(document_id) != ZERO
            || document_id <= INVALID_DOCUMENT_ID){
            throw invalid_argument("Invalid document's id"s);
        }

        const auto& words = SplitIntoWordsNoStop(document);
        const double inv_word_count = DOUBLE_ONE / words.size();

        for (const string& word : words) {
            word_to_document_freqs_[word][document_id] += inv_word_count;
        }
        index_to_doc_id_.push_back(document_id);
        documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status});
    }

    template <typename KeyMapper>
    vector<Document> FindTopDocuments(const string& raw_query,
                                      KeyMapper key_mapper) const {

        const auto& query = ParseQuery(raw_query);
        vector<Document> result = FindAllDocuments(query, key_mapper);

        sort(
            result.begin(), result.end(),
            [](const Document& lhs, const Document& rhs) {
                if (abs(lhs.relevance - rhs.relevance) < ELIPSON) {
                    return lhs.rating > rhs.rating;
                } else {
                    return lhs.relevance > rhs.relevance;
                }
            }
        );

        if (result.size() > MAX_RESULT_DOCUMENT_COUNT) {
            result.resize(MAX_RESULT_DOCUMENT_COUNT);
        }

        return result;
    }

    vector<Document> FindTopDocuments(const string& raw_query) const {

        return FindTopDocuments(
            raw_query,
            [](int document_id, DocumentStatus status, int rating){
                return status == DocumentStatus::ACTUAL;
            }
        );
    }
    vector<Document> FindTopDocuments(const string& raw_query,
            const DocumentStatus& doc_status) const {

        return FindTopDocuments(
            raw_query,
            [doc_status](int document_id, DocumentStatus status, int rating){
                return status == doc_status;
            }
        );
    }

    int GetDocumentCount() const {
        return documents_.size();
    }

    tuple<vector<string>, DocumentStatus> MatchDocument(
        const string& raw_query,
        int document_id) const {
        const auto& query = ParseQuery(raw_query);
        if (document_id < ZERO){
            throw invalid_argument("Id should be positive"s);
        }
        vector<string> matched_words;

        for (const string& word : query.plus_words) {
            if (word_to_document_freqs_.count(word) == ZERO) {
                continue;
            }
            if (word_to_document_freqs_.at(word).count(document_id)) {
                matched_words.push_back(word);
            }
        }

        for (const string& word : query.minus_words) {
            if (word_to_document_freqs_.count(word) == ZERO) {
                continue;
            }
            if (word_to_document_freqs_.at(word).count(document_id)) {
                matched_words.clear();
                break;
            }
        }

        return {matched_words, documents_.at(document_id).status};
    }

    int GetDocumentId(int index) const {
        return index_to_doc_id_.at(index);
    }
private:
    struct DocumentData {
        int rating;
        DocumentStatus status;
    };
    const set<string> stop_words_;
    map<string, map<int, double>> word_to_document_freqs_;
    map<int, DocumentData> documents_;
    vector<int> index_to_doc_id_;

    bool IsStopWord(const string& word) const {
        return stop_words_.count(word) > ZERO;
    }
    bool IsValidStopWords(){
        for (const auto& word : stop_words_){
            if(!IsValidWord(word)){
                return false;
            }
        }
        return true;
    }

    vector<string> SplitIntoWordsNoStop(const string& text) const {
        vector<string> words;

        for (const string& word : SplitIntoWords(text)) {
            if (!IsValidWord(word)){
                throw invalid_argument("Unvalueable word is "s + word);
            }
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }

        return words;
    }

    static int ComputeAverageRating(const vector<int>& ratings) {
        if (ratings.empty()) {
            return ZERO;
        }
        int rating_sum = accumulate(ratings.begin(), ratings.end(), ZERO);
        return rating_sum / static_cast<int>(ratings.size());
    }

    struct QueryWord {
        string data;
        bool is_minus;
        bool is_stop;
    };

    QueryWord ParseQueryWord(string text) const {
        bool is_minus = false;

        if (text[ZERO] == MINUS) {
            is_minus = true;
            text = text.substr(ZERO);
        }
        if (!IsValidWord(text) || text[ZERO] == MINUS || text.empty()){
            throw invalid_argument("Invalid query words"s);
        }

        return {text, is_minus, IsStopWord(text)};
    }

    struct Query {
        set<string> plus_words;
        set<string> minus_words;
    };

    Query ParseQuery(const string& text) const {
        Query query;
        for (const string& word : SplitIntoWords(text)) {
            const QueryWord query_word = ParseQueryWord(word);
            if (!query_word.is_stop) {
                if (query_word.is_minus) {
                    query.minus_words.insert(query_word.data);
                } else {
                    query.plus_words.insert(query_word.data);
                }
            }
        }
        return query;
    }
    
    static bool IsValidWord(const string& word) {

        return none_of(word.begin(), word.end(), [](char c) {
            return c >= ZERO && c < SPACE;
       });
    }

    double ComputeWordInverseDocumentFreq(const string& word) const {
        return log(GetDocumentCount() * DOUBLE_ONE / word_to_document_freqs_.at(word).size());
    }

    template <typename DocumentPredicate>
    vector<Document> FindAllDocuments(const Query& query,
                                      DocumentPredicate document_predicate) const {
        map<int, double> document_to_relevance;

        for (const string& word : query.plus_words) {
            if (word_to_document_freqs_.count(word) == ZERO) {
                continue;
            }
            const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
            for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word)) {
                const auto& document_data = documents_.at(document_id);
                if (document_predicate(document_id, document_data.status, document_data.rating)) {
                    document_to_relevance[document_id] += term_freq * inverse_document_freq;
                }
            }
        }

        for (const string& word : query.minus_words) {
            if (word_to_document_freqs_.count(word) == ZERO) {
                continue;
            }
            for (const auto [document_id, _] : word_to_document_freqs_.at(word)) {
                document_to_relevance.erase(document_id);
            }
        }

        vector<Document> matched_documents;
        for (const auto [document_id, relevance] : document_to_relevance) {
            matched_documents.push_back(
                {document_id, relevance, documents_.at(document_id).rating});
        }

        return matched_documents;
    }
};
