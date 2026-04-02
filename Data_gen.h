#pragma once
#include <vector>
#include <numeric>
#include <algorithm>
#include <random>
#include <string>

static std::mt19937 rng(12345);

std::vector<int> gen_random(int n) {
    std::uniform_int_distribution<int> dist(0, 1000000);
    std::vector<int> arr(n);
    for (auto& x : arr) x = dist(rng);
    return arr;
}

std::vector<int> gen_sorted(int n) {
    std::vector<int> arr(n);
    std::iota(arr.begin(), arr.end(), 1);
    return arr;
}

std::vector<int> gen_reverse_sorted(int n) {
    auto arr = gen_sorted(n);
    std::reverse(arr.begin(), arr.end());
    return arr;
}

// nearly_sorted: ~5% din elemente sunt mutate
std::vector<int> gen_nearly_sorted(int n) {
    auto arr = gen_sorted(n);
    int swaps = std::max(1, (int)(n * 0.05));
    std::uniform_int_distribution<int> dist(0, n - 1);
    for (int i = 0; i < swaps; i++)
        std::swap(arr[dist(rng)], arr[dist(rng)]);
    return arr;
}

// flat: valori doar intre 1-10 (putine elemente distincte)
std::vector<int> gen_flat(int n) {
    std::uniform_int_distribution<int> dist(1, 10);
    std::vector<int> arr(n);
    for (auto& x : arr) x = dist(rng);
    return arr;
}

enum class DataType { RANDOM, SORTED, REVERSE_SORTED, NEARLY_SORTED, FLAT };

struct DataConfig {
    DataType    type;
    std::string name;
};

std::vector<DataConfig> all_data_configs() {
    return {
        { DataType::RANDOM,         "random"         },
        { DataType::SORTED,         "sorted"         },
        { DataType::REVERSE_SORTED, "reverse_sorted" },
        { DataType::NEARLY_SORTED,  "nearly_sorted"  },
        { DataType::FLAT,           "flat"           },
    };
}

std::vector<int> generate(DataType type, int n) {
    switch (type) {
        case DataType::RANDOM:         return gen_random(n);
        case DataType::SORTED:         return gen_sorted(n);
        case DataType::REVERSE_SORTED: return gen_reverse_sorted(n);
        case DataType::NEARLY_SORTED:  return gen_nearly_sorted(n);
        case DataType::FLAT:           return gen_flat(n);
    }
    return {};
}