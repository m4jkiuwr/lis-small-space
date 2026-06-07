#include <iostream>
#include <vector>
#include <string>
#include <new>
#include <cstdlib>
#include <algorithm>
#include <fstream>
#include <chrono>
#include "lis.cpp"

using namespace std;
using namespace std::chrono;

size_t current_memory = 0;
size_t peak_memory = 0;

void* operator new(size_t size) {
    current_memory += size;
    peak_memory = max(peak_memory, current_memory);
    void* p = malloc(size);
    if (!p) throw bad_alloc();
    return p;
}

void operator delete(void* p, size_t size) noexcept {
    current_memory -= size;
    free(p);
}

void operator delete(void* p) noexcept {
    free(p);
}

bool is_increasing(const vector<int>& seq) {
    for (size_t i = 1; i < seq.size(); ++i) {
        if (seq[i] <= seq[i - 1]) return false;
    }
    return true;
}

bool is_subsequence(const vector<int>& sub, const vector<int>& arr) {
    size_t i = 0, j = 0;
    while (i < sub.size() && j < arr.size()) {
        if (sub[i] == arr[j]) i++;
        j++;
    }
    return i == sub.size();
}

int lislen(const vector<int>& a) {
    vector<int> tails;
    for (int x : a) {
        auto it = lower_bound(tails.begin(), tails.end(), x);
        if (it == tails.end()) tails.push_back(x);
        else *it = x;
    }
    return tails.size();
}

int main(int argc, char* argv[]) {

    string in_path = argv[1];
    ifstream fin(in_path);

    int n, s;
    if (!(fin >> n >> s)) return 0;
    
    vector<int> A(n);
    for (int i = 0; i < n; ++i) {
        fin >> A[i];
    }

    int expected_len = lislen(A);

    // ----------------------------------------------------
    // Benchmark Classic
    // ----------------------------------------------------
    vector<int> output_classic;
    output_classic.reserve(n);
    size_t baseline_c = current_memory;
    peak_memory = current_memory;
    
    auto start_c = high_resolution_clock::now();
    classicLIS(A, -INF, INF, output_classic);
    auto end_c = high_resolution_clock::now();
    double time_c = duration<double>(end_c - start_c).count();
    
    size_t overhead_c = peak_memory - baseline_c;
    double mem_c_mb = (double)overhead_c / (1024.0 * 1024.0);

    bool valid_c = ((int)output_classic.size() == expected_len && 
                    is_increasing(output_classic) && 
                    is_subsequence(output_classic, A));

    // ----------------------------------------------------
    // Benchmark Param
    // ----------------------------------------------------
    vector<int> output_param;
    output_param.reserve(n);
    size_t baseline_p = current_memory;
    peak_memory = current_memory;
    
    auto start_p = high_resolution_clock::now();
    Lis(A, s, -INF, INF, output_param);
    auto end_p = high_resolution_clock::now();
    double time_p = duration<double>(end_p - start_p).count();
    
    size_t overhead_p = peak_memory - baseline_p;
    double mem_p_mb = (double)overhead_p / (1024.0 * 1024.0);

    bool valid_p = ((int)output_param.size() == expected_len && 
                    is_increasing(output_param) && 
                    is_subsequence(output_param, A));

    bool valid = valid_c && valid_p;

    cout << n << "," << s << "," 
        << time_c << "," << mem_c_mb << "," 
        << time_p << "," << mem_p_mb << "," 
        << (valid ? "True" : "False") << endl;

    return 0;
}