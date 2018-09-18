#ifndef _IndexFile_
#define _IndexFile_


#include<iostream>
#include<vector>
#include<sstream>
#include<fstream>
#include<direct.h>
#include<string>

using namespace std;

#include"DataBase.h"
#include"DataFile.h"
#include"Cache.h"

//设置树的节点大小

const int maxsize = 127;                      //定义b+树的阶数
const int maxelem = maxsize-1;              //非叶节点元素的最大数
const int minsize = maxsize / 2;            //非叶节点元素最小个数
const int leafmax = maxsize;                //叶子节点最大元素数
const int leafmin = maxsize / 2 + 1;        //叶子节点最小元素数

class IndexFile;

//构造函数

IndexFile::IndexFile(){
    name="NONE";
    type=NONE;
    father="NONE";

    for(int i =0;i<maxsize+1;i++){
        children.push_back("NONE");
        key.push_back(0);
        data_file.push_back("NONE");
    }

    front=next="NONE";
    num=0;
}
IndexFile::IndexFile(string new_name){
    name="NONE";
    type=NONE;
    father="NONE";
    for(int i =0;i<maxsize+1;i++){
        children.push_back("NONE");
        key.push_back(0);
        data_file.push_back("NONE");
    }
    front=next="NONE";
    num=0;    
    name=new_name;
    string path="index/"+name;

    fstream file(path);
    if(!file) return;
    file.seekp(0,ios::beg);
    if(file.eof()){
        file.close();
        return;
    }

    else{
        vector<string> data;

        while(!file.eof()){
            string s;
            file>>s;
            data.push_back(s);                 
        }

        if(data[0]=="Brch"){
            type=Brch;
            father=data[1];
            front=data[2];
            next=data[3];
            stringstream ss;
            ss<<data[4];
            ss>>num;
            for(int i=0;i<maxsize+1;i++){
                stringstream ss;
                ss<<data[i+5];
                ss>>key[i];
            }
            for(int i=0;i<maxsize+1;i++){
                children[i]=data[i+5+maxsize+1];
            }
        }

        else{
            type=Leaf;
            father=data[1];
            front=data[2];
            next=data[3];
            stringstream ss;
            ss<<data[4];
            ss>>num;
            for(int i=0;i<maxsize;i++){
                stringstream ss;
                ss<<data[i+5];
                ss>>key[i];
            }
            for(int i=0;i<maxsize;i++){
                data_file[i]=data[i+5+maxsize+1];
            }
        }  
        file.close();    
    }       
}

//计算叶子数

int IndexFile::get_leaf_num(){
    int n=0;
    IndexFile first=this->getfirst();   //先到链表头，然后往后读
    if(first.name=="NONE"){
        return 0;
    }
    n++;
    while(first.next!="NONE"){
        first=IndexFile(first.next);
        n++;
    }
    return n;
}

//计算内节点数

int IndexFile::get_Brch_num(){           
    int n=0;
    n++;
    n+=(this->num+1);
    return n;
}

//获得链表头节点

IndexFile IndexFile::getfirst(){
    IndexFile first;
    IndexFile First=*this;

    while (First.get_name() != "NONE" && First.get_type() != Leaf)
    {
        First = First.get_children()[0];
    }

    if (First.get_type() == Leaf){
        first=First;
    }

    return first;
}

//复制节点

void IndexFile::copy(IndexFile one){
    name=one.name;
    type=one.type;
    father=one.father;
    children=one.children;
    key=one.key;
    data_file=one.data_file;
    front=one.front;
    next=one.next;
    num=one.num;
}

//移动叶子数据

void IndexFile::move_leaf(IndexFile &des, int pos,string newdatafile,Cache &buffer){
    int i = pos + 1;                             //根据pos移动，并将信息存入缓存

    for (int j = 0; i < num; j++, i++)
    {
        des.key[j] = key[i];
        des.data_file[j] = newdatafile;          
        data_file[i] = "NONE";
        key[i]=0;
    }

    des.num = num = leafmin;
    buffer.Add_Index(name,*this);
    buffer.Add_Index(des.name,des);
}

