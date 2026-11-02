#include <cstdint>
#include <iostream>
#include <variant>
#include <map>
#include <vector>
#include <stack>
#include <set>

enum Type { TXT, MD, BIN, DIR };

struct size_compare{
    bool operator()(const int i1,const int i2) const;
};

struct Entry {
    std::string name;
    Type type;
    int size=0;
    std::variant<std::string, std::vector<uint8_t>> content;
    std::map<std::string,int> children;
    std::set <int,size_compare> size_rank; 
    int father;
    bool deleted=false;
};

std::vector<Entry> v;
int curr_dir;

bool size_compare::operator()(const int i1,const int i2) const{
    if(v[i1].size!=v[i2].size){
    return v[i1].size>v[i2].size;
    }else {
    return v[i1].name < v[i2].name;
    }
}

enum Undoable_Type {MKDIR,CREATE,APPEND,CD,WRITE,RM,RM_R};
struct undoable_item{
    Undoable_Type type;
    int index;
    std::vector<uint8_t> former_content;
    int modify_pos_start;
    int modify_pos_end; //not included
    int former_size;
};
std::stack<undoable_item> undoable_history;

struct redoable_item{
    Undoable_Type type;
    int index;
    std::variant<std::string,std::vector<uint8_t>> modified_content;
    int modify_pos_start;
    int modify_pos_end;
    int size_after_modify;
};
std::stack<redoable_item> redoable_history;

std::string hex_char="0123456789ABCDEF";

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

    while(!redoable_history.empty()){
        redoable_history.pop();
    }
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
        new_file.content=std::move(cur_content);
    }
    v[curr_dir].children.insert({name,v.size()});
    v.push_back(std::move(new_file));

    undoable_item new_item;
    new_item.type=CREATE;
    new_item.index=v.size()-1;
    undoable_history.push(new_item);

    int size =0;
    if(new_file.type==TXT){
        size = (std::get<std::string>(v[new_item.index].content)).length();
    }else if(new_file.type==MD){
        size = (std::get<std::string>(v[new_item.index].content)).length();
    }else if(new_file.type==BIN){
        size = (std::get<std::vector<uint8_t>>(v[new_item.index].content)).size();
    }
    v[new_item.index].size = size;
    v[curr_dir].size_rank.insert(new_item.index);

    while(!redoable_history.empty()){
        redoable_history.pop();
    }
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
    new_item.modify_pos_start=(std::get<std::string>(v[find_it->second].content)).length();
    undoable_history.push(new_item);

    std::get<std::string>(v[find_it->second].content).append(content);


    auto find_it_in_rank = v[curr_dir].size_rank.find(find_it->second);
    if(find_it_in_rank!=v[curr_dir].size_rank.end()){
        v[curr_dir].size_rank.erase(find_it_in_rank);
    }
    
    v[find_it->second].size = std::get<std::string>(v[find_it->second].content).size();
    v[curr_dir].size_rank.insert(find_it->second);

    while(!redoable_history.empty()){
        redoable_history.pop();
    }
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

    std::string out = name+":";
    out.reserve(name.size() + 2 + 3 * length);
    auto & bin = std::get<std::vector<uint8_t>>(v[find_it->second].content);
    for(int i=offset;i<offset+length;++i){
        int cur_int = bin[i];
        int first = cur_int/16;
        int second = cur_int%16;
        out+=hex_char[first];
        out+=hex_char[second];
        out+=" ";
    }
    out+='\n';
    std::cout<<out;
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
    new_item.former_size=int(std::get<std::vector<uint8_t>>(v[find_it->second].content).size());
    new_item.modify_pos_start = offset;
    new_item.modify_pos_end = std::min(new_item.former_size,int(offset+(hexdata.length()+1)/3));
    int new_bin_size = std::max(new_item.former_size,int(offset+(hexdata.length()+1)/3)); 
    if(new_item.modify_pos_end>new_item.modify_pos_start && offset<new_item.former_size){
        new_item.former_content.assign(std::get<std::vector<uint8_t>>(v[find_it->second].content).begin()+new_item.modify_pos_start,
        std::get<std::vector<uint8_t>>(v[find_it->second].content).begin()+new_item.modify_pos_end);
    }

    undoable_history.push(std::move(new_item));

    std::get<std::vector<uint8_t>>(v[find_it->second].content).resize(new_bin_size,0);

    int add_cnt = (hexdata.length()+1)/3;
    auto & bin = std::get<std::vector<uint8_t>>(v[find_it->second].content);
    for(int i=0;i<add_cnt;++i){
        int tmp=0;
        if(hexdata[3*i]>='0' && hexdata[3*i]<='9'){
            tmp = int(hexdata[3*i]-'0')*16;
        }else if(hexdata[3*i]>='A' && hexdata[3*i]<='F'){
            tmp = (int(hexdata[3*i]-'A') +10)*16;
        }
        if(hexdata[3*i+1]>='0' && hexdata[3*i+1]<='9'){
            tmp += int(hexdata[3*i+1]-'0');
        }else if(hexdata[3*i+1]>='A' && hexdata[3*i+1]<='F'){
            tmp += (int(hexdata[3*i+1]-'A') +10);
        }
        bin[offset+i]=tmp;
    }

    auto find_it_in_rank = v[curr_dir].size_rank.find(find_it->second);
    if(find_it_in_rank!=v[curr_dir].size_rank.end()){
        v[curr_dir].size_rank.erase(find_it_in_rank);
    }
    v[find_it->second].size = std::get<std::vector<uint8_t>>(v[find_it->second].content).size();
    v[curr_dir].size_rank.insert(find_it->second);

    while(!redoable_history.empty()){
        redoable_history.pop();
    }
}

