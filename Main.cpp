/*
 * main.cpp  –  Experiment algoritmi de sortare
 *
 * Citeste datele de test din fisierele generate de generate_test_data.py
 * in loc sa le genereze la runtime.
 *
 * FORMAT FISIER (generat de Python):
 *   Linia 1: M N        (M = numar de instante, N = dimensiunea listei)
 *   Linia 2+: cate o lista pe linie (N numere separate prin spatiu)
 *             sau cate un numar pe linie (pentru N >= 1_000_000)
 *   In ambele cazuri, fscanf("%d") functioneaza identic.
 *
 * STRUCTURA ASTEPTATA PE DISK:
 *   test_data/
 *       random/10.txt  random/100.txt  ...
 *       sorted/10.txt  ...
 *       reverse_sorted/ nearly_sorted/ flat/
 *
 * COMPILARE:
 *   g++ -O2 -std=c++17 -o experiment main.cpp
 *
 * UTILIZARE:
 *   ./experiment
 *   (directorul test_data/ trebuie sa fie in root-ul proiectului)
 *
 * OUTPUT:
 *   rezultate_timp.csv
 *   rezultate_memorie.csv
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <functional>
#include <chrono>
#include <string>
#include <set>
#include <iomanip>
#include <filesystem>
#include <cstdio>
#include <algorithm>

#include "sorts.h"
// data_gen.h nu mai este inclus — datele vin din fisiere

namespace fs = std::filesystem;

using SortFn  = std::function<void(std::vector<int>&)>;
using MemFn   = std::function<size_t(int)>;
using Clock   = std::chrono::high_resolution_clock;
using Seconds = std::chrono::duration<double>;

// ─── Utilitare ────────────────────────────────────────────────────────────────

double measure_time(SortFn fn, const std::vector<int>& original) {
    std::vector<int> copy = original;
    auto t0 = Clock::now();
    fn(copy);
    auto t1 = Clock::now();
    return Seconds(t1 - t0).count();
}

bool is_sorted_asc(const std::vector<int>& v) {
    for (int i = 1; i < (int)v.size(); i++)
        if (v[i] < v[i-1]) return false;
    return true;
}

bool is_correct(SortFn fn, const std::vector<int>& original) {
    std::vector<int> copy = original;
    fn(copy);
    return is_sorted_asc(copy);
}

std::string format_bytes(size_t bytes) {
    if (bytes < 1024)           return std::to_string(bytes) + " B";
    if (bytes < 1024 * 1024)    return std::to_string(bytes / 1024) + " KB";
    return std::to_string(bytes / (1024 * 1024)) + " MB";
}

// ─── Citire fisier de test ────────────────────────────────────────────────────

struct TestFile {
    int M;   // numar de instante
    int N;   // dimensiunea fiecarei liste
    std::FILE* f = nullptr;

    // Deschide fisierul si citeste header-ul (M N)
    // Returneaza false daca fisierul nu exista sau are format gresit
    bool open(const std::string& path) {
        f = std::fopen(path.c_str(), "r");
        if (!f) return false;
        if (std::fscanf(f, "%d %d", &M, &N) != 2) {
            std::fclose(f); f = nullptr; return false;
        }
        return true;
    }

    // Citeste urmatoarea instanta din fisier
    // Returneaza vectorul cu N elemente
    // Functioneaza identic indiferent ca numerele sunt pe o linie sau pe linii separate
    std::vector<int> read_instance() {
        std::vector<int> arr(N);
        for (int j = 0; j < N; j++)
            std::fscanf(f, "%d", &arr[j]);
        return arr;
    }

    void close() { if (f) { std::fclose(f); f = nullptr; } }
    ~TestFile() { close(); }
};

// ─── Main ─────────────────────────────────────────────────────────────────────

int main() {

    // Calea catre directorul test_data
    fs::path data_root = fs::current_path() / "test_data";

    if (!fs::exists(data_root)) {
        std::cerr << "Eroare: directorul '" << data_root
                  << "' nu exista.\n"
                  << "Ruleaza mai intai generate_test_data.py\n";
        return 1;
    }

    // ── 1. Algoritmi ──────────────────────────────────────────────────────────

    struct AlgoEntry {
        std::string name;
        SortFn      fn;
        MemFn       mem_fn;
    };

    std::vector<AlgoEntry> algos = {
        { "BubbleSort",    bubble_sort,    bubble_sort_memory    },
        { "InsertionSort", insertion_sort, insertion_sort_memory },
        { "SelectionSort", selection_sort, selection_sort_memory },
        { "MergeSort",     merge_sort,     merge_sort_memory     },
        { "QuickSort",     quick_sort,     quick_sort_memory     },
    };

    // Algoritmii O(n^2) sunt sariti pentru liste mari
    std::set<std::string> slow_algos = { "BubbleSort", "InsertionSort", "SelectionSort" };
    const int SLOW_MAX_N = 10000;

    // ── 2. Criterii (corespund exact subdirectoarelor Python) ─────────────────

    std::vector<std::string> criteria = {
        "random", "sorted", "reverse_sorted", "nearly_sorted", "flat"
    };

    // ── 3. Verificare corectitudine (pe primul fisier random disponibil) ──────

    std::cout << "=== Verificare corectitudine ===\n";
    {
        fs::path check_path = data_root / "random" / "500.txt";
        TestFile tf;
        if (tf.open(check_path.string())) {
            auto sample = tf.read_instance();
            for (auto& a : algos) {
                bool ok = is_correct(a.fn, sample);
                std::cout << "  " << std::setw(15) << std::left << a.name
                          << (ok ? "OK" : "FAIL") << "\n";
            }
            tf.close();
        } else {
            std::cout << "  (fisierul de verificare " << check_path
                      << " nu exista, sarim verificarea)\n";
        }
    }

    // ── 4. Memorie teoretica pentru referinta ─────────────────────────────────

    std::cout << "\n=== Memorie extra teoretica (n=1.000.000) ===\n";
    for (auto& a : algos)
        std::cout << "  " << std::setw(15) << std::left << a.name
                  << format_bytes(a.mem_fn(1000000)) << "\n";

    // ── 5. Experiment ─────────────────────────────────────────────────────────

    std::ofstream csv_time("rezultate_timp.csv");
    std::ofstream csv_mem ("rezultate_memorie.csv");

    csv_time << "Algoritm,Criteriu,N,Instante,Secunde_Medie\n";
    csv_time << std::fixed << std::setprecision(9);
    csv_mem  << "Algoritm,Criteriu,N,Memorie_Extra_Bytes,Memorie_Extra_Lizibil\n";

    std::cout << "\n=== Experiment ===\n";

    for (const auto& criterion : criteria) {
        fs::path criterion_dir = data_root / criterion;
        if (!fs::exists(criterion_dir)) {
            std::cout << "[SKIP] Directorul '" << criterion_dir << "' lipseste.\n";
            continue;
        }

        std::cout << "\n[" << criterion << "]\n";

        // Colectam fisierele .txt si le sortam dupa N (numeric, nu lexicografic)
        std::vector<fs::path> files;
        for (const auto& entry : fs::directory_iterator(criterion_dir))
            if (entry.path().extension() == ".txt")
                files.push_back(entry.path());

        std::sort(files.begin(), files.end(), [](const fs::path& a, const fs::path& b) {
            return std::stoll(a.stem().string()) < std::stoll(b.stem().string());
        });

        for (const auto& fpath : files) {

            TestFile tf;
            if (!tf.open(fpath.string())) {
                std::cerr << "  Eroare la deschiderea: " << fpath << "\n";
                continue;
            }

            int M = tf.M, N = tf.N;

            // Citim toate instantele din fisier in memorie
            // (evitam sa redeschide fisierul de M ori)
            std::vector<std::vector<int>> instances;
            instances.reserve(M);
            for (int i = 0; i < M; i++)
                instances.push_back(tf.read_instance());
            tf.close();

            for (auto& a : algos) {

                bool is_slow = slow_algos.count(a.name) > 0;
                if (is_slow && N > SLOW_MAX_N) {
                    // Nu scriem "skipped" in CSV — raportul ramane curat
                    std::cout << "  [SKIP] " << std::setw(15) << std::left << a.name
                              << " n=" << N << " (O(n^2) pe lista mare)\n";
                    continue;
                }

                double total_time = 0.0;
                for (int i = 0; i < M; i++)
                    total_time += measure_time(a.fn, instances[i]);
                double avg_time = total_time / M;

                size_t extra_mem = a.mem_fn(N);

                csv_time << a.name << "," << criterion << "," << N << ","
                         << M << "," << avg_time << "\n";

                csv_mem  << a.name << "," << criterion << "," << N << ","
                         << extra_mem << "," << format_bytes(extra_mem) << "\n";

                std::cout << "  " << std::setw(15) << std::left << a.name
                          << " n=" << std::setw(10) << std::left << N
                          << " m=" << std::setw(6)  << std::left << M
                          << std::fixed << std::setprecision(6) << avg_time << "s"
                          << "  mem=" << format_bytes(extra_mem) << "\n";
            }
        }
    }

    csv_time.close();
    csv_mem.close();

    std::cout << "\n>>> Gata!\n";
    std::cout << "    Timpi   -> rezultate_timp.csv\n";
    std::cout << "    Memorie -> rezultate_memorie.csv\n";

    return 0;
}