//移动数据

void IndexFile::move_leaf_data(string name1,string name2,int pos,Cache &buffer){
    int i = pos + 1;
    DataFile D1=buffer.make_Data(name1);
    DataFile D2=buffer.make_Data(name2);
    int num=maxsize;

    for (int j = 0; i < num+1; j++, i++)
    {
        D2.change_data(D1.get_data()[i],j);
        D1.change_data("NONE",i);
    }

    D2.change_num(leafmin);
    D1.change_num(leafmin);

    buffer.Add_Data(D1.get_name(),D1);
    buffer.Add_Data(D2.get_name(),D2);

}

//移动内节点数据

void IndexFile::move_brch(IndexFile &des, int pos,Cache &buffer){
    int i = pos + 1;

    for (int j = 0; i <= num; j++, i++)
    {
        des.key[j] = key[i];
        key[i]=0;
        des.children[j] = children[i];
        IndexFile Child;
        Child=buffer.make_Index(des.children[j]);        
        Child.father=des.name;
        children[i] = "NONE";
        buffer.Add_Index(Child.name,Child);
    }

    des.num = num = minsize;
    buffer.Add_Index(name,*this);
    buffer.Add_Index(des.name,des);
}

//插入内节点

void IndexFile::insert_brch(int pos, unsigned long long new_key, IndexFile &right,Cache &buffer){
    int i = ++num;

    for (; i - 1 > pos; --i)
    {
        key[i] = key[i - 1];
        children[i] = children[i - 1];
    }

    key[i] = new_key;
    children[i] = right.name;

    if (right.name != "NONE")                       //判断是否有兄弟
    {
        right.father = name;
        buffer.Add_Index(name,*this);
        buffer.Add_Index(right.name,right);
    }

    buffer.Add_Index(name,*this);
    buffer.Add_Index(right.name,right);
}

//调整树（插入），在节点key数大于maxsize时使用

IndexFile IndexFile::adjust(DataBase *DataBase,Cache &buffer){
    IndexFile par;
    string datafile=children[0];
    string new_datafile;

    if(father!="NONE"){                            //判断是否有父节点，有则找父节点，没有就造
        par = buffer.make_Index(father);
    } 
    else{
        par=IndexFile();                          
    }
    IndexFile newnode = IndexFile();

    if (type == Leaf)                               //判断节点类型并新建节点     
    {
        new_datafile=DataBase->produce_dataname();
        newnode = DataBase->makeleaf();
        newnode.next = next;
        next = newnode.name;
        newnode.front = name;
        if (newnode.next != "NONE")
            buffer.make_Index(newnode.next).front = newnode.name;
    }
    else{
        newnode = DataBase->makebrch();
        new_datafile=DataBase->produce_dataname();
    }

    if (newnode.type == Leaf)                        //判断节点类型并操作节点
    {
        this->move_leaf(newnode, leafmin - 1,new_datafile,buffer);
        int i=0;
        while(data_file[i]=="NONE"){
            i++;
        }
        string source=data_file[i];
        this->move_leaf_data(source,new_datafile,leafmin-1,buffer);
    }
    else{
        this->move_brch( newnode, minsize,buffer);
    }

    if (par.name == "NONE")                       //判断父节点，无则对新节点调整数据，有则对原节点调整数据
    {
        IndexFile newroot = DataBase->makebrch();
        newroot.children[0] = name;
        newroot.children[1] = newnode.name;
        newroot.key[1] = newnode.key[0];
        newnode.father = newroot.name;
        father = newroot.name;
        newroot.num++;
        buffer.Add_Index(this->name,*this);
        buffer.Add_Index(newroot.name,newroot);
        buffer.Add_Index(newnode.name,newnode);
        return newroot;
    }
    else
    {
        int pos = par.num;
        for (; pos > 0 && name != par.children[pos]; --pos) {}

        par.insert_brch( pos, newnode.key[0], newnode,buffer);
        buffer.Add_Index(this->name,*this);
        buffer.Add_Index(newnode.name,newnode);

        if (par.num > maxelem)
        {
            return par.adjust(DataBase,buffer);             //若父节点过大则继续调整
        }
    }

    IndexFile wrong=IndexFile();                  
    return wrong;
}

