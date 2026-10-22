#include <iostream>
#include <cstdio>
#include <cstring>
#include <iterator>
#include <map>
#include <vector>
#include <set>

using namespace std;

struct Activity {
    string str;
    int avg;
    int level;

    bool operator==(const Activity &other) const {
        return (str == other.str) && (avg == other.avg) &&
               (level == other.level);
    }
};

struct Activity_cmp {
    bool operator()(const Activity &a, const Activity &b) const {
        if (a.avg != b.avg) {
            return a.avg > b.avg;
        } else if (a.level != b.level) {
            return a.level < b.level;
        } else {
            return a.str < b.str;
        }
    }
};

std::map<std::string, std::vector<int>> m;
std::set<Activity, Activity_cmp> s;

int main() {
    string map_or_set;
    string operates;
    string name;
    while (true) {
        cin >> map_or_set;
        if (map_or_set == "map") {
            cin >> operates;
            if (operates == "insert") {
                cin >> name;
                vector<int> nbs;
                int num, arg, arg_flag = 1, ch_flag = 1;
                cin >> num;
                for (int i = 0; i < num; ++i) {
                    cin >> arg;
                    if (arg < 0) {
                        arg_flag = 0;
                    }
                    nbs.push_back(arg);
                }
                if (!arg_flag || name.length() >= 20) {
                    cout << "Error\n";
                    continue;
                }
                for (auto ch : name) {
                    if (!((ch >= '0' && ch <= '9') || ch == '_' ||
                          (ch >= 'A' && ch <= 'Z') ||
                          (ch >= 'a' && ch <= 'z'))) {
                        ch_flag = 0;
                    }
                }
                if (!ch_flag) {
                    cout << "Error\n";
                    continue;
                }
                auto it = m.find(name);
                if (it != m.end()) {
                    cout << "Error\n";
                    continue;
                }

                if (num < 0) {
                    cout << "Error\n";
                    continue;
                }
                m.insert({name, nbs});
            } else if (operates == "size") {
                cout << m.size() << '\n';
            } else if (operates == "erase") {
                cin >> name;
                auto it = m.find(name);
                if (it == m.end()) {
                    cout << "Error\n";
                    continue;
                }
                m.erase(name);
            } else if (operates == "count") {
                cin >> name;
                cout << m.count(name) << '\n';
            } else if (operates == "traverse") {
                if (!m.empty()) {
                    for (auto it = m.begin(); it != m.end(); ++it) {
                        int size = it->second.size();
                        cout << it->first << ' ';
                        for (int i = 0; i < size; ++i) {
                            cout << it->second[i] << ' ';
                        }
                        cout << '\n';
                    }
                }
            } else if (operates == "visitVec") {
                cin >> name;
                int index;
                cin >> index;
                auto it = m.find(name);
                if (m.empty() || it == m.end()) {
                    cout << "Error\n";
                    continue;
                }
                if (it->second.size() == 0 || index < 0 ||
                    index > it->second.size() - 1) {
                    cout << "Error\n";
                    continue;
                }
                cout << it->second[index] << '\n';
            } else if (operates == "visit") {
                cin >> name;
                auto it = m.find(name);
                if (m.empty() || it == m.end()) {
                    cout << "Error\n";
                    continue;
                }
                int size = it->second.size();
                cout << size << ' ';
                for (int i = 0; i < size; ++i) {
                    cout << it->second[i] << ' ';
                }
                cout << '\n';
            } else if (operates == "append") {
                cin >> name;
                int element;
                cin >> element;
                auto it = m.find(name);
                if (m.empty() || it == m.end()) {
                    cout << "Error\n";
                    continue;
                }
                if (element < 0) {
                    cout << "Error\n";
                    continue;
                }
                it->second.push_back(element);
            } else {
                cout << "Error\n";
                continue;
            }
        } else if (map_or_set == "set") {
            cin >> operates;
            if (operates == "size") {
                cout << s.size() << '\n';
            } else if (operates == "insert") {
                Activity item;
                int ch_flag = 1;
                cin >> item.avg >> item.level >> item.str;
                for (auto ch : item.str) {
                    if (!((ch >= '0' && ch <= '9') || ch == '_' ||
                          (ch >= 'A' && ch <= 'Z') ||
                          (ch >= 'a' && ch <= 'z'))) {
                        ch_flag = 0;
                    }
                }
                if (!ch_flag) {
                    cout << "Error\n";
                    continue;
                }
                if (item.avg < 0 || item.level < 0 || item.str.length() >= 20) {
                    cout << "Error\n";
                    continue;
                }
                auto it = s.find(item);
                if (it != s.end()) {
                    cout << "Error\n";
                    continue;
                }
                s.insert(item);
            } else if (operates == "erase") {
                Activity item;
                cin >> item.avg >> item.level >> item.str;
                auto it = s.find(item);
                if (it == s.end()) {
                    cout << "Error\n";
                    continue;
                }
                s.erase(item);
            } else if (operates == "count") {
                Activity item;
                cin >> item.avg >> item.level >> item.str;
                cout << s.count(item) << '\n';
            } else if (operates == "traverse") {
                if (!s.empty()) {
                    for (auto it = s.begin(); it != s.end(); ++it) {
                        cout << it->avg << ' ' << it->level << ' ' << it->str
                             << '\n';
                    }
                }
            } else {
                cout << "Error\n";
                continue;
            }
        } else if (map_or_set == "end") {
            return 0;
        } else {
            cout << "Error\n";
            continue;
        }
    }
}