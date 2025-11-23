#include <iostream>
#include "Diccionario.hpp"
#include "Song.hpp"
#include "Util.hpp"

int main() {
    Diccionario d;
    d.load_sample();

    SongBST tree;
    Vocabulary vocab;

    while (true) {
        cout << "\n== Menu ==" << endl;
        cout << "1) Generar y almacenar cancion aleatoria" << endl;
        cout << "2) Ver titulos (in-order)" << endl;
        cout << "3) Ver cancion por titulo" << endl;
        cout << "4) Analizar fraude" << endl;
        cout << "5) Borrar cancion por titulo" << endl;
        cout << "6) Salir" << endl;
        cout << "Elija una opcion: ";

        string opt; getline(cin, opt);

        if (opt == "1") {
            string text = generar_cancion_text(d);
            string title = ask_line("Ingrese titulo: ");
            title = trim(title);
            if (title.empty())
                title = "Cancion_" + to_string((int)chrono::high_resolution_clock::now().time_since_epoch().count());

            Song s(title, text);
            tree.insert(move(s));
            cout << "Cancion guardada." << endl;

        } else if (opt == "2") {
            tree.print_inorder();

        } else if (opt == "3") {
            string t = ask_line("Titulo a ver: "); 
            t = trim(t);
            const Song* s = tree.find(t);
            if (!s) cout << "No encontrada." << endl;
            else {
                cout << "--- " << s->title << " ---\n";
                cout << s->text << "\n";
                cout << "(palabras indexadas: " << s->word_counts.size() << ")\n";
            }

        } else if (opt == "4") {
            cout << "Analizar contra titulo existente o crear nueva? (e/n): ";
            string t; getline(cin, t);

            Song probe;
            if (!t.empty() && (t[0]=='e' || t[0]=='E')) {
                string title = ask_line("Titulo: ");
                title = trim(title);
                const Song* s = tree.find(title);
                if (!s) { cout << "No encontrada.\n"; continue; }
                probe = *s;
            } else {
                cout << "Ingrese texto de la cancion:\n";
                string text; getline(cin, text);
                string title = ask_line("Titulo: ");
                title = trim(title);
                if (title.empty())
                    title = "probe_" + to_string((int)chrono::high_resolution_clock::now().time_since_epoch().count());
                probe = Song(title, text);
            }

            double thresh = 0.6;
            string thr = ask_line("Umbral [0.6 por defecto]: ");
            if (!thr.empty()) try { thresh = stod(thr); } catch(...) {}

            auto found = detect_similar(probe, tree, vocab, thresh);

            if (found.empty()) cout << "Sin coincidencias.\n";
            else {
                cout << "Coincidencias:\n";
                for (auto &p : found)
                    cout << " - " << p.first->title << " (sim=" << p.second << ")\n";
            }

        } else if (opt == "5") {
            string title = ask_line("Titulo: ");
            title = trim(title);
            if (tree.remove(title)) cout << "Borrado.\n";
            else cout << "No encontrado.\n";

        } else if (opt == "6") break;
        else cout << "Opcion invalida.\n";
    }

    cout << "Adios.\n";
}
