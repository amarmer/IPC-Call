## IPC-Call C++ framework for IPC call

The framework allows calling a C++ server function from a C++ client in the same way as it is called locally.

### Client: 

1. The framework utilizes an IPC transport function (for instance, implemented via TCP/IP, HTTP(S), etc.), <br/>however, the framework does not implement it.
   The function sends to and receives from the server ```std::vector<uint8>```.<br/>
   Its declaration - `std::vector<uint8> Ipc(const std::vector<uint8>& bytes);`

2. Function declaration that is called on the server is the same as a declaration of a local function -<br/> `Ret f(Par1 par1, Par2 par2, ... ParN parN)`, where all parameters can be `In` or `InOut`.
   
4. Function call - `Ret res =  IPC_CALL(f)(arg1, arg2, ...argN)(Ipc)`, <br/>where `Ipc` is IPC transport function described above.<br/>

#### For instance, a function declaration and call:
#### Declaration:
`std::list<int> XYZ(const std::map<std::string, int>& in, std::vector<std::tuple<std::string, int>>& inOut);`
<br/>
#### Call: 

```
std::vector<std::tuple<std::string, int>> inOut = { {"A", 1}, {"B", 2} };
std::list<int> res = IPC_CALL(XYZ)({ {"C", 3}, {"D", 4} }, inOut)(Ipc);
```
For a comparison, the local call would look like:<br/>
` std::list<int> res = XYZ({ {"C", 3}, {"D", 4} }, inOut);`
<br/><br/>

### Server: 
1. On the server, when `bytes` (parameter of the IPC transport function `Ipc` described above) is received from the client, `IpcCall::Server::Call(bytes)` should be called and its return (`std::vector<uint8_t>`) should be sent back to the client.<br/><br/> 
2. Every function should be registered via macro `IPC_CALL_REGISTER(f)`.<br/>

#### For instance, the implementation and registration of the function declared in the client example above:
```
std::list<int> XYZ(const std::map<std::string, int>& in,
                   std::vector<std::tuple<std::string, int>>& inOut) {
    std::list<int> ret;

    for (const auto& [key, value]: in) {
        inOut.emplace_back(std::make_tuple(key, value));

        ret.emplace_back(value);
    }

    return ret;
}
IPC_CALL_REGISTER(XYZ);
```

The framework supports most of STL data structures in [IpcCallData.h](https://github.com/amarmer/IPC-Call/blob/main/IpcCallData.h) and can be extended with custom data.

An example of client and server is in [main.cpp](https://github.com/amarmer/IPC-Call/blob/main/Main.cpp)

The framework can be tested on https://wandbox.org/permlink/S9i4utDMTIJTjIaD 

