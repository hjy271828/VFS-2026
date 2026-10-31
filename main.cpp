#include <cstdint>
#include <iostream>
#include <variant>
#include <optional>
#include <map>
#include <vector>
#include <cmath>
#include <stack>
#include <algorithm>

enum Type { TXT, MD, BIN, DIR };

struct Entry {
    std::string name;
    Type type;
    std::variant<std::string, std::vector<uint8_t>> content;
    std::map<std::string,int> children;
    int father;
    bool deleted=false;
};

std::vector<Entry> v;
int curr_dir;

struct idx_and_size{
    int idx;
    int size;
    bool operator<(const idx_and_size &other) const{
       if(size!=other.size){
        return size>other.size;
       }else {
        return v[idx].name < v[other.idx].name;
       }
    }
};

enum Undoable_Type {MKDIR,CREATE,APPEND,CD,WRITE,RM};
struct undoable_item{
    Undoable_Type type;
    int index;
    std::variant<std::string,std::vector<uint8_t>> former_content;
    int former_length;
};

std::stack<undoable_item> undoable_history;

int FindDir(const std::string & path) {
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
        if(find_it!=std::string::npos){
            if(find_it==curr_idx){
                return -1;
            }
            curr_str_end=find_it;
        }
        std::string next_dir_name = path.substr(curr_idx,curr_str_end-curr_idx);
        // cerr<<"next_dir_name: "<<next_dir_name<<'\n';
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
    std::string src;
    std::cin >> src;
    auto it = v[curr_dir].children.find(src);
    if(it!=v[curr_dir].children.end()){
        std::cout<<"Invalid operation\n";
        return;
    }
    Entry new_dir;
    new_dir.type=DIR;
    new_dir.father=curr_dir;
    new_dir.name=src;
    v[curr_dir].children.insert({src,v.size()});
    v.push_back(new_dir);

    undoable_item new_item;
    new_item.type=MKDIR;
    new_item.index=v.size()-1;
    undoable_history.push(new_item);
}

void cd(){
    std::string path;
    std::cin>>path;
    int path_pos= FindDir(path);
    if(path_pos==-1){
        std::cout<<"Invalid operation\n";
        return;
    }

    undoable_item new_item;
    new_item.type=CD;
    new_item.index=curr_dir;
    undoable_history.push(new_item);

    curr_dir = path_pos;
    // cerr<<"curr_dir: "<<curr_dir<<'\n';
}

