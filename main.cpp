#include <cstdint>
#include <iostream>
#include <variant>
#include <optional>
#include <unordered_map>
#include <vector>


using namespace std;
enum Type { TXT, MD, BIN, DIR };
bool curr_in_root;
struct Entry {
    Type type;
    variant<string, vector<uint8_t>> content;
    vector<string> children;
    string father;
    bool father_is_root;
};

unordered_map<string, Entry> m;
unordered_map<string, Entry>::iterator curr_dir;

optional<string> FindDir(string name) {
    // TODO
    return nullopt;
}

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    int n;
    cin >> n;
    for (int i = 0; i < n; ++i) {
        string operators;
        cin >> operators;
        if (operators == "mkdir") {
            string src;
            cin >> src;
        }
    }
}