void ls(){
    std::string out;
    for(const auto & [name,e]: v[curr_dir].children){
        bool d = (v[e].type==DIR);
        out+=name;
        out+=(d?"/":"");
        out+='\n';
    }
    std::cout<<out;
}

void rm_r();

void rm(){
    std::string name;
    std::cin>>name;
    if(name=="-r"){
        rm_r();
        return;
    }
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
    v[curr_dir].size_rank.erase(find_it->second);
    v[curr_dir].children.erase(find_it);    

    while(!redoable_history.empty()){
        redoable_history.pop();
    }
}

void rm_r(){
    std::string name;
    std::cin>>name;
    auto find_it = v[curr_dir].children.find(name);
    if(find_it==v[curr_dir].children.end()){
        std::cout<<"Invalid operation\n";
        return;
    }

    Type its_type = v[find_it->second].type;

    undoable_item new_item;
    new_item.index=find_it->second;
    new_item.type=RM_R;
    undoable_history.push(new_item);

    v[find_it->second].deleted=true;
    if(its_type!=DIR){
        v[curr_dir].size_rank.erase(find_it->second);
    }
    v[curr_dir].children.erase(find_it);    

    while(!redoable_history.empty()){
        redoable_history.pop();
    }
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

    if(its_type==TXT){
        std::cout<<"txt:";
    }else if(its_type==MD){
        std::cout<<"md:";
    }else if(its_type==BIN){
        std::cout<<"bin:";
    }else if(its_type==DIR){
        std::cout<<"dir:";
    }
    std::cout<<v[find_it->second].size<<'\n';
}

void top(){
    int k;
    std::cin>>k;
    int ouput_cnt =0;
    auto & cur_dir_rank = v[curr_dir].size_rank;
    for(auto it = cur_dir_rank.begin();it!=cur_dir_rank.end() && ouput_cnt<k;++it){
        ++ouput_cnt;
        std::cout<<v[(*it)].name<<':'<<v[(*it)].size<<'\n';
    }
}