void create(){
    std::string type;
    std::string name;
    std::string content;
    std::cin>>type>>name;
    std::getline(std::cin,content);

    if(type!="md"&&type!="bin"&&type!="txt"){
        std::cout<<"Invalid operation\n";
        return;
    }

    auto find_it = v[curr_dir].children.find(name);
    if(find_it!=v[curr_dir].children.end()){
        std::cout<<"Invalid operation\n";
        return;
    }

    int first_place = content.find('"');
    int second_place = content.rfind('"');
    if(first_place==std::string::npos || second_place==std::string::npos || first_place>=second_place){
        std::cout<<"Invalid operation\n";
        return;
    }
    content = content.substr(first_place+1,second_place-first_place-1);
    int content_len =content.length();
    Entry new_file;
    new_file.father=curr_dir;
    new_file.name=name;
    if(type=="md"){
        new_file.type=MD;
        new_file.content=std::string(content);
    }else if(type=="txt"){
        new_file.type=TXT;
        new_file.content=std::string(content);
    }else {
        new_file.type=BIN;
        int cur_hex=0;
        int one_finished=1;
        std::vector<uint8_t> cur_content;
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

    undoable_item new_item;
    new_item.type=CREATE;
    new_item.index=v.size()-1;
    undoable_history.push(new_item);
}

void append(){
    std::string name;
    std::string content;
    std::cin>>name;
    std::getline(std::cin,content);
    int first_place = content.find('"');
    int second_place = content.rfind('"');
    if(first_place==std::string::npos || second_place==std::string::npos || first_place>=second_place){
        std::cout<<"Invalid operation\n";
        return;
    }
    content = content.substr(first_place+1,second_place-first_place-1);

    auto find_it = v[curr_dir].children.find(name);
    if(find_it==v[curr_dir].children.end()){
        std::cout<<"Invalid operation\n";
        return;
    }
    Type its_type = v[find_it->second].type;
    if(its_type!=TXT && its_type!=MD){
        std::cout<<"Invalid operation\n";
        return;
    }

    undoable_item new_item;
    new_item.type=APPEND;
    new_item.index=find_it->second;
    new_item.former_length=(std::get<std::string>(v[find_it->second].content)).length();
    undoable_history.push(new_item);

    v[find_it->second].content=std::get<std::string>(v[find_it->second].content)+content;
}

void cat(){
    std::string name;
    std::cin>>name;

    auto find_it = v[curr_dir].children.find(name);
    if(find_it==v[curr_dir].children.end()){
        std::cout<<"Invalid operation\n";
        return;
    }
    Type its_type = v[find_it->second].type;
    if(its_type!=TXT && its_type!=MD){
        std::cout<<"Invalid operation\n";
        return;
    }
    std::cout<<name<<':';
    std::cout<<std::get<std::string>(v[find_it->second].content)<<'\n';
}

void read(){
    std::string name;
    int offset,length;
    std::cin>>name>>offset>>length;
    auto find_it = v[curr_dir].children.find(name);
    if(find_it==v[curr_dir].children.end()){
        std::cout<<"Invalid operation\n";
        return;
    }
    Type its_type = v[find_it->second].type;
    if(its_type!=BIN){
        std::cout<<"Invalid operation\n";
        return;
    }
    int bin_size = (std::get<std::vector<uint8_t>>(v[find_it->second].content)).size();
    if(offset+length>bin_size){
        std::cout<<"Invalid operation\n";
        return;
    }

    std::cout<<name<<':';
    for(int i=offset;i<offset+length;++i){
        int cur_int = (std::get<std::vector<uint8_t>>(v[find_it->second].content))[i];
        int first = cur_int/16;
        int second = cur_int%16;
        if(first<10){
            std::cout<<first;
        }else{
            std::cout<<char('A'+int(first-10));
        }
        if(second<10){
            std::cout<<second<<" ";
        }else{
            std::cout<<char('A'+int(second-10))<<" ";
        }
    }
    std::cout<<'\n';
}

void write(){
    std::string name;
    int offset;
    std::string hexdata;
    std::cin>>name>>offset;
    std::getline(std::cin,hexdata);

    int first_place = hexdata.find('"');
    int second_place = hexdata.rfind('"');
    if(first_place==std::string::npos || second_place==std::string::npos || first_place>=second_place){
        std::cout<<"Invalid operation\n";
        return;
    }
    hexdata = hexdata.substr(first_place+1,second_place-first_place-1);

    auto find_it = v[curr_dir].children.find(name);
    if(find_it==v[curr_dir].children.end()){
        std::cout<<"Invalid operation\n";
        return;
    }
    Type its_type = v[find_it->second].type;
    if(its_type!=BIN){
        std::cout<<"Invalid operation\n";
        return;
    }

    undoable_item new_item;
    new_item.type=WRITE;
    new_item.index=find_it->second;
    new_item.former_content=std::get<std::vector<uint8_t>>(v[find_it->second].content);
    undoable_history.push(new_item);

    int bin_size = (std::get<std::vector<uint8_t>>(v[find_it->second].content)).size();
    if(bin_size<offset){
        for(int i=bin_size;i<offset;++i){
            std::get<std::vector<uint8_t>>(v[find_it->second].content).push_back(0);
        }
    }
    bin_size = (std::get<std::vector<uint8_t>>(v[find_it->second].content)).size();

    int cur_hex=0;
    int one_finished=1;
    int hexdata_len=hexdata.length();

    int cur_idx = offset;
    for(int i=0;i<hexdata_len;++i){
        int tmp=0;
        if(hexdata[i]>='0' && hexdata[i]<='9'){
            tmp = int(hexdata[i]-'0');
        }else if(hexdata[i]>='A' && hexdata[i]<='F'){
            tmp = int(hexdata[i]-'A') +10;
        }else{
            continue;
        }
        if(one_finished){
            cur_hex+=16*tmp;
            one_finished=0;
        }else{
            cur_hex+=tmp;
            if(cur_idx<bin_size){
                std::get<std::vector<uint8_t>>(v[find_it->second].content)[cur_idx]=cur_hex;
            }else{
                std::get<std::vector<uint8_t>>(v[find_it->second].content).push_back(cur_hex);
            }
            cur_hex=0;
            one_finished=1;
            cur_idx++;
        }
    }
}

void ls(){
    for(auto [name,e]: v[curr_dir].children){
        bool d = (v[e].type==DIR);
        std::cout<<name<<(d?"/":"")<<'\n';
    }
}

void rm(){
    std::string name;
    std::cin>>name;
    auto find_it = v[curr_dir].children.find(name);
    if(find_it==v[curr_dir].children.end()){
        std::cout<<"Invalid operation\n";
        return;
    }

    Type its_type = v[find_it->second].type;
    if(its_type==DIR){
        std::cout<<"Invalid operation\n";
        return;
    }

    undoable_item new_item;
    new_item.index=find_it->second;
    new_item.type=RM;
    undoable_history.push(new_item);

    v[find_it->second].deleted=true;
    v[curr_dir].children.erase(find_it);    
}

void stat(){
    std::string name;
    std::cin>>name;
    auto find_it = v[curr_dir].children.find(name);
    if(find_it==v[curr_dir].children.end()){
        std::cout<<"Invalid operation\n";
        return;
    }

    Type its_type = v[find_it->second].type;
    std::cout<<name<<":";

    int size =0;
    if(its_type==TXT){
        std::cout<<"txt:";
        size = (std::get<std::string>(v[find_it->second].content)).length();
    }else if(its_type==MD){
        std::cout<<"md:";
        size = (std::get<std::string>(v[find_it->second].content)).length();
    }else if(its_type==BIN){
        std::cout<<"bin:";
        size = (std::get<std::vector<uint8_t>>(v[find_it->second].content)).size();
    }else if(its_type==DIR){
        std::cout<<"dir:";
        size =0;
    }
    std::cout<<size<<'\n';
}

void top(){
    int k;
    std::cin>>k;
    std::vector<idx_and_size> file_vec;
    for(auto [name,e]: v[curr_dir].children){
        idx_and_size tmp;
        tmp.idx=e;
        if(v[e].type==TXT){
            tmp.size = (std::get<std::string>(v[e].content)).length();
        }else if(v[e].type==MD){
            tmp.size = (std::get<std::string>(v[e].content)).length();
        }else if(v[e].type==BIN){
            tmp.size = (std::get<std::vector<uint8_t>>(v[e].content)).size();
        }else if(v[e].type==DIR){
            continue;
        }
        file_vec.push_back(tmp);
    }
    // int cur_top_cnt=0;
    // for(auto it = s.begin();it!=s.end() && cur_top_cnt<k;++it){
    //     std::cout<<v[it->idx].name<<':'<<(it->size)<<'\n';
    //     cur_top_cnt++;
    // }
    int file_vec_size = file_vec.size();
    if(k>log(file_vec_size)/log(2)){
        sort(file_vec.begin(),file_vec.end());
        for(int i=0;i<file_vec_size && i<k;++i){
            std::cout<<v[file_vec[i].idx].name<<':'<<file_vec[i].size<<'\n';
        }
    }else{
        for(int i=0;i<k;++i){
            bool flg=1;
            for(int j=0;j<file_vec_size-1;++j){
                if(file_vec[j]<file_vec[j+1]){
                    idx_and_size tmp = file_vec[j+1];
                    file_vec[j+1] = file_vec[j];
                    file_vec[j] = tmp;
                    flg=0;
                }
            }
            if(flg){
                break;
            }
        }
        for(int i=file_vec_size-1;i>=file_vec_size-k;--i){
            std::cout<<v[file_vec[i].idx].name<<':'<<file_vec[i].size<<'\n';
        }
    }
}

void undo(){
    if(undoable_history.empty()){
        std::cout<<"Invalid operation\n";
        return;
    }
    undoable_item cur_item = undoable_history.top();
    undoable_history.pop();
    Undoable_Type cur_type = cur_item.type;
    int cur_idx = cur_item.index;
    int cur_father = v[cur_idx].father;
    std::string cur_name = v[cur_idx].name;
    if(cur_type==MKDIR){
        v[cur_idx].deleted=true;
        v[cur_father].children.erase(cur_name);
    }else if(cur_type==CREATE){
        v[cur_idx].deleted=true;
        v[cur_father].children.erase(cur_name);
    }else if(cur_type==APPEND){
        v[cur_idx].content = std::get<std::string>(v[cur_idx].content).substr(0,cur_item.former_length);
    }else if(cur_type==CD){
        curr_dir=cur_idx;
    }else if(cur_type==WRITE){
        v[cur_idx].content=cur_item.former_content;
    }else if(cur_type==RM){
        v[cur_idx].deleted=false;
        v[cur_father].children.insert({cur_name,cur_idx});
    }
}

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    int n;
    std::cin >> n;
    Entry root;
    root.type=DIR;
    root.name="/";
    root.father=-1;
    v.push_back(root);
    curr_dir=0;
    for (int i = 0; i < n; ++i) {
        std::string operators;
        std::cin >> operators;
        if (operators == "mkdir") {
           mkdir();
        }else if(operators=="cd"){
            cd();
        }else if(operators=="create"){
            create();
        }else if(operators=="append"){
            append();
        }else if(operators=="cat"){
            cat();
        }else if(operators=="read"){
            read();
        }else if(operators=="write"){
            write();
        }else if(operators=="ls"){
            ls();
        }else if(operators=="rm"){
            rm();
        }else if(operators=="stat"){
            stat();
        }else if(operators=="top"){
            top();
        }else if(operators=="undo"){
            undo();
        }
    }
}