#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <map>
#include <cmath>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5U;
const int ONE = 1U;
const double DOUBLE_ONE = 1.0f;
const int ZERO = 0U;
const char SPACE = ' ';
const char MINUS = '-';

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

struct Query {
    set<string> minus_words;
    set<string> plus_words;
};


int ReadLineWithNumber() {
    int result = 0;
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
    int id;
    double relevance = 0.0;
};

class SearchServer {
public:
    void SetStopWords(const string& text) {
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }

    void AddDocument(int document_id, const string& document) {
        ++documents_count_;
        const vector<string> query = SplitIntoWordsNoStop(document);
        const int query_size = query.size();
        const double term_frequency = DOUBLE_ONE / query_size;

        for (const string& word : query) {
            documents_[word][document_id] += term_frequency;
        }
    }

    vector<Document> FindTopDocuments(const string& raw_query) const {
        const Query query_words = ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(query_words);

        sort(matched_documents.begin(), matched_documents.end(),
             [](const Document& lhs, const Document& rhs) {
                 return lhs.relevance > rhs.relevance;
             });

        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }

        return matched_documents;
    }

private:
    map<string, map<int, double>> documents_;
    set<string> stop_words_;
    int documents_count_ = 0;

    bool IsStopWord(const string& word) const {
        return stop_words_.count(word) > ZERO;
    }

    vector<string> SplitIntoWordsNoStop(const string& text) const {
        vector<string> words;

        for (const string& word : SplitIntoWords(text)) {
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }

        return words;
    }

    Query ParseQuery(const string& text) const {
        Query query_words;

        for (string word : SplitIntoWordsNoStop(text)) {
            if (word[ZERO] == MINUS){
                query_words.minus_words.insert(word.substr(ONE));
            }
            query_words.plus_words.insert(word);
        }

        return query_words;
    }
    
    static double CountInverseDocumentFrequency(
        const int& amount_documents, const int& size_word_ids ) {
        return log( static_cast<double>(amount_documents) /size_word_ids );
    }

    vector<Document> FindAllDocuments(const Query& query_words) const {
        vector<Document> matched_documents;
        map<int, double> doc_relevance;

        for (const string& plus : query_words.plus_words) {
            if (documents_.count(plus) != ZERO ){
                const auto& word_ids = documents_.at(plus);
                const double inverse_doc_frequency = CountInverseDocumentFrequency(
                    documents_count_, word_ids.size()
                );
                for (const auto& [id, term_frequency] : word_ids ) {
                    doc_relevance[id] += term_frequency * inverse_doc_frequency;
                }
            }
        }

        for ( const string& word : query_words.minus_words ){
            if (documents_.count(word) != ZERO ){
                for (const auto& [id, term_frequency]  : documents_.at(word)){
                    if (doc_relevance.count(id) != ZERO ){
                        doc_relevance.erase(id);
                    }
                }
            }
        }

        for (const auto& [id, relevance] : doc_relevance){
            matched_documents.push_back({id, relevance});
        }

        return matched_documents;
    }
};

SearchServer CreateSearchServer() {
    SearchServer search_server;
    search_server.SetStopWords(ReadLine());
    const int document_count = ReadLineWithNumber();

    for (int document_id = ZERO; document_id < document_count; ++document_id) {
        search_server.AddDocument(document_id, ReadLine());
    }

    return search_server;
}

int main() {
    const SearchServer search_server = CreateSearchServer();
    const string query = ReadLine();

    for (const auto& [document_id, relevance] : search_server.FindTopDocuments(query)) {
        cout << "{ document_id = "s << document_id << ", "
             << "relevance = "s << relevance << " }"s << endl;
    }
}