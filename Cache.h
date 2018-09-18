//缓存类的设计，主要使用了两个map，储存尚未存入硬盘的文件类
 
#ifndef _Cache_
#define _Cache_

#include<iostream>
#include <map>

using namespace std;

#include"IndexFile.h"
#include"DataFile.h"

// 声明文件类
class IndexFile;
class DataFile;

class Cache{

    friend class IndexFile;
    friend class DataFile;
    private:
        map<string,IndexFile> unsaved_file;
        map<string,DataFile> unsaved_data;
        
    public:
        int get_size(){return unsaved_data.size();};    //获得缓存大小

        void store_Data();                              //储存数据，清空缓存
        void store_Index();
        void clear();

        bool have_Index(string name);                   //检测文件对象是否在缓存中
        bool have_Data(string name);

        void Add_Data(string name,DataFile Data);       //向缓存中写入文件对象
        void Add_Index(string name,IndexFile Index);

        IndexFile make_Index(string name);              //从缓存中读取文件对象
        DataFile make_Data(string name);
};

// 查找函数的数据返回类型

struct result{
    string filename;
    int pos;
    bool ok;
};

// DataBase的声明

class DataBase{

    private:
        IndexFile *root;             //记录索引文件根文件
        IndexFile *first;            //记录链表头
        int Leaf_file_num;           //叶子个数
        int Brch_file_num;           //内节点个数   
        Cache buffer;                //缓存

    public:
        DataBase();                  //构造与析构函数
        ~DataBase();

        void open(string rootname);  //打开与关闭函数
        void close();

        void clear_buffer();         //对缓存的操作，储存与清空
        void buffer_store();

        int get_leaf_num(){return Leaf_file_num;};              //对成员变量的获取与修改
        int get_buffer_size(){return buffer.get_size();};
        Cache get_buffer(){return buffer;};
        IndexFile* get_first(){return first;};
        IndexFile* get_root(){return root;};
        void change_Cache(Cache a){buffer=a;};
        void set_first();
        void set_root(IndexFile root);
        void change_Leaf_file_num(int i){Leaf_file_num=i;};


        string produce_indexname(IndexFile OneFile);             //生成文件名
        string produce_dataname();

        bool Insert(unsigned long long key,string &record);      //对外接口
        string Find(unsigned long long key);
        bool delete_one(unsigned long long key);
        bool change(unsigned long long key,string &record);
        map<unsigned long long,string> RangeFind(unsigned long long key1,unsigned long long key2);

        IndexFile makeleaf();                                    //生成新文件
        IndexFile makebrch();


};

// DataFile的声明

class DataFile{
    private:
        vector<string> data;           //记录数据
        string name;                   //文件名 
        int num;                       //数据个数
    public: 
        DataFile(string name);         //构造函数，其中name为文件名 
        DataFile();

        int get_num(){return num;};    //获取和简单修改成员变量
        string get_name(){return name;};
        vector<string> get_data(){return data;};
        void change_data(string s,int i){data[i]=s;};
        void change_num(int i){num=i;};

        void insert(string &record,int pos);        //插入函数，在pos处插入record

        void store();                               //文件的储存与删除
        void delete_file();
};

// IndexFile类型的声明

typedef enum elemtype                       
{
    Leaf, Brch,NONE          //节点类型
}elemtype; 

// IndexFile的声明

class IndexFile{
    private:
        string name;                      //文件名
        elemtype type;                    //节点类型 
        string father;                    //父文件名  
        vector<string> children;          //子文件名们  
        vector<unsigned long long> key;   //子文件的key值
        vector<string> data_file;         //叶子节点会记录数据文件名  
        string front;                     //叶子节点的前文件名  
        string next;                      //叶子节点的后文件名  
        int num;                          //元素个数  
    public:
        IndexFile();                      //构造函数，name为文件名  
        IndexFile(string name);

        void store();                     //文件操作，储存、删除与复制文件
        void delete_file();
        void copy(IndexFile one);

        string get_next(){return next;};  //获取成员变量
        IndexFile getfirst();
        elemtype get_type(){return type;};
        string get_name(){return name;};
        vector<string> get_children(){return children;};
        int get_num(){return num;};
        vector<unsigned long long> get_key(){return key;};
        vector<string> get_data_file(){return data_file;};

        void change_type_to_leaf(){type=Leaf;};      //简单改变成员变量
        void change_type_to_Brch(){type=Brch;};
        void change_name(string s){name=s;};


        int get_leaf_num();                          //计算文件个数
        int get_Brch_num();

        bool Insert(unsigned long long key,string &record,DataBase *DataBase,Cache &buffer);  //插入、查找、删除、打印
        result find(unsigned long long key,Cache &buffer);          
        bool dele_tree( unsigned long long key,DataBase &DataBase,Cache &buffer);   
        void show();     

//      无根节点时生成根节点
        void makeroot(unsigned long long key,string data_file_name,DataBase *DataBase);       
        
//      插入叶子节点     
        void insert_leaf(int pos,unsigned long long key,string name,Cache &buffer);

//      分裂叶子后移动Index数据
        void move_leaf(IndexFile &des, int pos,string newdatafile,Cache &buffer);

//      分裂节点后移动Data数据
        void move_leaf_data(string name1,string name2,int pos,Cache &buffer);

//      分离后移动内节点数据
        void move_brch( IndexFile &des, int pos,Cache &buffer); 

//      插入内节点
        void insert_brch( int pos, unsigned long long key, IndexFile &right,Cache &buffer);

//      调整B+树（插入时）       
        IndexFile adjust(DataBase *DataBase,Cache &buffer);

//      删除内节点
        void dele_brch( int pos);

//      合并内节点
        void merge_lebrch( IndexFile &ptr, IndexFile &par, int parpos,Cache &buffer);

//      合并叶子节点
        void merge_leaf(IndexFile &des,  IndexFile &par, int parpos);

//      删除叶子节点
        void dele_leaf( int pos, int parpos,Cache &buffer);

//      调整内节点（删除时）
        IndexFile adjust_brch(Cache &buffer);

//      调整树（删除时）
        IndexFile adjust_tree(Cache &buffer);
};

//获取索引

IndexFile Cache::make_Index(string name){
    if(this->have_Index(name)){
        return unsaved_file[name];
    }
    else{
        return IndexFile(name);
    }
}   

//获取数据

DataFile Cache::make_Data(string name){
    if(this->have_Data(name)){
        return unsaved_data[name];
    }
    else{
        return DataFile(name);
    }
}

//清空缓存

void Cache::clear(){
    map<string,IndexFile> m1;
    map<string,DataFile> m2;
    unsaved_file=m1;
    unsaved_data=m2;
}

//储存数据

void Cache::store_Data(){
    map<string,DataFile>::iterator it;
    for(it=unsaved_data.begin();it!=unsaved_data.end();it++){
        it->second.store();
    }
}

//储存索引

void Cache::store_Index(){
    map<string,IndexFile>::iterator it;
    for(it=unsaved_file.begin();it!=unsaved_file.end();it++){
        it->second.store();
    }
}

//判断索引是否存在

bool Cache::have_Index(string name){
    return unsaved_file.count(name)==1;
}

//判断数据是否存在

bool Cache::have_Data(string name){
    return unsaved_data.count(name)==1;
}

//添加数据

void Cache::Add_Data(string name,DataFile Data){
    unsaved_data[name]=Data;
}

//添加索引

void Cache::Add_Index(string name,IndexFile Index){
    unsaved_file[name]=Index;
}


#endif // !_Cache