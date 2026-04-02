#include <iostream>
#include <fstream>
#include <vector>
#include <functional>
#include <chrono>
#include <string>
#include <set>
#include <iomanip>

#include "sorts.h"
#include "data_gen.h"


using SortFn  = std::function<void(std::vector<int>&)>;
using MemFn   = std::function<size_t(int)>;   // calculeaza memoria extra
using Clock   = std::chrono::high_resolution_clock;
using Seconds = std::chrono::duration<double>;

//  Masoara timpul pe o copie a datelor
//  Returneaza timpul in secunde
double measure_time(SortFn fn, const std::vector<int>& original) {
    std::vector<int> copy = original;
    auto t0 = Clock::now();
    fn(copy);
    auto t1 = Clock::now();
    return Seconds(t1 - t0).count();
}

//  Verifica daca algoritmul sorteaza corect
bool is_correct(SortFn fn, const std::vector<int>& original) {
    std::vector<int> copy = original;
    fn(copy);
    for (int i = 1; i < (int)copy.size(); i++)
        if (copy[i] < copy[i - 1]) return false;
    return true;
}

//  Formateaza bytes in mod lizibil (B / KB / MB)
std::string format_bytes(size_t bytes) {
    if (bytes < 1024)
        return std::to_string(bytes) + " B";
    if (bytes < 1024 * 1024)
        return std::to_string(bytes / 1024) + " KB";
    return std::to_string(bytes / (1024 * 1024)) + " MB";
}


int main() {

    //  1. Algoritmi  +  functiile lor de memorie
    struct AlgoEntry {
        std::string name;
        SortFn      fn;
        MemFn       mem_fn;  // calculeaza memoria extra in bytes
    };

    std::vector<AlgoEntry> algos = {
        { "BubbleSort",    bubble_sort,    bubble_sort_memory    },
        { "InsertionSort", insertion_sort, insertion_sort_memory },
        { "SelectionSort", selection_sort, selection_sort_memory },
        { "MergeSort",     merge_sort,     merge_sort_memory     },
        { "QuickSort",     quick_sort,     quick_sort_memory     },
    };

    // Algoritmi lenti, ne oprim la 10000
    std::set<std::string> slow    = { "BubbleSort", "InsertionSort", "SelectionSort" };
    const int             SLOW_MAX_N = 10000;   // crescut de la 5000

  
    //  2. Marimi de test si numarul de repetitii
    //     Extinse pana la 50.000.000 pentru MergeSort / QuickSort
    struct SizeConfig { long long n; int repeats; };
    std::vector<SizeConfig> sizes = {
        // Liste mici
        {        10,  5000 },
        {        20,  5000 },
        {        50,  2000 },
        {       100,  2000 },
        {       500,   500 },
        {      1000,   200 },
        {      5000,    50 },
        {     10000,    20 },
        // Liste medii
        {     50000,     5 },
        {    100000,     3 },
        {    500000,     2 },
        {   1000000,     1 },
        // Liste mari – doar pentru algoritmi rapizi (MergeSort, QuickSort)
        {   5000000,     1 },
        {  10000000,     1 },
        {  50000000,     1 },
    };

    //  3. Verificare corectitudine
    std::cout << "=== Verificare corectitudine (n=500, date random) ===\n";
    auto test_data = gen_random(500);
    for (auto& a : algos) {
        bool ok = is_correct(a.fn, test_data);
        std::cout << "  " << std::setw(15) << std::left << a.name
                  << (ok ? "OK" : "FAIL") << "\n";
    }
    std::cout << "\n";

    //  4. Afisam memoria extra teoretica pentru referinta
    std::cout << "=== Memorie extra teoretica (exemplu n=1.000.000) ===\n";
    const int demo_n = 1000000;
    for (auto& a : algos) {
        size_t mem = a.mem_fn(demo_n);
        std::cout << "  " << std::setw(15) << std::left << a.name
                  << format_bytes(mem) << "\n";
    }
    std::cout << "\n";

    //  5. Experiment
    auto data_configs = all_data_configs();

    // Numaram totalul de teste pentru progress
    int total = 0;
    for (auto& sc : sizes)
        for (auto& dc : data_configs)
            for (auto& a : algos)
                if (!(sc.n > SLOW_MAX_N && slow.count(a.name)))
                    total++;

    // CSV cu doua fisiere:
    //   rezultate_timp.csv   – timpii de rulare
    //   rezultate_memorie.csv – memoria extra
    std::ofstream csv_time("rezultate_timp.csv");
    std::ofstream csv_mem ("rezultate_memorie.csv");

    csv_time << "Algoritm,N,Tip_Date,Secunde_Medie\n";
    csv_time << std::fixed << std::setprecision(9);

    csv_mem  << "Algoritm,N,Tip_Date,Memorie_Extra_Bytes,Memorie_Extra_Lizibil\n";

    int done = 0;
    for (auto& sc : sizes) {
        int n = (int)sc.n;  

        for (auto& dc : data_configs) {
            for (auto& a : algos) {

                if (sc.n > SLOW_MAX_N && slow.count(a.name)) continue;

                //Timp 
                double total_time = 0.0;
                for (int r = 0; r < sc.repeats; r++) {
                    auto data = generate(dc.type, n);
                    total_time += measure_time(a.fn, data);
                }
                double avg_time = total_time / sc.repeats;

                // Memorie extra (calculata teoretic)
                size_t extra_mem = a.mem_fn(n);

                // Scriem in CSV-uri
                csv_time << a.name << "," << sc.n << "," << dc.name << ","
                         << avg_time << "\n";

                csv_mem  << a.name << "," << sc.n << "," << dc.name << ","
                         << extra_mem << "," << format_bytes(extra_mem) << "\n";

                done++;
                std::cout << "[" << std::setw(4) << done << "/" << total << "] "
                          << std::setw(15) << std::left << a.name
                          << " n=" << std::setw(10) << std::left << sc.n
                          << " " << std::setw(16) << std::left << dc.name
                          << std::fixed << std::setprecision(6) << avg_time << "s"
                          << "  mem_extra=" << format_bytes(extra_mem) << "\n";
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