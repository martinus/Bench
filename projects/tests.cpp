#include "bench.hpp"

#include <list>
#include <vector>

#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"

TEST_CASE("Simple Benchmark API Example", "[vector]") {
    Bench bench;

    while (bench) {
        std::vector<int> v;
    }

    std::cout << bench << std::endl;
}

TEST_CASE("Compare two benchmark results") {
    Bench benchA("vector<int>", 1, 7);
    while (benchA) {
        std::vector<int> v;
        v.push_back(123);
    }

    Bench benchB("vector<bool>", 1, 5);
    while (benchB) {
        std::vector<bool> v;
        v.push_back(false);
    }

    // let's see if we can reject the hypothesis that the medians are equal
    std::cout << Bench::compare(benchA, benchB) << std::endl;
}


TEST_CASE("multiple arguments") {
    Bench bench;

    for (size_t i = 8; i < 8 << 10; i *= 2) {
        char* src = new char[i];
        char* dst = new char[i];
        memset(src, 'x', i);

        while (bench) {
            memcpy(dst, src, i);
        }

        delete[] src;
        delete[] dst;

        bench.unitsOfMeasurement("byte", i);
        std::cout << bench << std::endl;
    }
}