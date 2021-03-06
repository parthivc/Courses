#include <algorithm>
#include <iostream>
#include <vector>
#include <cstdio>
#include <functional>
#include <limits>
#include <valarray>

const long double EPS = std::numeric_limits<long double>::epsilon();

using namespace std;

using Row = vector<long double>;
using Matrix = vector<Row>;

struct Position {
    short row;
    short column;
    bool optimal() const { return row == -1 || column == -1; }
};

struct simplex {
    Position findPivot(Row& row) {
        int x = 0, y = distance(row.begin(), min_element(row.begin(), row.end()));
        long double ratio = numeric_limits<long double>::max() - 1;
        if (row[y] >= 0.0) {
            return {-1, -1};
        }
        for (int x2 = 0; x2 < A.size(); ++x2) {
            long double newRatio = b[x2] / A[x2][y];
            if (A[x2][y] && newRatio - ratio < EPS && newRatio > 0.0) {
                ratio = newRatio;
                x = x2;
            }
        }
        if (ratio == numeric_limits<long double>::max() - 1) {
            unbounded = true;
        }
        return {static_cast<short>(x), static_cast<short>(y)};
    }

    void processPivot(Position p, Row& row, const bool phase) {
        long double scalar = A[p.row][p.column];
        b[p.row] /= scalar;
        for (long double &val : A[p.row]) {
            val /= scalar;
        }
        for (int x = 0, s = A.size(); x < s; ++x) {
            if (p.row != x && abs(A[x][p.column] - 0.0) >= EPS) {
                scalar = A[x][p.column];
                for (int y = 0; y < A[x].size(); ++y) {
                    A[x][y] -= A[p.row][y] * scalar;
                }
                b[x] -= b[p.row] * scalar;
            }
        }
        if (phase) {
            b[b.size() - 2] -= b[p.row] * c[p.column];
            b[b.size() - 1] -= b[p.row] * row[p.column];
            long double mScalar = row[p.column];
            long double cScalar = c[p.column];
            for (int y = 0; y < possibles.size(); ++y) {
                row[y] -= A[p.row][y] * mScalar;
                c[y] -= A[p.row][y] * cScalar;
            }
        }
        else {
            b[b.size() - 1] -= b[p.row] * c[p.column];
            long double cScalar = c[p.column];
            for (int y = 0; y < possibles.size(); ++y) {
                c[y] -= A[p.row][y] * cScalar;
            }
        }
    }

    void reducedRowEchelonForm(Row& obj, const bool phase) {
        Position p = findPivot(obj);
        while (!(p.optimal() || unbounded)) {
            possibleSolutions[p.column] = p.row;
            processPivot(p, obj, phase);
            p = findPivot(obj);
        }
    }

    pair<int, Row> solve() {
        unbounded = false;
        possibleSolutions = vector<int>(A[0].size(), -1);
        transform(c.begin(), c.end(), c.begin(), negate<double>());
        for (int x = 0; x < A.size(); ++x) {
            A[x][x + m] = 1;
        }
        if (b.end() != find_if(b.begin(), b.end(), [](long double val) {return val < 0.0;})) { // Check if values are negative
            possibles = Row(c.size());
            for (int x = 0, y = m + n; x < b.size() - 1; ++x, ++y) {
                if (b[x] < 0.0) {
                    possibleSolutions[x] = -2;
                    A[x][y] = -1;
                    b.back() += b[x];
                    b[x] *= -1;
                    transform(A[x].begin(), A[x].end(), A[x].begin(), negate<long double>());
                    for (int y2 = 0; y2 < n + m; ++y2) {
                        possibles[y2] += A[x][y2];
                    }
                }
            }
            transform(possibles.begin(), possibles.end(), possibles.begin(), negate<long double>());
            reducedRowEchelonForm(possibles, true);
            if (abs(b.back() - 0.0) >= 0.001) {  // Infeasible
                return {-1, {}};
            }
        }
        c.resize(c.size() - n);
        b.pop_back();
        for (Row &r : A) {
            r.resize(r.size() - n);
        }
        reducedRowEchelonForm(c, false);
        if (unbounded) {
            return {1, {}};
        }
        Row result(m);
        for (int y = 0; y < m; ++y) {
            long double sum = 0.0;
            int index = 0;
            for (int x = 0; x < A.size(); ++x) {
                if (possibleSolutions[x] >= 0.0) {
                    sum += fabs(A[x][y]);
                }
                if (abs(A[x][y] - 1.0) < 0.001) {
                    index = x;
                }
            }
            result[y] = (sum - 1.0 > EPS) ? 0.0 : b[index];
        }
        return {0, move(result)};
    }

