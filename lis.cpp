#include <span>
#include <vector>
#include <optional>
#include <utility>

using namespace std;

// Writes the LIS of A (for elements > lb and <= ub) to output
void classicLIS(span<const int> A, int lb, int ub, vector<int>& output);

// Computes the LisLen and a pred of an element from the last pile
pair<int, optional<int>> LisLen(span<const int> A, int s, int lb, int ub);

// Compute the j for next ppass and indicates whether j is the lislen
pair<int, bool> Ppass_first(span<const int> A, int s, const vector<int>& P_i, int lb, int ub);

// Compute the P_j or top(P_j) if is_last_pile
vector<int> Ppass_second(span<const int> A, const vector<int>& P_i, int j, bool is_last_pile, int lb, int ub);

void Lis(span<const int> A, int s, int lb, int ub, vector<int>& output) {

    if (A.size() <= static_cast<size_t>(s)) {
        classicLIS(A, lb, ub, output);
        return;
    }
    auto [k, m] = LisLen(A, s, lb, ub);

    size_t mid = A.size() / 2;

    // If m has no value we conclude that L_{<= m} is empty in the left half
    if (!m.has_value()) {
        Lis(A.subspan(mid), s, lb, ub, output);
    } 
    else {
        Lis(A.subspan(0, mid), s, lb, m.value(), output);        
        Lis(A.subspan(mid), s, m.value(), ub, output);
    }
}