void undo(){
    if(undoable_history.empty()){
        std::cout<<"Invalid operation\n";
        return;
    }
    undoable_item cur_item = std::move(undoable_history.top());
    undoable_history.pop();
    Undoable_Type cur_type = cur_item.type;
    int cur_idx = cur_item.index;
    int cur_father = v[cur_idx].father;
    std::string cur_name = v[cur_idx].name;
    if(cur_type==MKDIR){
        redoable_item new_redoable;
        new_redoable.type = MKDIR;
        new_redoable.index = cur_idx;
        redoable_history.push(new_redoable);

        v[cur_idx].deleted=true;
        v[cur_father].children.erase(cur_name);
    }else if(cur_type==CREATE){
        redoable_item new_redoable;
        new_redoable.type = CREATE;
        new_redoable.index = cur_idx;
        redoable_history.push(new_redoable);

        v[cur_idx].deleted=true;
        v[cur_father].children.erase(cur_name);
        v[cur_father].size_rank.erase(cur_idx);
    }else if(cur_type==APPEND){
        redoable_item new_redoable;
        new_redoable.type = APPEND;
        new_redoable.index = cur_idx;
        new_redoable.modify_pos_start = cur_item.modify_pos_start;
        std::get<std::string>(new_redoable.modified_content) 
      = std::get<std::string>(v[cur_idx].content).substr(cur_item.modify_pos_start,v[cur_idx].size-cur_item.modify_pos_start);
        redoable_history.push(new_redoable);

        std::get<std::string>(v[cur_idx].content).resize(cur_item.modify_pos_start);
        v[cur_father].size_rank.erase(cur_idx);
        v[cur_idx].size = std::get<std::string>(v[cur_idx].content).size();
        v[cur_father].size_rank.insert(cur_idx);
    }else if(cur_type==CD){
        redoable_item new_redoable;
        new_redoable.type = cur_type;
        new_redoable.index = curr_dir;
        redoable_history.push(new_redoable);

        curr_dir=cur_idx;
    }else if(cur_type==WRITE){
        redoable_item new_redoable;
        new_redoable.type = WRITE;
        new_redoable.index = cur_idx;
        new_redoable.modified_content = std::vector<uint8_t>{};
        new_redoable.modify_pos_start = cur_item.modify_pos_start;
        if(v[cur_idx].size>cur_item.former_size){
            new_redoable.modify_pos_end = v[cur_idx].size;
        }else{
            new_redoable.modify_pos_end = cur_item.modify_pos_end;
        }
        std::get<std::vector<uint8_t>>(new_redoable.modified_content).assign(std::get<std::vector<uint8_t>>(v[cur_idx].content).begin()+new_redoable.modify_pos_start,
        std::get<std::vector<uint8_t>>(v[cur_idx].content).begin()+new_redoable.modify_pos_end);
        redoable_history.push(new_redoable);
        
        std::get<std::vector<uint8_t>>(v[cur_idx].content).resize(cur_item.former_size);
        auto & bin = std::get<std::vector<uint8_t>>(v[cur_idx].content);
        for(int i=cur_item.modify_pos_start;i<=cur_item.modify_pos_end-1 && i<=cur_item.former_size-1;++i){
            bin[i]=cur_item.former_content[i-cur_item.modify_pos_start];
        }
        v[cur_father].size_rank.erase(cur_idx);
        v[cur_idx].size = std::get<std::vector<uint8_t>>(v[cur_idx].content).size();
        v[cur_father].size_rank.insert(cur_idx);
    }else if(cur_type==RM){
        redoable_item new_redoable;
        new_redoable.type = RM;
        new_redoable.index = cur_idx;
        redoable_history.push(new_redoable);

        v[cur_idx].deleted=false;
        v[cur_father].children.insert({cur_name,cur_idx});
        v[cur_father].size_rank.insert(cur_idx);
    }else if(cur_type==RM_R){
        redoable_item new_redoable;
        new_redoable.type = RM_R;
        new_redoable.index = cur_idx;
        redoable_history.push(new_redoable);

        v[cur_idx].deleted=false;
        v[cur_father].children.insert({cur_name,cur_idx});
        if(v[cur_idx].type!=DIR){
            v[cur_father].size_rank.insert(cur_idx);
        }
    }
}