//查找key

result IndexFile::find(unsigned long long key,Cache &buffer){
    IndexFile tmep = buffer.make_Index(this->name);                                      
    result res = { "NONE",0,false };

    if (name == "NONE"){
        return res;
    }

    while (tmep.get_name() != "NONE" && tmep.get_type() != Leaf)       //找到节点
    {
        int i = tmep.get_num();                          
        for (; i > 0 && key < tmep.get_key()[i]; i--) {}
        res.filename = tmep.get_name();
        res.pos = i;
        tmep = buffer.make_Index(tmep.get_children()[i]);
    }

    if (tmep.name != "NONE")                            //找到数据
    {
        int i = tmep.get_num() - 1;
        for (; i >= 0 && key < tmep.get_key()[i]; i--) {}
        res.filename = tmep.get_name();
        res.pos = i;
        if (tmep.get_key()[i] == key)
        {
            res.ok = true;
        }
    }

    return res;
};

//设置根节点

void IndexFile::makeroot(unsigned long long new_key,string data_file_name,DataBase *DataBase){
    *this=DataBase->makeleaf();
    key[0]=new_key;
    data_file[0]=data_file_name;
    num=1;
    type=Leaf;
}

//插入叶子节点

void IndexFile::insert_leaf( int pos, unsigned long long new_key, string record,Cache &buffer)    //插入叶子
{
    int i = this->get_num();

    for (; i - 1 > pos; --i)
    {
        key[i] = key[i - 1];
        data_file[i] = data_file[i - 1];
    }

    key[i] = new_key;
    data_file[i] = record;
    num++;
    buffer.Add_Index(this->name,*this);
}

//删除内节点

void IndexFile::dele_brch(int pos){
    int i = pos;

    for (; i < num; ++i)
    {
        key[i] = key[i + 1];
        children[i] = children[i + 1];
    }

    --num;
}

//合并内节点

void IndexFile::merge_lebrch( IndexFile &ptr, IndexFile &par, int parpos,Cache &buffer){
    int i = ++num;
    ptr.key[0] = par.key[parpos];

    for (int j = 0; j < ptr.num; ++j, ++i)
    {
        key[i] = ptr.key[j];
        children[i] = ptr.children[j];
        buffer.make_Index(ptr.children[j]).father = name;
    }
    num += ptr.num;
    par.dele_brch(parpos);
    ptr = *this;
}

//合并叶子节点

void IndexFile::merge_leaf(IndexFile &des,  IndexFile &par, int parpos){
    int i = des.num;
    for (int j = 0; j < num; ++j, ++i)
    {
        des.key[i] = key[j];
        des.data_file[i] = data_file[j];
    }
    des.num += num;
    par.dele_brch(parpos);
    *this = des;
}

//删除叶子节点

void IndexFile::dele_leaf( int pos, int parpos,Cache &buffer)
{
    DataFile a=buffer.make_Data(data_file[pos]);

    for (int i = pos; i < num; ++i)
    {
        a.change_data(a.get_data()[i+1],i);
        key[i] = key[i+1];
        data_file[i] = data_file[i+1];
    }

    if (pos == 0)
    {
        if(father!="NONE"){
            buffer.make_Index(father).key[parpos] = key[0];
        }

    }

    --num;

    buffer.Add_Data(a.get_name(),a);
    buffer.Add_Index(this->name,*this);
    a.delete_file();
}

//调整内节点

