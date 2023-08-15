#include "test.pb.h"
#include<iostream>
#include<string>
using namespace fixbug;
using namespace std;
int main()
{
    /*
        // 序列化
        LoginRequest req;
        req.set_name("zs");
        req.set_pwd("123");
        string rs;
        if(req.SerializeToString(&rs)) 
        {
            cout<<rs<<endl;
        }

        // 反序列化
        LoginRequest reqB;
        if(req.ParseFromString(rs))
        {
            cout<<req.name()<<endl<<req.pwd()<<endl;
        }
    */

    // LoginResponse rsp;
    // ResultCode*ptr = rsp.mutable_rs(); // 类内成员 使用mutable xx 获取指针
    // ptr->set_errmsg("login wrong");
    // ptr->set_errno("1");

    GetFriendListsResponse rsp;
    ResultCode*rc = rsp.mutable_rs();
    rc->set_errmsg("Wrong");
    rc->set_errnum("1");
    // user1
    auto ptr =rsp.add_friend_list(); // ptr 是user*
    ptr->set_age(1);
    ptr->set_name("abc");
    ptr->set_sex(User::MAN);

    // user2
    auto p =rsp.add_friend_list(); // ptr 是user*
    p->set_age(2);
    p->set_name("ddd");
    p->set_sex(User::WOMAN);

    cout<<rsp.friend_list_size()<<endl;
    return 0;
}