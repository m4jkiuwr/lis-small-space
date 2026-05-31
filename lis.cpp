#include <vector>
#include <optional>
#include <utility>
#include <limits>
#include <algorithm>
#include <span>

using namespace std;

const int INF = numeric_limits<int>::max();

struct Element {
    int index;
    optional<int> pred;
};

// Helper for finding the pile index using binary search
// returns the smallest index h such that A[top(P_h)] >= val
template<typename Iterator>
int find_pile_index(span<const int> A, Iterator begin, Iterator end, int val) {
    return (int)distance(begin, lower_bound(begin, end, val, [&](int idx, int target) {
        return A[idx] < target;
    }));
}

// Writes the LIS of A (for elements > lb and <= ub) to output
void classicLIS(span<const int> A, int lb, int ub, vector<int>& output);

// Compute the j for next ppass and indicates whether j is the lislen
pair<int, bool> Ppass_first(span<const int> A, int s, const vector<Element>& P_i, int lb, int ub);

// Compute the P_j or top(P_j) if is_last_pile
vector<Element> Ppass_second(span<const int> A, const vector<Element>& P_i, int j, bool is_last_pile, int lb, int ub);

// Computes the LisLen and a pred of an element from the last pile
pair<int, optional<int>> LisLen(span<const int> A, int s, int lb, int ub) {
    if (A.empty()) return {0, nullopt};

    vector<Element> current_pile;
    int total_len = 0;
    
    while (true) {
        auto [j, is_last] = Ppass_first(A, s, current_pile, lb, ub);
        current_pile = Ppass_second(A, current_pile, j, is_last, lb, ub);
        total_len += j;

        if (is_last) {
            if (current_pile.empty()) return {total_len, nullopt};
            return {total_len, current_pile.back().pred};
        }
        
        if (j == 0 && current_pile.empty() && !is_last) break; 
    }
    return {total_len, nullopt};
}

void Lis(span<const int> A, int s, int lb, int ub, vector<int>& output) {
    if (A.empty()) return;
    if (A.size() <= static_cast<size_t>(s)) {
        classicLIS(A, lb, ub, output);
        return;
    }
    auto [k, m] = LisLen(A, s, lb, ub);

    size_t mid = A.size() / 2;

    if (!m.has_value()) {
        Lis(A.subspan(mid), s, lb, ub, output);
    } 
    else {
        Lis(A.subspan(0, mid), s, lb, m.value(), output);        
        Lis(A.subspan(mid), s, m.value(), ub, output);
    }
}


void classicLIS(span<const int> A, int lb, int ub, vector<int>& output) {
    if (A.empty()) return;

    vector<int> P_tops;
    vector<int> pred(A.size(), -1);

    for (int t = 0; t < (int)A.size(); t++) {
        int a = A[t];
        if (a <= lb || a > ub) continue;

        if (P_tops.empty() || a > A[P_tops.back()]) {
            if (!P_tops.empty()) pred[t] = P_tops.back();
            P_tops.push_back(t);
        } else {
            int h = find_pile_index(A, P_tops.begin(), P_tops.end(), a);
            if (h > 0) pred[t] = P_tops[h - 1];
            P_tops[h] = t;
        }
    }

    if (P_tops.empty()) return;

    vector<int> res;
    for (int curr = P_tops.back(); curr != -1; curr = pred[curr]) {
        res.push_back(A[curr]);
    }
    reverse(res.begin(), res.end());
    output.insert(output.end(), res.begin(), res.end());
}

pair<int, bool> Ppass_first(span<const int> A, int s, const vector<Element>& P_i, int lb, int ub) {
    int h = 0; 
    int r = 0; 
    vector<int> p(2*s + 1, -1);
    vector<int> c(2*s + 1, 0);  

    for (int t = 0; t < (int)A.size(); ++t) {
        int a = A[t];
        if (a <= lb || a > ub) continue;

        if (h < (int)P_i.size() && t == P_i[h].index) {
            h++;
            continue;
        }

        int p_min_val = (h > 0) ? A[P_i[h - 1].index] : lb;
        int p_max_val = (r == 2*s) ? A[p[2*s]] : numeric_limits<int>::max();

        if (a <= p_min_val || a > p_max_val) continue;

        int top_r_val = (r == 0) ? p_min_val : A[p[r]];

        int k = 0;
        if (r < 2*s && a > top_r_val) {
            r++;
            k = r;
        } else {
            k = find_pile_index(A, p.begin() + 1, p.begin() + r + 1, a) + 1;
        }

        if (k >= 1 && k <= 2*s) {
            p[k] = t; 
            c[k]++;
        }
    }
    
    if (r < 2*s) {
        return {r, true};
    } else {
        for (int j = 2*s; j >= 1; --j) {
            if (c[j] > 0 && c[j] <= s) {
                return {j, false};
            }
        }
    }
    return {r, true};
}

vector<Element> Ppass_second(span<const int> A, const vector<Element>& P_i, int j, bool is_last_pile, int lb, int ub) {
    int h = 0; 
    int r = 0; 
    vector<int> p(j + 1, -1);
    vector<optional<int>> p_pred(j + 1, nullopt);
    vector<Element> P_target;
    int mid = (int)A.size() / 2;

    for (int t = 0; t < (int)A.size(); ++t) {
        int a = A[t];
        if (a <= lb || a > ub) continue;

        if (h < (int)P_i.size() && t == P_i[h].index) {
            h++;
            continue;
        }

        int p_min_val = (h == 0) ? lb : A[P_i[h - 1].index];
        if (a <= p_min_val) continue;

        if (r == j && a > A[p[r]]) continue;

        int top_r_val = (r == 0) ? p_min_val : A[p[r]];

        int k = 0;
        if (r < j && a > top_r_val) {
            r++;
            k = r;
        } else {
            k = find_pile_index(A, p.begin() + 1, p.begin() + r + 1, a) + 1;
        }

        if (k >= 1 && k <= j) {
            optional<int> current_pred;
            if (t < mid) {
                current_pred = a;
            } else {
                if (k == 1) {
                    current_pred = (h == 0) ? nullopt : P_i[h - 1].pred;
                } else {
                    current_pred = p_pred[k - 1];
                }
            }

            p[k] = t;
            p_pred[k] = current_pred;

            if (k == j) {
                if (is_last_pile) {
                    if (P_target.empty()) P_target.push_back({t, current_pred});
                    else P_target[0] = {t, current_pred};
                } else {
                    P_target.push_back({t, current_pred});
                }
            }
        }
    }
    return P_target;
}
