#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <random>
#include <chrono>
#include <cctype>
#include <algorithm>
#include <memory>
#include <iomanip>

using namespace std;

// RNG
static mt19937 rng((unsigned)chrono::high_resolution_clock::now().time_since_epoch().count());

string pick_random(const vector<string>& v) {
    if (v.empty()) return "";
    uniform_int_distribution<int> dist(0, (int)v.size() - 1);
    return v[dist(rng)];
}

string join_tokens(const vector<string>& tokens) {
    ostringstream out;
    for (size_t i = 0; i < tokens.size(); ++i) {
        if (!tokens[i].empty()) {
            if (i > 0) out << " ";
            out << tokens[i];
        }
    }
    return out.str();
}

// -------------------- Diccionario (usa std::map en vez de unordered_map) --------------------
struct Diccionario {
    map<string, vector<string>> cat;

    void add(const string& category, const string& word) {
        cat[category].push_back(word);
    }

    const vector<string>& get(const string& category) const {
        static const vector<string> empty;
        auto it = cat.find(category);
        return (it != cat.end()) ? it->second : empty;
    }

    void load_sample() {
        cat = {
            {"N", {"corazon","ciudad","mar","noche","camino"}},
            {"V", {"late","brilla","canta","corre","susurra"}},
            {"Adj", {"oscuro","silencioso","eterno","dulce"}},
            {"P", {"en","sobre","bajo","entre"}},
            {"S", {"mi","tu","nuestro"}},
            {"C", {"de la madrugada","sin final","de papel"}}
        };
    }
};

// -------------------- Generadores de versos/canciones --------------------
string generar_verso_simple(const Diccionario& d) {
    vector<string> tokens;
    if (uniform_int_distribution<int>(0,1)(rng)) tokens.push_back(pick_random(d.get("S")));
    if (uniform_int_distribution<int>(0,1)(rng)) tokens.push_back(pick_random(d.get("P")));
    tokens.push_back(pick_random(d.get("N")));
    tokens.push_back(pick_random(d.get("V")));
    if (uniform_int_distribution<int>(0,1)(rng)) tokens.push_back(pick_random(d.get("C")));
    string verso = join_tokens(tokens);
    if (!verso.empty()) verso[0] = toupper((unsigned char)verso[0]);
    return verso + ".";
}

string generar_verso_poetico(const Diccionario& d) {
    vector<string> primera;
    if (uniform_int_distribution<int>(0,1)(rng)) primera.push_back(pick_random(d.get("Adj")));
    primera.push_back(pick_random(d.get("N")));
    vector<string> segunda = {
        pick_random(d.get("P")),
        pick_random(d.get("N")),
        pick_random(d.get("V"))
    };
    string verso = join_tokens(primera) + ", " + join_tokens(segunda) + ".";
    if (!verso.empty()) verso[0] = toupper((unsigned char)verso[0]);
    return verso;
}

string generar_estribillo(const Diccionario& d, int lines = 2) {
    string out;
    string hook = pick_random(d.get("N"));
    for (int i = 0; i < lines; ++i) {
        string line = "Oh, " + hook + " " + pick_random(d.get("V"));
        if (!line.empty()) line[0] = toupper((unsigned char)line[0]);
        out += line + ".\n";
    }
    return out;
}

string generar_cancion_text(Diccionario& d) {
    if (d.get("N").empty()) d.load_sample();
    ostringstream out;
    out << "--- Verso 1 ---\n" << generar_verso_poetico(d) << "\n" << generar_verso_simple(d) << "\n\n";
    out << "--- Estribillo ---\n" << generar_estribillo(d) << "\n";
    out << "--- Verso 2 ---\n" << generar_verso_simple(d) << "\n" << generar_verso_poetico(d) << "\n\n";
    out << "--- Estribillo ---\n" << generar_estribillo(d) << "\n";
    return out.str();
}

// -------------------- Song representation --------------------
struct Song {
    string title;
    string text; // full song text

    // sparse word counts using ordered map (word -> count)
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

// -------------------- Binary Search Tree for Songs (keyed by title) --------------------
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

    static unique_ptr<BSTNode>& find_node(unique_ptr<BSTNode>& node, const string& title) {
        return node; // helper not used
    }

