#include <iostream>
#include "Diccionario.hpp"
#include "Song.hpp"
#include "Util.hpp"

string color(const string& t, int c) {
    return "\033[" + to_string(c) + "m" + t + "\033[0m";
}

void mostrar_menu() {
    cout << color("\n========================================\n", 36);
    cout << color("           GESTOR DE CANCIONES      \n", 35);
    cout << color("========================================\n", 36);

    cout << color(" [1] ", 33) << " Generar y almacenar cancion aleatoria\n";
    cout << color(" [2] ", 33) << " Ver titulos (in-order)\n";
    cout << color(" [3] ", 33) << " Ver cancion por titulo\n";
    cout << color(" [4] ", 33) << " Analizar fraude\n";
    cout << color(" [5] ", 33) << " Borrar cancion\n";
    cout << color(" [6] ", 33) << " Salir\n";

    cout << color("----------------------------------------\n", 36);
    cout << color(" Opcion: ", 32);
}

int main() {
    Diccionario d;
    d.load_sample();

    SongBST tree;
    Vocabulary vocab;

    while (true) {
        mostrar_menu();

        string opt; 
        getline(cin, opt);

        if (opt == "1") {
            cout << color("\n→ Generando cancion...\n", 34);
            string text = generar_cancion_text(d);

            string title = ask_line(color("Ingrese titulo: ", 33));
            title = trim(title);
            if (title.empty())
                title = "Cancion_" + to_string((int)chrono::high_resolution_clock::now().time_since_epoch().count());

            Song s(title, text);
            tree.insert(move(s));

            cout << color(" Cancion guardada.\n", 32);

        } else if (opt == "2") {
            cout << color("\n--- Titulos almacenados ---\n", 36);
            tree.print_inorder();

        } else if (opt == "3") {
            string t = ask_line(color("Titulo a ver: ", 33));
            t = trim(t);

            const Song* s = tree.find(t);
            if (!s) cout << color(" No encontrada.\n", 31);
            else {
                cout << color("\n--- " + s->title + " ---\n", 36);
                cout << s->text << "\n";
                cout << color("(Palabras indexadas: " + to_string(s->word_counts.size()) + ")\n", 34);
            }

        } else if (opt == "4") {
            cout << color("Analizar contra existente o crear nueva? (e/n): ", 33);
            string t; getline(cin, t);

            Song probe;

            if (!t.empty() && (t[0]=='e' || t[0]=='E')) {
                string title = ask_line(color("Titulo: ", 33));
                title = trim(title);

                const Song* s = tree.find(title);
                if (!s) { 
                    cout << color(" No encontrada.\n", 31); 
                    continue; 
                }
                probe = *s;

            } else {
                cout << color("Ingrese texto de la cancion:\n", 36);
                string text; getline(cin, text);

                string title = ask_line(color("Titulo: ", 33));
                title = trim(title);

                if (title.empty())
                    title = "probe_" + to_string((int)chrono::high_resolution_clock::now().time_since_epoch().count());

                probe = Song(title, text);
            }

            double thresh = 0.6;
            string thr = ask_line(color("Umbral [0.6 por defecto]: ", 33));
            if (!thr.empty()) try { thresh = stod(thr); } catch(...) {}

            auto found = detect_similar(probe, tree, vocab, thresh);

            if (found.empty()) 
                cout << color("Sin coincidencias.\n", 31);
            else {
                cout << color("Coincidencias:\n", 36);
                for (auto &p : found)
                    cout << color(" - ", 32) 
                         << p.first->title 
                         << " (sim=" << p.second << ")\n";
            }

        } else if (opt == "5") {
            string title = ask_line(color("Titulo: ", 33));
            title = trim(title);

            if (tree.remove(title)) 
                cout << color(" Borrado.\n", 32);
            else 
                cout << color(" No encontrado.\n", 31);

        } else if (opt == "6") {
            cout << color("\nSaliendo... ¡Hasta la proxima!\n", 35);
            break;

        } else {
            cout << color("Opcion invalida.\n", 31);
        }
    }

    return 0;
}
