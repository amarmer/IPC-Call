#include <iostream>
#include <cassert>

#include "IpcCallClient.h"
#include "IpcCallServer.h"

// Example of a custom struct 'Data' and its implementation of serialization and unserialization.
struct Data {
    bool operator == (const Data& data) const { 
        return str_ == data.str_ && n_ == data.n_;
    }

    std::string str_;
    int n_;
};

// 'Data' serialization and unserialization.
namespace IpcCall {
    Serializer& operator << (Serializer& serializer, const Data& data) {
        return serializer << data.str_ << data.n_;
    }

    Unserializer& operator >> (Unserializer& unserializer, Data& data) {
        return unserializer >> data.str_ >> data.n_;
    }
}


//
// Client
//

// 'ABC' declaration.
void ABC();

// 'XYZ' declaration.
std::list<Data> XYZ(const std::map<std::string, int>& in, std::vector<std::tuple<std::string, int>>& inOut);

//
// IPC transport needs to implement 'Ipc' function.
// 'bytes' is data that is sent to the server, 'return' is data that is received from the server. 
//
// The function needs to be passed as the last argument of `IPC_CALL`.
std::vector<uint8_t> Ipc(const std::vector<uint8_t>& bytes) noexcept(false) {
    // IPC transport on the server needs to call 'IpcCall::Server::Call(bytes)' (implememented in IpcCallServer.h), 
    // and the return of the function needs to be sent back to the client.
    //
    // For testing, call the server directly.
    return IpcCall::Server::Call(bytes);
}

static bool s_abcCalled;

int main(int, char**) {
    // Test 'ABC'
    IPC_CALL(ABC)()(Ipc);
    assert(s_abcCalled);

    // Test 'XYZ'
    std::vector<std::tuple<std::string, int>> inOut = { {"A", 1}, {"B", 2} };
    std::list<Data> res = IPC_CALL(XYZ)({ {"C", 3}, {"D", 4} }, inOut)(Ipc);

    assert((inOut == decltype(inOut){ {"A", 1}, {"B", 2}, {"C", 3}, {"D", 4} }));
    assert((res == decltype(res){ {"C", 3}, {"D", 4} }));

    std::cout << "!!!\n";
}


//
// Server
//

// 'ABC' implementation.
void ABC() {
    s_abcCalled = true;
}
IPC_CALL_REGISTER(ABC);

// 'XYZ' implementation.
// 'in' data is added to 'inOut' and 'in' data is copied to the return.
std::list<Data> XYZ(const std::map<std::string, int>& in, std::vector<std::tuple<std::string, int>>& inOut) {
    std::list<Data> ret;

    for (const auto& [key, value] : in) {
        inOut.emplace_back(std::make_tuple(key, value));

        ret.push_back({ key, value });
    }

    return ret;
}
IPC_CALL_REGISTER(XYZ);
