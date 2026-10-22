#include <codecvt>
#include <iostream>
#include <iterator>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <string>
#include <stack>
#include <vector>

using namespace std;

int tmp_score[10];
int started = 0;

struct student_info {
    char gender;
    int class_num;
    int avg_score;
    int total_score;
    int rank;
    int score[10];
    int new_avg_score;
    int new_total_score;
    int new_score[10];
};


unordered_map<string, student_info> m;

struct student_cmp {
    bool operator()(const string &a, const string &b) const {
        auto it1 = m.find(a);
        auto it2 = m.find(b);
        if (it1 == m.end() || it2 == m.end()) {
            return a < b;
        }
        if ((it1->second.avg_score) != (it2->second.avg_score)) {
            return (it1->second.avg_score) > (it2->second.avg_score);
        }
        for (int i = 0; i < 9; ++i) {
            if ((it1->second.score[i]) != (it2->second.score[i])) {
                return (it1->second.score[i]) > (it2->second.score[i]);
            }
        }
        return a < b;
    }
};

set<string, student_cmp> s;
unordered_set<string> updated;


int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    string operates;
    while (true) {
        cin >> operates;
        if (operates == "ADD") {
            string name;
            student_info tmp_info;
            int total_score = 0;
            cin >> name >> tmp_info.gender >> tmp_info.class_num;
            for (int i = 0; i < 9; ++i) {
                cin >> tmp_info.score[i];
                tmp_info.new_score[i] = tmp_info.score[i];
                total_score += tmp_info.score[i];
            }
            tmp_info.total_score = total_score;
            tmp_info.new_total_score = total_score;
            tmp_info.avg_score = total_score / 9;
            tmp_info.new_avg_score = total_score / 9;
            if (started) {
                cout << "[Error]Cannot add student now.\n";
                continue;
            }
            auto it = m.find(name);
            if (it != m.end()) {
                cout << "[Error]Add failed.\n";
                continue;
            }
            m.insert({name, tmp_info});
            s.insert(name);
        } else if (operates == "START") {
            started = 1;
            if (!s.empty()) {
                int curr_rank = 1;
                auto it = s.begin();
                while (it != s.end()) {
                    m[*it].rank = curr_rank;
                    curr_rank++;
                    ++it;
                }
            }
        } else if (operates == "UPDATE") {
            string name;
            cin >> name;
            int code, score;
            cin >> code >> score;
            auto it = m.find(name);
            if (it == m.end()) {
                cout << "[Error]Update failed.\n";
                continue;
            }
            it->second.new_total_score =
                it->second.new_total_score + score - it->second.new_score[code];
            it->second.new_avg_score = it->second.new_total_score / 9;
            it->second.new_score[code] = score;
            updated.insert(name);
        } else if (operates == "FLUSH") {
            if (!m.empty()) {
                for (string name : updated) {
                    s.erase(name);
                    m[name].avg_score = m[name].new_avg_score;
                    m[name].total_score = m[name].new_total_score;
                    for (int i = 0; i < 9; ++i) {
                        m[name].score[i] = m[name].new_score[i];
                    }
                    s.insert(name);
                }
                updated.clear();
                int curr_rank = 1;
                auto it = s.begin();
                while (it != s.end()) {
                    m[*it].rank = curr_rank;
                    curr_rank++;
                    ++it;
                }
            }
        } else if (operates == "PRINTLIST") {
            int rank = 0;
            if (!s.empty()) {
                for (auto it = s.begin(); it != s.end(); ++it) {
                    ++rank;
                    cout << rank << ' ' << *it << ' ';
                    cout << (m[*it].gender == 'M' ? "male" : "female") << ' ';
                    cout << m[*it].class_num << ' ' << m[*it].new_avg_score
                         << '\n';
                }
            }
        } else if (operates == "QUERY") {
            string name;
            cin >> name;
            auto it = s.find(name);
            if (it == s.end()) {
                cout << "[Error]Query failed.\n";
                continue;
            }
            cout << "STUDENT " << name << " NOW AT RANKING " << m[name].rank
                 << '\n';
        } else if (operates == "END") {
            return 0;
        }
    }
}