    int n, m;
    Matrix A;
    Row b, c, possibles;
    vector<int> possibleSolutions;
    bool unbounded;
};

int main() {
    ios_base::sync_with_stdio(false);
    int n, m, x;
    cin >> n >> m;
    Matrix A(n, Row (2 * n + m, 0.0));
    for (x = 0; x < n; ++x) {
        for (int y = 0; y < m; ++y) {
            cin >> A[x][y];
        }
    }
    Row b(n + 2);
    for (x = 0; x < n; ++x) {
        cin >> b[x];
    }
    Row c(2 * n + m);
    for (x = 0; x < m; ++x) {
        cin >> c[x];
    }

    simplex instance{n, m, move(A), move(b), move(c)};
    pair<int, Row> result = instance.solve();

    switch (result.first) {
        case -1:
            printf("No solution\n");
            break;
        case 0:
            printf("Bounded solution\n");
            for (int i = 0; i < m; i++) {
                printf("%.18Lf%c", result.second[i], " \n"[i + 1 == m]);
            }
            break;
        case 1:
            printf("Infinity\n");
            break;
    }
    return 0;
}

//#include <algorithm>
//#include <iostream>
//#include <vector>
//#include <cstdio>
//#include <functional>
//#include <limits>
//#include <functional>
//#include <valarray>
//#include <iomanip>
//
//const long double EPS = std::numeric_limits<long double>::epsilon();
//
//enum class state { optimal,
//    infeasible,
//    unbounded };
//enum class method_phase { one,
//    two };
//
//using namespace std;
//
//using matrix = vector<vector<long double> >;
//
//using column = std::vector<long double>;
//using row = std::vector<long double>;
//
//struct position {
//    short row;
//    short column;
//    bool is_optimal() { return row == -1 || column == -1; }
//};
//
//struct simplex_method {
//    void handle_artficial_vars()
//    {
//        for (std::size_t i = 0, j = m + n; i < b.size() - 1; ++i, ++j) {
//            if (b[i] < 0.0) {
//
//                solusion_vars[i] = -2;
//                A[i][j] = -1;
//
//                b.back() += b[i];
//                b[i] = -b[i];
//
//                std::transform(A[i].begin(), A[i].end(), A[i].begin(), std::negate<long double>());
//
//                for (int k = 0; k < n + m; ++k) {
//                    w[k] += A[i][k];
//                }
//            }
//        }
//
//        std::transform(w.begin(), w.end(), w.begin(), std::negate<long double>());
//    }
//
//    void handle_slack_vars()
//    {
//        for (std::size_t i = 0; i < A.size(); ++i) {
//            A[i][i + m] = 1;
//        }
//    }
//
//    // find pivots and run eliminations to get optimal objective
//    void Gauss_Jordam_eliminations(row& obj)
//    {
//        while (true) {
//            position p = find_pivot(obj);
//
//            if (p.is_optimal() || cur_solution == state::unbounded) {
//                break;
//            }
//
//            solusion_vars[p.column] = p.row;
//
//            scale_pivot(p);
//
//            process_pivot(p, obj);
//        }
//    }
//
//    void trim_table_from_avars()
//    {
//        c.resize(c.size() - n);
//        b.pop_back();
//
//        for (auto& r : A) {
//            r.resize(r.size() - n);
//        }
//    }
//
//    void phase_two()
//    {
//        phase = method_phase::two;
//        trim_table_from_avars();
//        Gauss_Jordam_eliminations(c);
//    }
//
//    void phase_one()
//    {
//        phase = method_phase::one;
//        Gauss_Jordam_eliminations(w);
//        cur_solution = double_equals_zero(b.back()) ? state::optimal : state::infeasible;
//    }
//
//    void prepare_table()
//    {
//        solusion_vars = std::vector<int>(A[0].size(), -1);
//        std::transform(c.begin(), c.end(), c.begin(), std::negate<double>());
//        handle_slack_vars();
//        if (mc) {
//            w = row(c.size());
//            handle_artficial_vars();
//        }
//    }
//
//    bool double_equals(double a, double b, double epsilon = 0.001)
//    {
//        return std::abs(a - b) < epsilon;
//    }
//
//    bool mixed_constraints() const
//    {
//        auto it = std::find_if(b.cbegin(), b.cend(), [](auto j) { return j < 0.0; });
//        return it == b.cend() ? false : true;
//    }
//
//    std::pair<int, vector<long double> > read_optimal_solution()
//    {
//        vector<long double> result(m);
//
//        for (int i = 0; i < m; ++i) {
//            long double sum = 0.0;
//            int k = 0;
//            for (std::size_t j = 0; j < A.size(); ++j) {
//                if (solusion_vars[j] >= 0.0)
//                    sum += fabs(A[j][i]);
//                if (double_equals(A[j][i], 1.0)) {
//                    k = j;
//                }
//            }
//
//            result[i] = (sum - 1.0 > EPS) ? 0.0 : b[k];
//        }
//
//        return { 0, std::move(result) };
//    }
//
//    std::pair<int, vector<long double> > solve()
//    {
//        mc = mixed_constraints();
//
//        prepare_table();
//
//        if (mc) {
//            phase_one();
//            if (cur_solution == state::infeasible) {
//                return { -1, {} };
//            }
//        }
//
//        phase_two();
//
//        if (cur_solution == state::unbounded) {
//            return { 1, {} };
//        }
//
//        return read_optimal_solution();
//    }
//
//    bool double_equals_zero(long double a, long double epsilon = 0.001)
//    {
//        return std::abs(a - 0.0) < epsilon;
//    }
//
//    position find_pivot(row& cw)
//    {
//        short i = 0, j = distance(cw.begin(), min_element(cw.begin(), cw.end()));
//        long double ratio = numeric_limits<long double>::max() - 1;
//
//        if (cw[j] >= 0.0) {
//            return { -1, -1 };
//        }
//
//        for (std::size_t k = 0; k < A.size(); ++k) {
//            long double r = b[k] / A[k][j];
//            if ((A[k][j] > 0.0 || A[k][j] < 0.0) && r - ratio < EPS && r > 0.0) {
//                ratio = r;
//                i = k;
//            }
//        }
//
//        if (ratio == numeric_limits<long double>::max() - 1) {
//            cur_solution = state::unbounded;
//        }
//
//        return { i, j };
//    }
//
//    void process_pivot(position p, row& w)
//    {
//        long double m{ 0.0 };
//
//        for (int i = 0, s = A.size(); i < s; ++i) {
//            if (p.row != i && !double_equals_zero(A[i][p.column], EPS)) {
//
//                m = A[i][p.column];
//
//                for (std::size_t j = 0; j < A[i].size(); ++j) {
//                    A[i][j] -= A[p.row][j] * m;
//                }
//
//                b[i] -= b[p.row] * m;
//            }
//        }
//
//        if (phase == method_phase::one) {
//            b[b.size() - 2] -= b[p.row] * c[p.column];
//            b[b.size() - 1] -= b[p.row] * w[p.column];
//            auto mw = w[p.column];
//            auto cw = c[p.column];
//            for (std::size_t i = 0; i < w.size(); ++i) {
//                w[i] -= A[p.row][i] * mw;
//                c[i] -= A[p.row][i] * cw;
//            }
//        }
//        else {
//            b[b.size() - 1] -= b[p.row] * c[p.column];
//            auto cw = c[p.column];
//            for (std::size_t i = 0; i < w.size(); ++i) {
//                c[i] -= A[p.row][i] * cw;
//            }
//        }
//    }
//
//    void scale_pivot(position p)
//    {
//        auto d = A[p.row][p.column];
//        b[p.row] /= d;
//        for (auto& n : A[p.row]) {
//            n /= d;
//        }
//    }
//
//    int n, m;
//    matrix A;
//    vector<long double> b, c, w;
//    vector<int> solusion_vars;
//    state cur_solution;
//    method_phase phase;
//    bool mc;
//};
//
//int main()
//{
//    std::ios_base::sync_with_stdio(false);
//
//    int n, m;
//    cin >> n >> m;
//
//    matrix A(n, vector<long double>(n + m + n, 0.0));
//    for (int i = 0; i < n; i++) {
//        for (int j = 0; j < m; j++) {
//            cin >> A[i][j];
//        }
//    }
//    vector<long double> b(n + 2);
//    for (int i = 0; i < n; i++) {
//        cin >> b[i];
//    }
//
//    vector<long double> c(n + m + n);
//    for (int i = 0; i < m; i++) {
//        cin >> c[i];
//    }
//
//    simplex_method sm{ n, m, std::move(A), std::move(b), std::move(c) };
//
//    pair<int, vector<long double> > ans = sm.solve();
//
//    switch (ans.first) {
//        case -1:
//            printf("No solution\n");
//            break;
//        case 0:
//            printf("Bounded solution\n");
//            for (int i = 0; i < m; i++) {
//                printf("%.18Lf%c", ans.second[i], " \n"[i + 1 == m]);
//            }
//            break;
//        case 1:
//            printf("Infinity\n");
//            break;
//    }
//    return 0;
//}
