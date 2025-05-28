## IPC-Call C++ framework for IPC call

The framework allows calling a C++ server function from a C++ client in the same way as it is called locally.

### Client: 

1. The framework uses an IPC transport function (for instance TCP/IP, HTTP(S), etc., the framework does not implement it).<br/>
   The function sends to and receives from the server ```std::vector<uint8>```.<br/>
   Its declaration - `std::vector<uint8> Ipc(const std::vector<uint8>& bytes);`

2. Function declaration that is called on the server - `Ret f(Par1 par1, Par2 par2, ... ParN parN)`, <br/>where all parameters can be `In` or `InOut`
   (the same as a declaration of a local function).
   
4. Function call - `Ret res =  IPC_CALL(f)(arg1, arg2, ...argN)(Ipc)`, <br/>where `Ipc` is IPC transport function described above.<br/>

#### For instance, a function declaration and call:
#### Declaration:
`std::list<Data> XYZ(const std::map<std::string, int>& in, std::vector<std::tuple<std::string, int>>& inOut);`
<br/>
#### Call: 

```
std::vector<std::tuple<std::string, int>> inOut = { {"A", 1}, {"B", 2} };
std::list<Data> res = IPC_CALL(XYZ)({ {"C", 3}, {"D", 4} }, inOut)(Ipc);
```
For comparison, the local call would look like:<br/>
` std::list<Data> res = XYZ({ {"C", 3}, {"D", 4} }, inOut);`
<br/><br/>

### Server: 
1. On the server, when `bytes` (parameter of the `Ipc` described above) is received from the client, <br/>`IpcCall::Server::Call(bytes)` should be called (implemented in `IpcCallServer.h`).<br/><br/> 
2. Every function should be registered via macro `IPC_CALL_REGISTER(f)`.<br/>

#### For instance, the implementation and registration of the function declared in the client example above:
```
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
```


