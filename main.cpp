#include <cstdint>
#include <iostream>
#include <variant>
#include <optional>
#include <map>
#include <vector>


using namespace std;
enum Type { TXT, MD, BIN, DIR };
struct Entry {
    string name;
    Type type;
    variant<string, vector<uint8_t>> content;
    map<string,int> children;
    int father;
    bool deleted=false;
};

vector<Entry> v;
int curr_dir;
int FindDir(const string & path) {
    int length = path.length();
    if(!length){
        return -1;
        cerr<<"no char in path\n";
    }
    int curr_idx;
    int curr_pos;
    if(path[0]=='/'){
        curr_idx=1;
        curr_pos=0;
    }else{
        curr_idx=0;
        curr_pos=curr_dir;
    }
    while(curr_idx<length){
        int curr_str_end=length;
        int find_it = path.find('/',curr_idx);
        if(find_it!=string::npos){
            if(find_it==curr_idx){
                return -1;
            }
            curr_str_end=find_it;
        }
        string next_dir_name = path.substr(curr_idx,curr_str_end-curr_idx);
        cerr<<"next_dir_name: "<<next_dir_name<<'\n';
        if(next_dir_name==".."){
            int father_pos = v[curr_pos].father;
            if(father_pos<0){
                return -1;
            }
            curr_pos = father_pos;
        }else if(next_dir_name=="."){
            
        }else{
            auto find_children = v[curr_pos].children.find(next_dir_name);
            if(find_children==v[curr_pos].children.end()){
                return -1;
            }
            if(v[(*find_children).second].type!=DIR){
                return -1;
            }
            curr_pos=(*find_children).second;
        }
        curr_idx=curr_str_end+1;
    }
    return curr_pos;
}

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    int n;
    cin >> n;
    Entry root;
    root.type=DIR;
    root.name="/";
    root.father=-1;
    v.push_back(root);
    curr_dir=0;
    for (int i = 0; i < n; ++i) {
        string operators;
        cin >> operators;
        if (operators == "mkdir") {
            string src;
            cin >> src;
            auto it = v[curr_dir].children.find(src);
            if(it!=v[curr_dir].children.end()){
                cout<<"Invalid operation\n";
                continue;
            }
            Entry new_dir;
            new_dir.type=DIR;
            new_dir.father=curr_dir;
            new_dir.name=src;
            v[curr_dir].children.insert({src,v.size()});
            v.push_back(new_dir);
        }else if(operators=="cd"){
            string path;
            cin>>path;
            int path_pos= FindDir(path);
            if(path_pos==-1){
                cout<<"Invalid operation\n";
                continue;
            }
            curr_dir = path_pos;
            cerr<<"curr_dir: "<<curr_dir<<'\n';
        }else if(operators=="create"){
            string type;
            string name;
            string content;
            cin>>type>>name>>content;
            if(type!="MD"&&type!="BIN"&&type!="TXT"){
                cout<<"Invalid operation\n";
                continue;
            }
            int content_len =content.length();
            if(content_len<2 || content[0]!='"' || content[content_len]!='"'){
                cout<<"Invalid operation\n";
                continue;    
            }
            Entry new_file;
            new_file.father=curr_dir;
            new_file.name=name;
            if(type=="MD"){
                new_file.type=MD;
                new_file.content=content.substr(1,n-2);
            }else if(type=="TXT"){
                new_file.type=TXT;
                new_file.content=content.substr(1,n-2);
            }else {

            }
        }
    }
}