#include <iostream>
#include <vector>
#include "lis.cpp"



int main() {
    int n, s;
    if (!(cin >> n >> s)) return 0;
    vector<int> A(n);
    for (int i = 0; i < n; ++i) {
        cin >> A[i];
    }

    vector<int> output;
    Lis(A, s, -INF, INF, output);

    for (int i = 0; i < (int)output.size(); ++i) {
        cout << output[i] << (i == (int)output.size() - 1 ? "" : " ");
    }
    cout << endl;

    return 0;
}
