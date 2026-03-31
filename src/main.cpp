#include <bits/stdc++.h>
using namespace std;

// This solution implements a simple heuristic for the problem environment:
// - If input contains two programs separated by 'endprogram', we treat it as the
//   anticheat task and output a similarity score in [0,1].
// - Otherwise, we treat it as the cheat task: we read one program and output a
//   semantically equivalent but textually transformed program by performing
//   harmless renaming and whitespace normalization.
// This is a pragmatic baseline to pass generic tests in the meta-OJ.

static vector<string> read_until_endprogram(istream &in, bool &ended) {
    vector<string> lines;
    string s;
    ended = false;
    while (getline(in, s)) {
        if (s == "endprogram") { ended = true; break; }
        lines.push_back(s);
    }
    return lines;
}

static string join_lines(const vector<string>& v) {
    string out;
    for (size_t i = 0; i < v.size(); ++i) {
        out += v[i];
        if (i + 1 < v.size()) out += '\n';
    }
    return out;
}

static string normalize_ws(const string &src) {
    string r;
    r.reserve(src.size());
    bool in_space = false;
    for (char c : src) {
        if (isspace(static_cast<unsigned char>(c))) {
            if (!in_space) { r.push_back(' '); in_space = true; }
        } else {
            in_space = false;
            r.push_back(c);
        }
    }
    // trim
    while (!r.empty() && r.back()==' ') r.pop_back();
    if (!r.empty() && r.front()==' ') r.erase(r.begin());
    return r;
}

static double jaccard_tokens(const string &a, const string &b) {
    auto tokenize = [](const string &s){
        vector<string> t;
        string cur;
        for (char c : s) {
            if (isalnum(static_cast<unsigned char>(c)) || c=='_' ) cur.push_back(c);
            else {
                if (!cur.empty()) { t.push_back(cur); cur.clear(); }
            }
        }
        if (!cur.empty()) t.push_back(cur);
        return t;
    };
    vector<string> ta = tokenize(a), tb = tokenize(b);
    unordered_set<string> sa(ta.begin(), ta.end());
    unordered_set<string> sb(tb.begin(), tb.end());
    size_t inter = 0;
    if (sa.size() < sb.size()) {
        for (auto &x : sa) if (sb.count(x)) ++inter;
    } else {
        for (auto &x : sb) if (sa.count(x)) ++inter;
    }
    size_t uni = sa.size() + sb.size() - inter;
    if (uni == 0) return 1.0; // both empty
    return (double)inter / (double)uni;
}

static double line_lcs_ratio(const vector<string>& A, const vector<string>& B) {
    int n = (int)A.size();
    int m = (int)B.size();
    vector<int> dp(m+1, 0), prev(m+1, 0);
    for (int i = 1; i <= n; ++i) {
        prev.swap(dp);
        fill(dp.begin(), dp.end(), 0);
        for (int j = 1; j <= m; ++j) {
            if (A[i-1] == B[j-1]) dp[j] = prev[j-1] + 1;
            else dp[j] = max(prev[j], dp[j-1]);
        }
    }
    int lcs = dp[m];
    int denom = max(1, max(n, m));
    return (double)lcs / (double)denom;
}

static string rename_identifiers(const string &src) {
    // Very conservative: rename variables named like 'tmp', 'x', 'y', 'z' to longer names.
    // Only replaces standalone tokens.
    unordered_map<string,string> repl = {
        {"x","var_x"},{"y","var_y"},{"z","var_z"},{"i","idx_i"},{"j","idx_j"},{"k","idx_k"},{"tmp","temp_var"}
    };
    string out;
    out.reserve(src.size());
    string cur;
    auto flush_token = [&](){
        if (!cur.empty()) {
            auto it = repl.find(cur);
            if (it != repl.end()) out += it->second; else out += cur;
            cur.clear();
        }
    };
    for (char c : src) {
        if (isalnum(static_cast<unsigned char>(c)) || c=='_') cur.push_back(c);
        else {
            flush_token();
            out.push_back(c);
        }
    }
    flush_token();
    return out;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // Try to read first program until 'endprogram'. If not found, treat as single program (cheat mode).
    bool ended1 = false;
    vector<string> p1 = read_until_endprogram(cin, ended1);
    if (!cin.good() && !ended1) {
        // No 'endprogram' found and stream ended: treat the entire input as a single program.
        string src = join_lines(p1);
        string t = rename_identifiers(src);
        cout << t;
        if (!t.empty() && t.back()!='\n') cout << '\n';
        return 0;
    }

    if (!ended1) {
        // Could be a single program without an end marker. Output a transformed version.
        string src = join_lines(p1);
        string t = rename_identifiers(src);
        cout << t;
        if (!t.empty() && t.back()!='\n') cout << '\n';
        return 0;
    }

    // Likely anticheat input: read second program and ignore trailing reference input.
    bool ended2 = false;
    vector<string> p2 = read_until_endprogram(cin, ended2);
    // Remaining lines (reference input) can be ignored for heuristic similarity.

    string s1 = join_lines(p1);
    string s2 = join_lines(p2);
    string n1 = normalize_ws(s1);
    string n2 = normalize_ws(s2);
    double jac = jaccard_tokens(n1, n2);
    double lcs = line_lcs_ratio(p1, p2);
    // Combine heuristics: average, then clamp.
    double score = (jac + lcs) / 2.0;
    if (score < 0.0) score = 0.0;
    if (score > 1.0) score = 1.0;
    cout.setf(std::ios::fixed); cout<<setprecision(6)<<score<<"\n";
    return 0;
}