IndexFile IndexFile::adjust_brch(Cache &buffer){
    if (name == "NONE") return *this;                     //没东西，直接返回

    IndexFile par = buffer.make_Index(father);            //找父节点

    if (par.name != "NONE")                               //有父，找兄弟，判断节点过小后能否合并
    {
        int parpos = par.num;
        for (; parpos >= 0 && par.children[parpos] != name; --parpos) {}

        IndexFile left = buffer.make_Index (par.children[parpos - 1]);
        IndexFile right =buffer.make_Index (par.children[parpos + 1]);

        if (left.name != "NONE" && left.num > minsize)
        {
            int i = ++num;
            key[0] = par.key[parpos];

            for (; i > 0; i--)
            {
                key[i] = key[i - 1];
                children[i] = children[i - 1];
            }

            children[0] = left.children[left.num];
            par.key[parpos] = left.key[left.num];
            left.num--;
        }
        else if (right.name != "NONE" && right.num > minsize)
        {
            IndexFile ptnode = *this;
            ptnode.key[++ptnode.num] = par.key[parpos + 1];
            ptnode.children[ptnode.num] = right.children[0];
            buffer.make_Index(right.children[0]).father = name;

            for (int i = 0; i < right.num; i--)
            {
                right.key[i] = right.key[i + 1];
                right.children[i] = right.children[i + 1];
            }
            par.key[parpos + 1] = right.key[0];
            --right.num;
        }
        else if (left.name != "NONE")
        {
            left.merge_lebrch( *this, par, parpos,buffer);
        }
        else if (right.name != "NONE")
        {
            this->merge_lebrch( right, par, parpos + 1,buffer);
        }

        if (par.father == "NONE" && par.num == 0)
        {
            return *this;
        }
        else if (par.father != "NONE" && par.num < minsize)
        {
            return par.adjust_brch(buffer);
        }
    }

    else                                                   //无父，直接改
    {
        if (num == 0)
        {
            IndexFile temp = buffer.make_Index(children[0]);
            return temp;
        }
    }
    IndexFile wrong=IndexFile();
    return wrong;
}

//调整树

IndexFile IndexFile::adjust_tree(Cache &buffer){
    IndexFile left = buffer.make_Index(front);        //左右兄弟，父节点
    IndexFile right = buffer.make_Index(next);
    IndexFile pare = buffer.make_Index(father);

    int parpos = buffer.make_Index(father).num;         //找到自己的位置
    for (; parpos >= 0 && buffer.make_Index(father).children[parpos] != name; --parpos) {}    

    if (left.name != "NONE" &&left.num > leafmin)         //判断左右兄弟，能否合并
    {
        int i = ++num;
        for (; i > 0; i--)
        {
            key[i] = key[i - 1];
            data_file[i] = data_file[i - 1];
        }
        left.num--;
        key[0] = left.key[left.num];
        buffer.make_Index(father).key[parpos] = key[0];
        data_file[0] = left.data_file[left.num];

    }
    else if (right.name != "NONE" && right.num > leafmin)
    {
        key[num] = right.key[0];
        data_file[num] = right.data_file[0];
        ++num;
        --right.num;
        for (int i = 0; i < right.num; i--)
        {
            right.key[i] = right.key[i + 1];
            right.data_file[i] = right.data_file[i + 1];
        }
        buffer.make_Index(father).key[parpos + 1] = right.key[0];
    }
    else if (left.name != "NONE")
    {
        IndexFile parent=buffer.make_Index(father);
        left.merge_leaf( *this, parent, parpos);
    }
    else if (right.name != "NONE")
    {
        IndexFile parent=buffer.make_Index(father);
        this->merge_leaf( right, parent, parpos + 1);
    }
    if (pare.num < minsize)
    {
        return pare.adjust_brch(buffer);
    }
    else
    {
        IndexFile wrong=IndexFile();
        return wrong;
    }
}

//删除特定key

bool IndexFile::dele_tree( unsigned long long key,DataBase &DataBase,Cache &buffer){
    if (name == "NONE")return false;
    result res = this->find( key,buffer);
    IndexFile ptnode = buffer.make_Index(res.filename);
    int pos = res.pos;

    if (res.ok == false)
        return false;

    int parpos = buffer.make_Index(ptnode.father).num;

    for (; parpos >= 0 && buffer.make_Index(ptnode.father).children[parpos] != ptnode.name; --parpos) {}  //找位置
    ptnode.dele_leaf( pos, parpos,buffer);
    

    if (ptnode.num < leafmin)
    {
        if (ptnode.father == "NONE" && ptnode.num == 0)                             //整个树只有一个节点时，又恰好被删掉唯一的数据
        {
            this->delete_file();
            name = "NONE";
        }
        else if (ptnode.father != "NONE" && buffer.make_Index(ptnode.father).num < minsize)     //不平衡，调整分支
        {
            IndexFile temp = ptnode.adjust_tree(buffer);
            if (temp.name != "NONE")
                *this = temp;
        }
    }
    
    return true;

}

