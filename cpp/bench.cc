// Taken from:
// https://github.com/adrian17/cpp-drop-merge-sort/blob/master/benchmark.cpp
#include <vector>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <random>

#include "mergesort.h"
#include "timsort.h"

using namespace std::chrono;

static float rng_uniform(float a, float b)
{
    static std::mt19937 gen(1);
    return std::uniform_real_distribution<float>(a, b)(gen);
}

template<typename T>
std::vector<T> randomize(float factor) {
    int max = 1000000;
    std::vector<T> ret(max);
    for (int i = 0; i < max; ++i) {
        if (rng_uniform(0.0, 1.0) < factor)
            ret[i] = rng_uniform(0, max-1);
        else
            ret[i] = i;
    }
    return ret;
}
template<>
std::vector<std::string> randomize<std::string>(float factor) {
    int max = 100000;
    std::vector<std::string> ret(max);
    for (int i = 0; i < max; ++i) {
        std::string s;
        if (rng_uniform(0.0, 1.0) < factor)
            s = std::to_string(rng_uniform(0, max-1));
        else
            s = std::to_string(i);
        ret[i] = std::string(100 - s.size(), '0') + s;
    }
    return ret;
}

template<typename T, typename Sort>
float measure(float factor, Sort sort) {
    size_t total = 0;
    constexpr size_t sz = 5;
    for (int i = 0; i < (int)sz; ++i) {
        auto tab = randomize<T>(factor);
        //auto copy = tab;
        auto t1 = high_resolution_clock::now();
        sort(tab);
        auto t2 = high_resolution_clock::now();
        size_t time = duration_cast<microseconds>(t2-t1).count();
        total += time;

        // sanity check
        //std::sort(copy.begin(), copy.end());
        //if(copy != tab)
        //    throw "asdf";
    }
    return total / (1000.0 * sz);
}

template<typename T>
void benchmark() {
    for (int i = 0; i <= 100; ++i) {
        float factor = i * 0.01;
        auto dt1 = measure<T>(factor, [](auto &tab){std::stable_sort(std::begin(tab), std::end(tab));});
        auto dt2 = measure<T>(factor, [](auto &tab){mergesort(std::begin(tab), std::end(tab));});
        auto dt3 = measure<T>(factor, [](auto &tab){gfx::timsort(std::begin(tab), std::end(tab));});
        std::cout << (factor) << "\t" << dt1 << "\t" << dt2 << "\t" << dt3 << "\n";
    }
    std::cout << "\n";
}

int main(){
    benchmark<int>();
    benchmark<std::string>();
}
