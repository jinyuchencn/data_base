#ifndef _DataFile_
#define _DataFile_

#include<iostream>
#include<vector>
#include<map>
#include<sstream>
#include<windows.h>
#include<fstream>
#include<string>

using namespace std;

#include "IndexFile.h"
#include "DataFile.h"

class IndexFile;

//构造函数

DataFile::DataFile(){
    name = "NONE";
    num = 0;
    for(int i = 0;i<maxsize+1;i++){
        data.push_back("NONE");
    }
}
DataFile::DataFile(string filename){
    name=filename;
    num=0;
    fstream file("data/"+filename);           //打开文件

    if(file){                                 //文件存在，读数据
        while(!file.eof()){
            string s;
            file >> s;
            data.push_back(s);
        }
        while(data.size()>maxsize+1){
            data.erase(data.end());
        }
        num=data.size();
    }
    else{                                     //文件不存在，初始化数据  
        for(int i=0;i<maxsize+1;i++){
            data.push_back("NONE");
            file<<"NONE";
            file<<" ";
        }
    }
}

//插入数据

void DataFile::insert(string &record,int pos){
    string dirName2 = "data";
    bool flag2 = CreateDirectory(dirName2.data(), NULL);    
    vector <string>::iterator Iter;
    Iter = data.begin();                                  //用迭代器处理

    int i=0;
    for(;i<pos;i++){
        Iter++;
    }
    
    if(data[i]=="NONE"){ 
        data.erase(Iter);
    }
    else{
        data.erase(data.end());
        if(data.size()==maxsize+1){
            data.insert(Iter,record);
        }
    }
    data.insert(Iter,record);
}

//储存数据

void DataFile::store(){
    string dirName2 = "data";
    bool flag2 = CreateDirectory(dirName2.data(), NULL);            //将原文件清空后用新数据重建
    ofstream outfile("data/"+name);
    outfile.clear();
    for(int i=0;i<data.size();i++){
        outfile<<data[i];
        outfile<<"\n";
    }
    outfile.close();
}

//删除文件

void DataFile::delete_file(){
    string path="data/"+name;
    const char *savePath = path.data();        //判断文件大小，为0则删除
    if(get_num()==0){
        remove(savePath);
    }
}

#endif // !_DataFile_