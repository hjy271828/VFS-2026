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

void mkdir(){
    string src;
    cin >> src;
    auto it = v[curr_dir].children.find(src);
    if(it!=v[curr_dir].children.end()){
        cout<<"Invalid operation\n";
        return;
    }
    Entry new_dir;
    new_dir.type=DIR;
    new_dir.father=curr_dir;
    new_dir.name=src;
    v[curr_dir].children.insert({src,v.size()});
    v.push_back(new_dir);
}

void cd(){
    string path;
    cin>>path;
    int path_pos= FindDir(path);
    if(path_pos==-1){
        cout<<"Invalid operation\n";
        return;
    }
    curr_dir = path_pos;
    cerr<<"curr_dir: "<<curr_dir<<'\n';
}

void create(){
    string type;
    string name;
    string content;
    cin>>type>>name;
    getline(cin,content);

    if(type!="md"&&type!="bin"&&type!="txt"){
        cout<<"Invalid operation\n";
        return;
    }

    auto find_it = v[curr_dir].children.find(name);
    if(find_it!=v[curr_dir].children.end()){
        cout<<"Invalid operation\n";
        return;
    }

    int first_place = content.find('"');
    int second_place = content.rfind('"');
    if(first_place==string::npos || second_place==string::npos || first_place>=second_place){
        cout<<"Invalid operation\n";
        return;
    }
    content = content.substr(first_place+1,second_place-first_place-1);
    int content_len =content.length();
    Entry new_file;
    new_file.father=curr_dir;
    new_file.name=name;
    if(type=="md"){
        new_file.type=MD;
        new_file.content=string(content);
    }else if(type=="txt"){
        new_file.type=TXT;
        new_file.content=string(content);
    }else {
        new_file.type=BIN;
        int cur_hex=0;
        int one_finished=1;
        vector<uint8_t> cur_content;
        for(int i=0;i<content_len;++i){
            int tmp=0;
            if(content[i]>='0' && content[i]<='9'){
                tmp = int(content[i]-'0');
            }else if(content[i]>='A' && content[i]<='F'){
                tmp = int(content[i]-'A') +10;
            }else{
                continue;
            }
            if(one_finished){
                cur_hex+=16*tmp;
                one_finished=0;
            }else{
                cur_hex+=tmp;
                cur_content.push_back(cur_hex);
                cur_hex=0;
                one_finished=1;
            }
        }
        new_file.content=cur_content;
    }
    v[curr_dir].children.insert({name,v.size()});
    v.push_back(new_file);
}

void append(){
    string name;
    string content;
    cin>>name;
    getline(cin,content);
    int first_place = content.find('"');
    int second_place = content.rfind('"');
    if(first_place==string::npos || second_place==string::npos || first_place>=second_place){
        cout<<"Invalid operation\n";
        return;
    }
    content = content.substr(first_place+1,second_place-first_place-1);

    auto find_it = v[curr_dir].children.find(name);
    if(find_it==v[curr_dir].children.end()){
        cout<<"Invalid operation\n";
        return;
    }
    Type its_type = v[find_it->second].type;
    if(its_type!=TXT && its_type!=MD){
        cout<<"Invalid operation\n";
        return;
    }

    v[find_it->second].content=get<string>(v[find_it->second].content)+content;
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
           mkdir();
        }else if(operators=="cd"){
            cd();
        }else if(operators=="create"){
            create();
        }else if(operators=="append"){
            append();
        }
    }
}