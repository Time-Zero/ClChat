#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

// 序列化实例1
string func1()
{
    json js;
    js["msg_type"] = 2;
    js["from"] = "zhang san";
    js["to"] = "li si";
    js["msg"] = "hello, what are you doing now ?";
    string sendBuf = js.dump();
    cout << sendBuf << endl;
    
    return sendBuf;
}

// 序列化实例2
string func2()
{
    json js;
    js["id"] = {1,2,3,4,5};
    js["name"]["zhang san"] = "Hello World";
    js["name"]["liu shuo"] = "Hello";
    js["msg"] = {{"zhang san", "hello world"},{"li si","hello"}};
    string sendBuf = js.dump();
    cout << sendBuf << endl; 

    return sendBuf;
}

// 序列化实例代码3
string func3()
{
    json js;
    vector<int> vec;
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);

    js["list"] = vec;
    map<int, string> m;
    m.insert({1,"黄山"});
    m.insert({2,"华山"});
    m.insert({3,"泰山"});
    js["path"] = m;
    
    cout << js << endl;
    return js.dump();
}

int main(){
    string recvBuf = func1();
    // 数据的反序列化 json字符串=>反序列化 数据对象(视为容器)
    json jsbuf = json::parse(recvBuf);
    cout << jsbuf["msg_type"] << endl;
    cout << jsbuf["from"] << endl;
    cout << jsbuf["to"] << endl;
    cout << jsbuf["msg"] << endl;
    
    recvBuf = func2();
    json jsbuf_2 = json::parse(recvBuf);
    auto arr = jsbuf_2["id"];
    cout << arr[2] << endl;
    func3();
    return 0;
}


