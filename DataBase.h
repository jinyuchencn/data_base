//DataBase类的实现


#ifndef _DataBase_
#define _DataBase_

#include<iostream>
#include<sstream>
#include<map>

using namespace std;

#include "DataFile.h"
#include "IndexFile.h"

class IndexFile;

DataBase::DataBase(){
   root=new IndexFile();
   first=new IndexFile();
   Leaf_file_num=0;
   Brch_file_num=0;
}

DataBase::~DataBase(){
    delete root;
    delete first; 
}

//生成叶子节点

IndexFile DataBase::makeleaf(){
    IndexFile ret = IndexFile();        
    ret.change_type_to_leaf();   
    ret.change_name(this->produce_indexname(ret));    //根据文件数生成文件名
    Leaf_file_num++;
    return ret;     
}

//生成内节点

IndexFile DataBase::makebrch()                                     
{
    IndexFile ret = IndexFile();
    ret.change_type_to_Brch();
    ret.change_name(this->produce_indexname(ret));     //根据文件数生成文件名
    Brch_file_num += 1;
    return ret;
}

//清空缓存

void DataBase::clear_buffer(){
    buffer.clear();
}

//储存缓存

void DataBase::buffer_store(){
    buffer.store_Data();
    buffer.store_Index();
}

//修改数据，返回是否成功

bool DataBase::change(unsigned long long key,string &record){
    bool ok=this->delete_one(key);
    if(ok){
        ok=this->Insert(key,record);
        if(ok){
            return true;
        }
    }
    return false;
}

//范围查找

map<unsigned long long,string> DataBase::RangeFind(unsigned long long key1,unsigned long long key2){
    string temp="NONE";
    map<unsigned long long,string> back;

    while(temp=="NONE"){
        temp=this->Find(key1);
        
        if(key1>=key2){
            break;
        }
        key1++;
    }

    if(temp!="NONE"){
        back[key1-1]=temp;
    }



    IndexFile first;

    string IndexName=(root->find(key1,buffer)).filename;
    first=buffer.make_Index(IndexName);

    while (first.get_name() != "NONE" && first.get_key()[0]<=key2)
    {
        unsigned long long pos;
        bool range=true;
        //char *p;
        for (int i = 0; i < first.get_num(); ++i)
        {
            if(first.get_key()[i]>key2){
                range=false;
                break;
            }
            pos = first.get_key()[i];
            if(pos!=0&&pos>key1-1){
                string s;
                s=first.get_data_file()[i];
                DataFile OneData= buffer.make_Data(s);
                

                back[pos]=OneData.get_data()[i];
                // back.push_back(OneData.get_data()[i]);
            }
        }
        if(range==false){
            break;
        }
        if(first.get_next()=="NONE"){
            first.get_name()="NONE";
        }
        else{
            first = IndexFile(first.get_next());
        }
    }

    return back;

}

//查找

string DataBase::Find(unsigned long long key){
    result back =root->find(key,buffer);

    if(back.ok){
        IndexFile Index=buffer.make_Index(back.filename);
        DataFile Data=buffer.make_Data(Index.get_data_file()[back.pos]);
        return Data.get_data()[back.pos];
    }
    return "no record";
    
}

//删除

bool DataBase::delete_one(unsigned long long key){
    bool a=root->dele_tree(key,*this,buffer);
    return a;
}

//插入

bool DataBase::Insert(unsigned long long key,string &record){
    bool a=root->Insert(key,record,this,buffer);

    if(a){
        return true;
    }
    return false;
}

// 生成文件名

string DataBase::produce_indexname(IndexFile OneFile){
    string name="index_";
    if(OneFile.get_type()==Leaf){
        name+="Leaf";
        stringstream ss;
        string num;
        ss<<Leaf_file_num;
        ss>>num;
        name+=num;
        name+=".idx";
    }
    else{
        name+="Brch";
        stringstream ss;
        string num;
        ss<<Brch_file_num;
        ss>>num;
        name+=num;
        name+=".idx";
    }
    return name;
}
string DataBase::produce_dataname(){
    string name="data_";
    stringstream ss;
    string num;
    ss<<Leaf_file_num;
    ss>>num;
    name+=num;
    name+=".dat";
    return name;
}

//设置初节点与根节点

void DataBase::set_first()   
{
    IndexFile First=root->getfirst();
    first->copy(First);
}
void DataBase::set_root(IndexFile new_root){
    root->copy(new_root);
}

//打开与关闭

void DataBase::open(string rootname){
    root=new IndexFile(rootname);
    IndexFile f=root->getfirst();
    first=new IndexFile(f.get_name());
    Leaf_file_num=root->get_leaf_num() ;
    Brch_file_num=root->get_Brch_num() ;

}
void DataBase::close(){
    this->buffer_store();
    exit(0);
}

#endif 
