#ifndef SONG_HPP
#define SONG_HPP

#include <string>
#include <map>
#include <memory>
#include <vector>
#include <iostream>
#include <cctype>
#include "Diccionario.hpp"

struct Song {
    string title;
    string text; 
    map<string,int> word_counts;

    Song() = default;
    Song(string t, string txt): title(move(t)), text(move(txt)) {
        index_words();
    }

    void index_words() {
        word_counts.clear();
        string token;
        for (size_t i = 0; i < text.size(); ++i) {
            char c = text[i];
            if (isalnum((unsigned char)c) || c == '\'') token += (char)tolower((unsigned char)c);
            else {
                if (!token.empty()) { ++word_counts[token]; token.clear(); }
            }
        }
        if (!token.empty()) ++word_counts[token];
    }
};

struct BSTNode {
    Song song;
    unique_ptr<BSTNode> left, right;
    BSTNode(Song s): song(move(s)), left(nullptr), right(nullptr) {}
};

class SongBST {
    unique_ptr<BSTNode> root;

    static void inorder_collect(BSTNode* node, vector<const Song*>& out) {
        if (!node) return;
        inorder_collect(node->left.get(), out);
        out.push_back(&node->song);
        inorder_collect(node->right.get(), out);
    }

    static BSTNode* find_min(BSTNode* node) {
        while (node && node->left) node = node->left.get();
        return node;
    }

    static unique_ptr<BSTNode> remove_rec(unique_ptr<BSTNode> node, const string& title, bool &removed) {
        if (!node) return nullptr;
        if (title < node->song.title) node->left = remove_rec(move(node->left), title, removed);
        else if (title > node->song.title) node->right = remove_rec(move(node->right), title, removed);
        else {
            removed = true;
            if (!node->left) return move(node->right);
            if (!node->right) return move(node->left);
            BSTNode* succ = find_min(node->right.get());
            node->song = succ->song;
            node->right = remove_rec(move(node->right), succ->song.title, removed);
        }
        return node;
    }

public:
    SongBST(): root(nullptr) {}

    void insert(Song s) {
        if (!root) { root = make_unique<BSTNode>(move(s)); return; }
        BSTNode* cur = root.get();
        while (true) {
            if (s.title == cur->song.title) {
                cur->song = move(s);
                return;
            } else if (s.title < cur->song.title) {
                if (cur->left) cur = cur->left.get(); else { cur->left = make_unique<BSTNode>(move(s)); return; }
            } else {
                if (cur->right) cur = cur->right.get(); else { cur->right = make_unique<BSTNode>(move(s)); return; }
            }
        }
    }

    const Song* find(const string& title) const {
        BSTNode* cur = root.get();
        while (cur) {
            if (title == cur->song.title) return &cur->song;
            cur = (title < cur->song.title) ? cur->left.get() : cur->right.get();
        }
        return nullptr;
    }

    bool remove(const string& title) {
        bool removed = false;
        root = remove_rec(move(root), title, removed);
        return removed;
    }

    vector<const Song*> all_songs_inorder() const {
        vector<const Song*> out;
        inorder_collect(root.get(), out);
        return out;
    }

    void print_inorder() const {
        auto v = all_songs_inorder();
        for (auto s : v) {
            cout << "- " << s->title << "\n";
        }
    }
};

#endif