//打印树

void IndexFile::show(){
    IndexFile first;
    first=this->getfirst();

    while (first.name != "NONE")
    {
        unsigned long long s;
        for (int i = 0; i < first.num; ++i)
        {
            s = first.key[i];
            if(s!=0){
                cout<<s;
                cout<<" ";
                string DataName=first.data_file[i];
                DataFile *a= new DataFile(DataName);

                cout<<a->get_data()[i];
                delete a;
                cout<<endl;
            }
        }
        if(first.next=="NONE"){
            first.name="NONE";
        }
        else{
            first = IndexFile(first.next);
        }
    }

    std::cout<<std::endl;
}

//插入数据

bool IndexFile::Insert(unsigned long long key,string &record,DataBase *DataBase,Cache &buffer){
    IndexFile root;

    if (name == "NONE")                                       //新建节点
    {
        string name=DataBase->produce_dataname();
        DataFile Data=DataFile(name);
        Data.insert(record,0);
        this->makeroot(key, name,DataBase);
        buffer.Add_Index(this->name,*this);
        buffer.Add_Data(Data.get_name(),Data);
        return true;
    }

    result res = this->find(key,buffer);                     //找位置
    
    if (res.ok == true) {                                     //已找到，插入失败
        return false;
    }

    else
    {
        IndexFile pnode = buffer.make_Index(res.filename);       //未找到，调节点，找数据文件
        int pos = res.pos;
        string name;

        if(pos==-1){
            if(pnode.get_data_file()[pos+1]=="NONE"){
                string name=DataBase->produce_dataname();
            }
            else{
                name=pnode.data_file[pos+1];
            }
        }

        else if(pnode.data_file[pos]=="NONE"){
            string name=DataBase->produce_dataname();
        }
        else{
            name=pnode.data_file[pos];
        }


        pnode.insert_leaf( pos, key, name,buffer);


        DataFile leaf=buffer.make_Data(name);                    //调数据文件
        leaf.insert(record,res.pos+1); 
        buffer.Add_Data(leaf.get_name(),leaf);

        if (pnode.num > leafmax)                                  //树不平衡则调树
        {
            IndexFile temp = pnode.adjust(DataBase,buffer);
            if (temp.name !="NONE"){
                IndexFile a=buffer.make_Index(temp.name);
                this->copy(a);
            }
        }

        IndexFile ptr=buffer.make_Index(this->name);    

        this->copy(ptr);                                    //改变根节点

        return true;
    }

}

//储存数据

void IndexFile::store(){
    string dirName1 = "index";
    mkdir(dirName1.c_str());
    ofstream File("index/"+name);
    if(type==Leaf){
        File<<"Leaf"<<" ";
        File<<father<<" ";
        File<<front<<" ";
        File<<next<<" ";
        File<<num<<" ";
        for(int i=0;i<maxsize+1 ; i++){
            File<<key[i]<<" ";
        }
        for(int i=0;i<maxsize+1 ; i++){
            File<<data_file[i]<<" ";
        }
        File.close();
    }
    else{
        File<<"Brch"<<" ";
        File<<father<<" ";
        File<<front<<" ";
        File<<next<<" ";
        File<<num<<" ";
        for(int i=0;i<maxsize+1 ; i++){
            File<<key[i]<<" ";
        }
        for(int i=0;i<maxsize+1 ; i++){
            File<<children[i]<<" ";
        }
        File.close();
    }
}

//删除文件

void IndexFile::delete_file(){
    string path="index/"+name;
    const char *savePath = path.data();
    if(father=="NONE"&&num==0){
        remove(savePath);
    }
}

#endif // !IndexFile
