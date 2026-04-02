#pragma once
#include <vector>
#include <algorithm>
#include <cstddef>

//  STRUCTURA pentru rezultatul unui sort
//  Contine timpul de rulare si memoria extra folosita (bytes)
struct SortResult {
    double time_seconds;
    size_t extra_memory_bytes;  // memorie alocata EXTRA fata de input
};

//  BUBBLE SORT  –  O(n^2) timp, O(1) memorie extra
void bubble_sort(std::vector<int>& arr) {
    int n = (int)arr.size();
    for (int i = 0; i < n; i++) {
        bool swapped = false;
        for (int j = 0; j < n - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                std::swap(arr[j], arr[j + 1]);
                swapped = true;
            }
        }
        if (!swapped) break;
    }
}
// Memorie extra: O(1)
inline size_t bubble_sort_memory(int /*n*/) {
    return sizeof(int) * 3;  // i, j, swapped
}

//  INSERTION SORT  –  O(n^2) worst, O(n) best, O(1) memorie
void insertion_sort(std::vector<int>& arr) {
    int n = (int)arr.size();
    for (int i = 1; i < n; i++) {
        int key = arr[i];
        int j = i - 1;
        while (j >= 0 && arr[j] > key) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
}
// Memorie extra: O(1)
inline size_t insertion_sort_memory(int /*n*/) {
    return sizeof(int) * 3;  // key, i, j
}

//  SELECTION SORT  –  O(n^2) mereu, O(1) memorie
void selection_sort(std::vector<int>& arr) {
    int n = (int)arr.size();
    for (int i = 0; i < n; i++) {
        int min_idx = i;
        for (int j = i + 1; j < n; j++)
            if (arr[j] < arr[min_idx]) min_idx = j;
        std::swap(arr[i], arr[min_idx]);
    }
}
// Memorie extra: O(1)
inline size_t selection_sort_memory(int /*n*/) {
    return sizeof(int) * 3;  // i, j, min_idx
}

//  MERGE SORT  –  O(n log n) mereu, O(n) memorie extra
//  Iterativ (fara recursivitate)
void merge_sort(std::vector<int>& arr) {
    int n = (int)arr.size();
    std::vector<int> tmp(n);  //asta e memoria extra: O(n)

    for (int width = 1; width < n; width *= 2) {
        for (int lo = 0; lo < n; lo += 2 * width) {
            int mid = std::min(lo + width, n);
            int hi  = std::min(lo + 2 * width, n);

            int i = lo, j = mid, k = lo;
            while (i < mid && j < hi)
                tmp[k++] = (arr[i] <= arr[j]) ? arr[i++] : arr[j++];
            while (i < mid) tmp[k++] = arr[i++];
            while (j < hi)  tmp[k++] = arr[j++];
        }
        std::swap(arr, tmp);
    }
}
// Memorie extra: O(n) – vectorul tmp de n elemente
inline size_t merge_sort_memory(int n) {
    return (size_t)n * sizeof(int);
}

//  QUICK SORT  –  O(n log n) average
//  Iterativ cu stiva proprie
//  Partitionare 3-way (Dutch National Flag) – bun pentru "flat"
static void partition3(std::vector<int>& arr, int lo, int hi,
                        int& out_lt, int& out_gt) {
    int mid_idx = lo + (hi - lo) / 2;
    if (arr[mid_idx] < arr[lo]) std::swap(arr[mid_idx], arr[lo]);
    if (arr[hi]      < arr[lo]) std::swap(arr[hi],      arr[lo]);
    if (arr[mid_idx] < arr[hi]) std::swap(arr[mid_idx], arr[hi]);
    int pivot = arr[hi];

    int lt = lo, gt = hi, i = lo;
    while (i <= gt) {
        if      (arr[i] < pivot) std::swap(arr[lt++], arr[i++]);
        else if (arr[i] > pivot) std::swap(arr[i],    arr[gt--]);
        else                     i++;
    }
    out_lt = lt;
    out_gt = gt;
}

void quick_sort(std::vector<int>& arr) {
    if ((int)arr.size() <= 1) return;
    // Stiva proprie de intervale [lo, hi]
    std::vector<std::pair<int,int>> stack;
    stack.reserve(64);  // evitam realocari pentru n mic
    stack.push_back({0, (int)arr.size() - 1});
    while (!stack.empty()) {
        auto [lo, hi] = stack.back();
        stack.pop_back();
        if (lo < hi) {
            int lt, gt;
            partition3(arr, lo, hi, lt, gt);
            stack.push_back({lo, lt - 1});
            stack.push_back({gt + 1, hi});
        }
    }
}
// Memorie extra: O(log n) in medie, adancimea stivei de recursivitate
// In cel mai rau caz O(n), dar cu 3-way si median-of-3 e practic O(log n)
// Adancimea medie = log2(n)
inline size_t quick_sort_memory(int n) {
    if (n <= 1) return 0;
    int depth = 0;
    int tmp = n;
    while (tmp > 1) { tmp >>= 1; depth++; }  // log2(n)
    return (size_t)depth * 2 * sizeof(int);   // 2 int-uri per nivel
}