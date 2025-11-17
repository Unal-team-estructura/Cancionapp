#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <sstream>
#include <random>
#include <chrono>

using namespace std;

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

struct Diccionario {
    unordered_map<string, vector<string>> cat;

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
            {"N", {"corazón","ciudad","mar","noche","camino"}},
            {"V", {"late","brilla","canta","corre","susurra"}},
            {"Adj", {"oscuro","silencioso","eterno","dulce"}},
            {"P", {"en","sobre","bajo","entre"}},
            {"S", {"mi","tu","nuestro"}},
            {"C", {"de la madrugada","sin final","de papel"}}
        };
    }
};

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
        line[0] = toupper((unsigned char)line[0]);
        out += line + ".\n";
    }
    return out;
}

string generar_cancion(Diccionario& d) {
    if (d.get("N").empty()) d.load_sample();
    ostringstream out;
    out << "--- Verso 1 ---\n" << generar_verso_poetico(d) << "\n" << generar_verso_simple(d) << "\n\n";
    out << "--- Estribillo ---\n" << generar_estribillo(d) << "\n";
    out << "--- Verso 2 ---\n" << generar_verso_simple(d) << "\n" << generar_verso_poetico(d) << "\n\n";
    out << "--- Estribillo ---\n" << generar_estribillo(d) << "\n";
    return out.str();
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    Diccionario d;
    d.load_sample();
    for (int i = 0; i < 3; ++i) {
        cout << "===== CANCION " << (i+1) << " =====\n";
        cout << generar_cancion(d) << "\n";
    }
    return 0;
}
