#include<iostream>
#include<map>
#include<sstream>

using namespace std;

#include"DataBase.h"        
#include"DataFile.h"
#include"Cache.h"
#include"IndexFile.h"

//define order type

enum order                       
{
    OPEN,CLOSE,insert,Delete,change,one_find,range_find,wrong
}; 

//change input to order

order get_order(string s){   
    if(s=="open"){
        return OPEN;
    }
    else if(s=="close"){
        return CLOSE;
    }
    else if(s=="insert"){
        return insert;
    }
    else if(s=="delete"){
        return Delete;
    }
    else if(s=="change"){
        return change;
    }
    else if(s=="find"){
        return one_find;
    }
    else if(s=="rangefind"){
        return range_find;
    }
    else{
        return wrong;
    }
}

void Do(order order,DataBase &DB,string &mes);

void print(string &mes){
    cout<<mes<<endl;;
}

int main(){

    //note

    cout<<"Welcome to my DataBase program!"<<endl;
    cout<<"You can do these:"<<endl;
    cout<<"1.input open to open a DataBase"<<endl;
    cout<<"2.input close to close a DataBase"<<endl;
    cout<<"3.input insert to insert a record to DataBase"<<endl;
    cout<<"4.input delete to delete a record"<<endl;
    cout<<"5.input change to change a record"<<endl;
    cout<<"6.input find to find a record"<<endl;
    cout<<"7.input rangefind to find many records"<<endl;
    cout<<"Note:If you input a wrong order, you need to input again"<<endl;
    cout<<endl;
    DataBase DB=DataBase();

    while(true){
        cout<<"Please input your order:"<<endl;
        string s;
        cin>>s;
        order O=get_order(s);
        string mes;
        Do(O,DB,mes);

        print(mes);
        cout<<endl;

    }
}

//get key

bool get_key(unsigned long long &key){
    cout<<"Please input your key(Positive integer):"<<endl;
    // unsigned long long key;
    cin>>key;
    if(cin.fail()){
        // mes="wrong key";
        return false;
    }
    return true;
}

//get record

void get_record(string &record){
    cout<<"Please input your record:"<<endl;
    // string rec;
    cin>>record;
}

//operation

void Do(order order,DataBase &DB,string &mes){
    if(order==OPEN){
        cout<<"Please input file name"<<endl;
        string name;
        cin>>name;
        DB.open(name);
        DB.buffer_store();
        mes= "success";
    }
    else if(order==CLOSE){
        DB.close();
    }
    else if(order==insert){
        if(DB.get_leaf_num()==0){
            cout<<"You create a new DataBase!"<<endl;
        }

        unsigned long long key;
        if(!get_key(key)){
            mes="wrong key";
            return;
        }
        string rec;
        get_record(rec);
        // cin>>rec;

        bool ok=DB.Insert(key,rec);
        if(ok){
            mes="success";
        }
        else{
            mes="we have this key";
        }
    }
    else if(order==Delete){
        // cout<<"Please input your key(Positive integer):"<<endl;
        unsigned long long key;

        if(!get_key(key)){
            mes="wrong key";
            return;
        }
        bool ok=DB.delete_one(key);
        if(ok){
            mes="success";
        }
        else{
            mes="we don't have this key";
        }

    }

    else if(order ==change){
        unsigned long long key;
        if(!get_key(key)){
            mes="wrong key";
            return;
        }
        string rec;
        get_record(rec);
        bool ok=DB.change(key,rec);
        if(ok){
            mes="success";
        }
        else{
            mes="we don't have this key";
        }
    }
    else if(order==one_find){
        unsigned long long key;
        if(!get_key(key)){
            mes="wrong key";
            return;
        }
        string back=DB.Find(key);
        if(back=="NONE"){
            mes="we don't have this key";
        }
        else{
            mes=back;
        }
    }
    else if(order==range_find){
        unsigned long long key1;
        if(!get_key(key1)){
            mes="wrong key";
            return;
        }
        unsigned long long key2;
        if(!get_key(key2)){
            mes="wrong key";
            return;
        }

        map<unsigned long long,string> back=DB.RangeFind(key1,key2);
        if(back.size()==0){
            mes="we find nothing";
        }
        else{
            map<unsigned long long,string>::iterator it;
            for(it=back.begin();it!=back.end();it++){
                stringstream ss;
                ss<<it->first;
                string s_key;
                ss>>s_key;
                mes+=s_key;
                mes+=" ";
                mes+=it->second;
                mes+="\n";
            }
        }
    }
    else{
        mes="wrong order";
    }

}