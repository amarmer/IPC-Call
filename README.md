## IPC-Call C++ framework for IPC call

The framework allows calling a C++ server function from a C++ client in the same way as it is called locally.

The primary purpose of the framework is to demonstrate that a function called by the client is executed on the server.<br/> 
The IPC transport functions and their implementation are placeholders.

### Client: 

The framework utilizes an IPC transport function (for instance, implemented via TCP/IP, HTTP(S), etc.), <br/>however, the framework does not implement it.<br/>

#### Synchronous call 
The synchronous  IPC transport function sends to and receives from the server ```std::vector<uint8>```.<br/>In case of a transport error, this function should throw an exception.<br/>
Its declaration - `std::vector<uint8> IpcSync(const std::vector<uint8>& bytes) noexcept(false)`

Function declaration that is called on the server is the same as a declaration of a local function -<br/> `Ret f(Par1 par1, Par2 par2, ... ParN parN)`, where all parameters can be `In` or `InOut`.
   
Function call - `Ret res =  IPC_SEND_RECEIVE(f)(arg1, arg2, ...argN)(IpcSync)`, <br/>where `IpcSync` is IPC transport function described above.<br/>

#### Example of declaration and call:
`std::list<int> XYZ(const std::map<std::string, int>& in, std::vector<std::tuple<std::string, int>>& inOut);`<br/><br/>
`std::vector<std::tuple<std::string, int>> inOut = { {"A", 1}, {"B", 2} };`<br/>
`std::list<int> res = IPC_SEND_RECEIVE(XYZ)({ {"C", 3}, {"D", 4} }, inOut)(IpcSync);`
<br/><br/>
For a comparison, the local call would look like:<br/>
`std::list<int> res = XYZ({ {"C", 3}, {"D", 4} }, inOut);`

#### Asynchronous call 
The asynchronous  IPC transport function sends to the server ```std::vector<uint8>```.<br/>In case of a transport error, this function should throw an exception.<br/>
Its declaration - `void IpcAsync(const std::vector<uint8>& bytes) noexcept(false)`

Function declaration that is called on the server is the same as a declaration of a local function -<br/> `void f(Par1 par1, Par2 par2, ... ParN parN)`, where all parameters are `In` only.
   
Function call - `IPC_SEND(f)(arg1, arg2, ...argN)(IpcAsync)`, <br/>where `IpcAsync` is IPC transport function described above.<br/>

#### Example of declaration and call:
`void ABC(const std::string& in);`<br/><br/>
`IPC_SEND(ABC)("QAZ")(IpcAsync);`
<br/><br/>

### Server: 

#### Synchronous call 
On the server, when `bytes` (parameter of the IPC transport function `IpcSync` described above) is received from the client, `IpcCall::Server::SyncCall(bytes)` should be called and its return (`std::vector<uint8_t>`) should be sent back to the client.<br/>

#### Asynchronous call 
On the server, when `bytes` (parameter of the IPC transport function `IpcAync` described above) is received from the client, `IpcCall::Server::AsyncCall(bytes)` should be called.<br/><br/>

Every function should be registered via macro `IPC_CALL_REGISTER(f)`.<br/>

#### For instance, an implementation and registration of the function declared in the client example above:
`void ABC(const std::string& in) {...}`<br/>
`IPC_CALL_REGISTER(ABC);`<br/><br/>

The framework supports most of the STL data structures in [IpcCallData.h](https://github.com/amarmer/IPC-Call/blob/main/IpcCallData.h) and can be extended with custom data.

An example of client and server is in [main.cpp](https://github.com/amarmer/IPC-Call/blob/main/Main.cpp)

The framework can be tested on https://wandbox.org/permlink/c5puwAykpNub5TH0

