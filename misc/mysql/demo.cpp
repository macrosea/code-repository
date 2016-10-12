

#include <string.h>
#include <iostream>
#include <string>
using namespace std;

#include "mysqlHelper.h"
using namespace mysqlhelper;


int main(int argc,char* argv[]){


    MysqlHelper mysqlHelper;
    mysqlHelper.init("xxxx.xxx.xxx.xxxx","root","123456","StudentCourse");
    try{
        mysqlHelper.connect();
    }catch(MysqlHelper_Exception& excep){
        cout<<excep.errorInfo;
        return -1;
    }


    MysqlHelper::RECORD_DATA record;
    record.insert(make_pair("studentNo",make_pair(MysqlHelper::DB_STR,"201621031060")));
    record.insert(make_pair("name",make_pair(MysqlHelper::DB_STR,"name")));
    record.insert(make_pair("school",make_pair(MysqlHelper::DB_STR,"school")));
    record.insert(make_pair("grade",make_pair(MysqlHelper::DB_STR,"2016")));
    record.insert(make_pair("major",make_pair(MysqlHelper::DB_STR,"计算机科学与技术")));
    record.insert(make_pair("gender",make_pair(MysqlHelper::DB_INT,"1")));
       int res=0;
    try{
        res=mysqlHelper.insertRecord("student",record);
    }catch(MysqlHelper_Exception& excep){
        cout<<excep.errorInfo;
        return -1;
    }
    cout<<"res:"<<res<<" insert successfully "<<endl;

    //删除一条学生记录，学号为201421031059
    try{
        res=mysqlHelper.deleteRecord("student","where studentNo=\"201421031059\"");
    }catch(MysqlHelper_Exception& excep){
        cout<<excep.errorInfo;
        return -1;
    }
    cout<<"res:"<<res<<" delete successfully "<<endl;

    //查找学号为201421031059的学生选择的所有课程名称
    MysqlHelper::MysqlData dataSet;
    string querySQL="select courseName from course co where co.courseNo in (select courseNo from courseSelection where studentNo=\"201421031060\")";
    try{
        dataSet=mysqlHelper.queryRecord(querySQL);
    }catch(MysqlHelper_Exception& excep){
        cout<<excep.errorInfo;
        return -1;
    }
    cout<<"query successfully"<<endl;
    for(size_t i=0;i<dataSet.size();++i){
        cout<<dataSet[i]["courseName"]<<endl;
    }

    //修改学号为201421031060的学生专业
    MysqlHelper::RECORD_DATA recordChange;
    recordChange.insert(make_pair("major",make_pair(MysqlHelper::DB_STR,"软件工程")));
    try{
        res=mysqlHelper.updateRecord("student",recordChange,"where studentNo=\"201421031060\"");
    }catch(MysqlHelper_Exception& excep){
        cout<<excep.errorInfo;
        return -1;
    }
    cout<<"res:"<<res<<" update successfully"<<endl;

    return 0;
}
