#ifndef UTIL_HPP
#define UTIL_HPP

#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <cmath>
#include <iostream>
#include "Song.hpp"

using SparseVec = vector<pair<int,double>>;

struct Vocabulary {
    map<string,int> index_of; 
    vector<string> words;

    int ensure(const string& w) {
        auto it = index_of.find(w);
        if (it != index_of.end()) return it->second;
        int idx = (int)words.size();
        words.push_back(w);
        index_of[w] = idx;
        return idx;
    }

    int get_index(const string& w) const {
        auto it = index_of.find(w);
        return it == index_of.end() ? -1 : it->second;
    }

    size_t size() const { return words.size(); }
};

SparseVec build_sparse(const Song& s, Vocabulary& V) {
    SparseVec out;
    for (const auto &p : s.word_counts) {
        int idx = V.ensure(p.first);
        out.emplace_back(idx, (double)p.second);
    }
    sort(out.begin(), out.end(), [](auto &a, auto &b){ return a.first < b.first; });
    return out;
}

double cosine_similarity(const SparseVec& a, const SparseVec& b) {
    size_t ia = 0, ib = 0;
    double dot = 0.0;
    double na = 0.0, nb = 0.0;

    while (ia < a.size() && ib < b.size()) {
        if (a[ia].first == b[ib].first) {
            dot += a[ia].second * b[ib].second;
            na += a[ia].second * a[ia].second;
            nb += b[ib].second * b[ib].second;
            ++ia; ++ib;
        } else if (a[ia].first < b[ib].first) {
            na += a[ia].second * a[ia].second;
            ++ia;
        } else {
            nb += b[ib].second * b[ib].second;
            ++ib;
        }
    }

    while (ia < a.size()) { na += a[ia].second * a[ia].second; ++ia; }
    while (ib < b.size()) { nb += b[ib].second * b[ib].second; ++ib; }

    if (na == 0.0 || nb == 0.0) return 0.0;
    return dot / (sqrt(na) * sqrt(nb));
}

vector<pair<const Song*, double>> detect_similar(
    const Song& s, const SongBST& tree, Vocabulary& V, double threshold=0.6
) {
    vector<pair<const Song*, double>> found;

    SparseVec sv = build_sparse(s, V);
    auto all = tree.all_songs_inorder();

    for (auto other : all) {
        if (other->title == s.title) continue;
        SparseVec ov = build_sparse(*other, V);
        double sim = cosine_similarity(sv, ov);
        if (sim >= threshold) found.emplace_back(other, sim);
    }

    sort(found.begin(), found.end(), [](auto &a, auto &b){
        return a.second > b.second;
    });

    return found;
}

string trim(const string& s) {
    size_t a = s.find_first_not_of(" \t\n\r");
    if (a == string::npos) return "";
    size_t b = s.find_last_not_of(" \t\n\r");
    return s.substr(a, b - a + 1);
}

string ask_line(const string& prompt) {
    cout << prompt;
    string s; getline(cin, s);
    return s;
}

#endif