void redo(){
    if(redoable_history.empty()){
        std::cout<<"Invalid operation\n";
        return;
    }
    redoable_item redo_item = redoable_history.top();
    redoable_history.pop();
    Undoable_Type cur_type = redo_item.type;
    int cur_index = redo_item.index;
    int cur_father = v[cur_index].father;
    std::string cur_name = v[cur_index].name;
    if(cur_type==MKDIR){
        undoable_item new_undoable;
        new_undoable.type = cur_type;
        new_undoable.index = cur_index;
        undoable_history.push(new_undoable);

        v[cur_index].deleted = false;
        v[cur_father].children.insert({cur_name,cur_index});
    }else if(cur_type == CREATE){
        undoable_item new_undoable;
        new_undoable.type = cur_type;
        new_undoable.index = cur_index;
        undoable_history.push(new_undoable);

        v[cur_index].deleted = false;
        v[cur_father].children.insert({cur_name,cur_index});
        v[cur_father].size_rank.insert(cur_index);
    }else if(cur_type == APPEND){
        undoable_item new_undoable;
        new_undoable.type = cur_type;
        new_undoable.index = cur_index;
        new_undoable.modify_pos_start = redo_item.modify_pos_start;
        undoable_history.push(new_undoable);

        (std::get<std::string>(v[cur_index].content)).append(std::get<std::string>(redo_item.modified_content));
        v[cur_father].size_rank.erase(cur_index);
        v[cur_index].size = std::get<std::string>(v[cur_index].content).length();
        v[cur_father].size_rank.insert(cur_index);
    }else if(cur_type == CD){
        undoable_item new_undoable;
        new_undoable.type = cur_type;
        new_undoable.index = curr_dir;
        undoable_history.push(new_undoable);

        curr_dir = redo_item.index;
    }else if(cur_type == WRITE){
        undoable_item new_undoable;
        new_undoable.type = cur_type;
        new_undoable.index = cur_index;
        auto & bin = std::get<std::vector<uint8_t>>(v[cur_index].content);
        new_undoable.former_size = int(bin.size());
        new_undoable.modify_pos_start = redo_item.modify_pos_start;
        int new_bin_size = std::max(new_undoable.former_size,redo_item.modify_pos_end);
        new_undoable.modify_pos_end = std::min(new_undoable.former_size,redo_item.modify_pos_end);
        if(new_undoable.modify_pos_start<new_undoable.modify_pos_end){
            new_undoable.former_content.assign(bin.begin()+new_undoable.modify_pos_start,
            bin.begin()+new_undoable.modify_pos_end);
        }
        undoable_history.push(std::move(new_undoable));

        bin.resize(new_bin_size,0);
        for(int i=redo_item.modify_pos_start;i<new_bin_size;++i){
            bin[i] = std::get<std::vector<uint8_t>>(redo_item.modified_content)[i-redo_item.modify_pos_start];
        }
        v[cur_father].size_rank.erase(cur_index);
        v[cur_index].size = bin.size();
        v[cur_father].size_rank.insert(cur_index);        
    }else if(cur_type == RM){
        undoable_item new_undoable;
        new_undoable.type = cur_type;
        new_undoable.index = cur_index;
        undoable_history.push(new_undoable);

        v[cur_index].deleted = true;
        v[cur_father].children.erase(cur_name);
        v[cur_father].size_rank.erase(cur_index);
    }else if(cur_type == RM_R){
        undoable_item new_undoable;
        new_undoable.type = cur_type;
        new_undoable.index = cur_index;
        undoable_history.push(new_undoable);

        v[cur_index].deleted = true;
        v[cur_father].children.erase(cur_name);
        if(v[cur_index].type!=DIR){
            v[cur_father].size_rank.erase(cur_index);
        }
    }
}

const std::string T = "\xE2\x94\x9C\xE2\x94\x80\xE2\x94\x80\x20";
const std::string L = "\xE2\x94\x94\xE2\x94\x80\xE2\x94\x80\x20";
const std::string I ="\xE2\x94\x82\x20\x20\x20";
const std::string blanks = "\x20\x20\x20\x20";

void print_tree(int cur_pos,std::string & head){
    std::cerr<<"cur_pos:"<<cur_pos<<" head:"<<head<<'\n';
    std::cout<<v[cur_pos].name<<'\n';
    for(auto it = v[cur_pos].children.begin();it!=v[cur_pos].children.end();++it){
        std::cout<<head;
        if(it==std::prev(v[cur_pos].children.end())){
            std::cout<<L;
            if(v[it->second].type!=DIR){
                std::cout<<(it->first)<<'\n';
                continue;
            }else{
                head+=blanks;
                cur_pos = it->second;
                print_tree(cur_pos,head);
                cur_pos = v[cur_pos].father;
                head.resize(head.length()-blanks.length());
            }
        }else{
            std::cout<<T;
            if(v[it->second].type!=DIR){
                std::cout<<(it->first)<<'\n';
                continue;
            }else{
                head+=I;
                cur_pos = it->second;
                print_tree(cur_pos,head);
                cur_pos = v[cur_pos].father;
                head.resize(head.length()-I.length());
            }
        }
    }
}

void tree(){
    std::string head ="";    
    print_tree(curr_dir,head);
}

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    int n;
    std::cin >> n;
    v.reserve(n+1);
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
        }else if(operators=="redo"){
            redo();
        }else if(operators=="tree"){
            tree();
        }
    }
}