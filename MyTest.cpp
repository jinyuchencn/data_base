#include<iostream>
#include<vector>
#include<fstream>
#include<random>
#include <sstream>
#include<map>
#include<time.h>

using namespace std;

#include "DataBase.h"
#include"IndexFile.h"
#include"Cache.h"


vector<string> get_ar(string filename){
    vector<string> back;
    ifstream infile(filename);
    int i=1;
    while(!infile.eof()){
        string s_value;
        string b;
        infile>>s_value;
        b+=s_value;
        while(s_value!="00:00:00"){
            infile>>s_value;
            b+=s_value;
        }
        b+=s_value;
        back.push_back(b);
        if(i%100000==0){
            cout<<"I'm working"<<endl;
        }
        if(i==50000){
            break;
        }
        i++;
    }
    return back;
}

vector<unsigned long long> get_key(int size){
    vector<unsigned long long> back;
    for(int i=1;i<size+1;i++){
        back.push_back(i);
    }
    return back;
}

int main(int argc, char **argv)
{
    clock_t start,finish;
    double totaltime;

    DataBase tree = DataBase();
    string filename="transfer.txt";
    vector<string>  ar2=get_ar(filename);
    vector<unsigned long long> ar1=get_key(ar2.size());

    cout<<ar1.size()<<endl;

    int n = ar1.size() - 1;
    string dataname=tree.produce_dataname();
    IndexFile root;
    root.makeroot(ar1[0],dataname,&tree);
    root.store();
    Cache buffer=tree.get_buffer();
    DataFile Data1=DataFile(dataname);
    Data1.insert(ar2[0],root.find(ar1[0],buffer).pos);
    tree.change_Cache(buffer);
    Data1.store();
    tree.set_root(root);
    tree.set_first();
    tree.change_Leaf_file_num(1);
    
    cout<<"First:insert 50000 records(key from 1 to 50000)"<<endl;
    system("pause");

    start=clock();
    for (int i = 1; i < n+1; ++i)
    {
        tree.Insert(ar1[i],ar2[i]);
        if(i%10000==0){
            cout<<i<<" ";
            cout<<"I'm working"<<endl;
        }
        if(tree.get_buffer_size()>100){
                tree.buffer_store();
                tree.clear_buffer();
        }
    }

    tree.set_first();
    tree.buffer_store();
    tree.clear_buffer();
    cout<<"Finish insert!"<<endl;
    finish=clock();
    totaltime=(double)(finish-start)/CLOCKS_PER_SEC;
    cout<<"time:"<<totaltime<<"s!"<<endl;
    IndexFile newroot=IndexFile(tree.get_root()->get_name());
    cout<<"root_name:"<<newroot.get_name()<<endl;

    cout<<"Second: delete 1000 records(key from 10000 to 11000) "<<endl;
    system("pause");

    start=clock();
    for(int i=10000;i<10000+1000;i++){
        tree.delete_one(i);
        if(i%200==0){
            cout<<i<<" ";
            cout<<"I'm working"<<endl;
        }
        if(tree.get_buffer_size()>100){
            tree.buffer_store();
            tree.clear_buffer();
        }
    }
    tree.set_first();
    tree.buffer_store();
    tree.clear_buffer();
    cout<<"Finish delete!"<<endl;
    finish=clock();
    totaltime=(double)(finish-start)/CLOCKS_PER_SEC;
    cout<<"time:"<<totaltime<<"s!"<<endl;
    newroot=IndexFile(tree.get_root()->get_name());
    cout<<"root_name:"<<newroot.get_name()<<endl;

    cout<<"Third: change 1000 records(key from 30000 to 31000 record:test_change)"<<endl;
    system("pause");

    start=clock();
    string s="test_change";
    for(int i=30000;i<30000+1000;i++){
        tree.change(i,s);
        if(i%200==0){
            cout<<i<<" ";
            cout<<"I'm working"<<endl;
        }
        if(tree.get_buffer_size()>100){
            tree.buffer_store();
            tree.clear_buffer();
        }
    }
    tree.set_first();
    tree.buffer_store();
    tree.clear_buffer();
    cout<<"Finish change!"<<endl;
    finish=clock();
    totaltime=(double)(finish-start)/CLOCKS_PER_SEC;
    cout<<"time:"<<totaltime<<"s!"<<endl;
    newroot=IndexFile(tree.get_root()->get_name());
    cout<<"root_name:"<<newroot.get_name()<<endl;

    cout<<"Fourth: RangeFind 1000 records(key from 30500 to 31500)"<<endl;
    system("pause");

    start=clock();
    map<unsigned long long,string>  back=tree.RangeFind(30500,31500);
    map<unsigned long long,string>::iterator it;
    for(it=back.begin();it!=back.end();it++){
        cout<<it->first<<" "<<it->second<<endl;
    }
    tree.set_first();
    tree.buffer_store();
    tree.clear_buffer();
    cout<<"Finish RangeFind!"<<endl;
    finish=clock();
    totaltime=(double)(finish-start)/CLOCKS_PER_SEC;
    cout<<"time:"<<totaltime<<"s!"<<endl;
    newroot=IndexFile(tree.get_root()->get_name());
    cout<<"root_name:"<<newroot.get_name()<<endl;

    cout<<"Fifth: Find 10 Records"<<endl;
    system("pause");
    for(int i=0;i<10;i++){
        string record;
        unsigned long long key;
        cout<<"Please input key:";
        cin>>key;
        record=tree.Find(key);
        cout<<"the record is:"<<record<<endl;
    }

    newroot=IndexFile(tree.get_root()->get_name());
    cout<<"root_name:"<<newroot.get_name()<<endl;
    system("pause");
    


    return 0;
}