    static unique_ptr<BSTNode> remove_rec(unique_ptr<BSTNode> node, const string& title, bool &removed) {
        if (!node) return nullptr;
        if (title < node->song.title) node->left = remove_rec(move(node->left), title, removed);
        else if (title > node->song.title) node->right = remove_rec(move(node->right), title, removed);
        else {
            removed = true;
            if (!node->left) return move(node->right);
            if (!node->right) return move(node->left);
            // two children: replace with inorder successor
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
                // replace
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

// -------------------- Vocabulary & Sparse vectors --------------------
// We maintain a global vocabulary (ordered map: word -> index)
struct Vocabulary {
    map<string,int> index_of; // ordered -- avoids hashes
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

// Convert song word_counts (map<string,int>) to a sparse vector of (index, value)
using SparseVec = vector<pair<int,double>>; // sorted by index

SparseVec build_sparse(const Song& s, Vocabulary& V) {
    SparseVec out;
    for (const auto &p : s.word_counts) {
        int idx = V.ensure(p.first);
        out.emplace_back(idx, (double)p.second);
    }
    sort(out.begin(), out.end(), [](auto &a, auto &b){ return a.first < b.first; });
    return out;
}

// Cosine similarity between two sparse vectors
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
            na += a[ia].second * a[ia].second; ++ia;
        } else { nb += b[ib].second * b[ib].second; ++ib; }
    }
    while (ia < a.size()) { na += a[ia].second * a[ia].second; ++ia; }
    while (ib < b.size()) { nb += b[ib].second * b[ib].second; ++ib; }
    if (na == 0.0 || nb == 0.0) return 0.0;
    return dot / (sqrt(na) * sqrt(nb));
}

// -------------------- Fraud detection --------------------
// Simple approach: compute pairwise similarity between newSong and existing songs.
// If similarity >= threshold -> flagged as potential fraud.
vector<pair<const Song*, double>> detect_similar(const Song& s, const SongBST& tree, Vocabulary& V, double threshold=0.6) {
    vector<pair<const Song*, double>> found;
    // build sparse vector for s without adding new vocabulary entries permanently:
    // To keep code simple we will allow vocabulary extension (it's fine), but note it.
    SparseVec sv = build_sparse(s, V);
    auto all = tree.all_songs_inorder();
    for (auto other : all) {
        if (other->title == s.title) continue;
        SparseVec ov = build_sparse(*other, V);
        double sim = cosine_similarity(sv, ov);
        if (sim >= threshold) found.emplace_back(other, sim);
    }
    sort(found.begin(), found.end(), [](auto &a, auto &b){ return a.second > b.second; });
    return found;
}

// -------------------- Utilities --------------------
string ask_line(const string& prompt) {
    cout << prompt;
    string s;
    getline(cin, s);
    return s;
}

// sanitize a title (trim)
string trim(const string& s) {
    size_t a = s.find_first_not_of(" \t\n\r");
    if (a == string::npos) return "";
    size_t b = s.find_last_not_of(" \t\n\r");
    return s.substr(a, b - a + 1);
}

// -------------------- Main interactive menu --------------------
int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    Diccionario d;
    d.load_sample();

    SongBST tree;
    Vocabulary vocab;

    while (true) {
        cout << "\n== Menu ==\n";
        cout << "1) Generar y almacenar cancion aleatoria\n";
        cout << "2) Crear cancion manualmente\"\n";
        cout << "3) Ver titulos (in-order)\n";
        cout << "4) Ver cancion por titulo\n";
        cout << "5) Analizar fraude (comparar una cancion contra almacenadas)\n";
        cout << "6) Borrar cancion por titulo\n";
        cout << "7) Salir\n";
        cout << "Elija una opcion: ";
        string opt; getline(cin, opt);
        if (opt == "1") {
            string text = generar_cancion_text(d);
            cout << "Ingrese titulo para esta cancion: ";
            string title; getline(cin, title); title = trim(title);
            if (title.empty()) title = "Cancion_" + to_string((int)chrono::high_resolution_clock::now().time_since_epoch().count());
            Song s(title, text);
            tree.insert(move(s));
            cout << "Cancion guardada.\n";
        } else if (opt == "2") {
            string title = ask_line("Titulo: "); title = trim(title);
            string text;
            cout << "Ingrese el texto de la cancion (una linea).\n";
            getline(cin, text);
            if (title.empty()) { cout << "Titulo invalido.\n"; continue; }
            Song s(title, text);
            tree.insert(move(s));
            cout << "Cancion guardada.\n";
        } else if (opt == "3") {
            cout << "Titulos almacenados (in-order):\n";
            tree.print_inorder();
        } else if (opt == "4") {
            string t = ask_line("Titulo a ver: "); t = trim(t);
            const Song* s = tree.find(t);
            if (!s) cout << "No encontrada.\n";
            else {
                cout << "--- " << s->title << " ---\n";
                cout << s->text << "\n";
                cout << "(palabras indexadas: " << s->word_counts.size() << ")\n";
            }
        } else if (opt == "5") {
            cout << "Analizar contra titulo existente o crear nueva? (e=existente / n=nueva): ";
            string t; getline(cin, t);
            Song probe;
            if (!t.empty() && (t[0]=='e' || t[0]=='E')) {
                string title = ask_line("Titulo a analizar: "); title = trim(title);
                const Song* s = tree.find(title);
                if (!s) { cout << "No encontrada.\n"; continue; }
                probe = *s; // copy
            } else {
                cout << "Ingrese o genere el texto de la cancion (una linea):\n";
                string text; getline(cin, text);
                cout << "Ingrese un titulo para esta prueba: "; string title; getline(cin, title); title = trim(title);
                if (title.empty()) title = "probe_" + to_string((int)chrono::high_resolution_clock::now().time_since_epoch().count());
                probe = Song(title, text);
            }
            double thresh = 0.6;
            cout << "Umbral de similitud (0.0 - 1.0) [por defecto 0.6]: "; string thr; getline(cin, thr);
            if (!thr.empty()) try { thresh = stod(thr); } catch(...) { thresh = 0.6; }
            auto found = detect_similar(probe, tree, vocab, thresh);
            if (found.empty()) cout << "No se encontraron canciones similares (umbral="<<thresh<<").\n";
            else {
                cout << "Posibles fraudes (similitud >= " << thr << "):\n";
                for (auto &p : found) {
                    cout << fixed << setprecision(3) << " - " << p.first->title << " (sim=" << p.second << ")\n";
                }
            }
        } else if (opt == "6") {
            string title = ask_line("Titulo a borrar: "); title = trim(title);
            if (tree.remove(title)) cout << "Borrado.\n"; else cout << "No encontrado.\n";
        } else if (opt == "7") break;
        else cout << "Opcion invalida.\n";
    }

    cout << "Adios.\n";
    return 0;
}

