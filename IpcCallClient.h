#pragma once 

#include <utility>

#include "IpcCallData.h"

namespace IpcCall {
  // 'IPC_SEND_RECEIVE' calls 'SyncCall'.
  template <typename> struct SyncCall;

  template <typename Ret, typename ...Params>
  struct SyncCall<Ret(*)(Params...)> {
    SyncCall(const std::string& funcName) : funcName_(funcName) {}

    auto operator()(Params... params) {
      if constexpr (0 == sizeof...(Params)) {
        return TupleWithParamsProxy<decltype(std::tuple<>())>(funcName_, std::tuple<>());
      } else {
        const auto tupleWithParams = std::tuple<Params...>(std::forward<Params>(params)...);

        return TupleWithParamsProxy<decltype(tupleWithParams)>(funcName_, tupleWithParams);
      }
    }

    template <typename TupleWithParams>
    struct TupleWithParamsProxy {
      static constexpr auto TupleSize = std::tuple_size_v<TupleWithParams>;

      TupleWithParamsProxy(const std::string& funcName, const TupleWithParams& tupleWithParams) :
        funcName_(funcName), tupleWithParams_(tupleWithParams) {}

      // 'ipcSync' is a pointer to the IPC transport function, it is the last argument in 'IPC_SYNC_CALL'.
      Ret operator() (std::vector<uint8_t>(ipcSync)(const std::vector<uint8_t>&)) {
        Serializer serializer;

        serializer << funcName_;

        // Enumerate 'params' from the tuple and serialize them in 'serializer'.
        if constexpr (TupleSize) {
          SerializeParams<0>(serializer);
        }

        // 'ipcSync' sends 'std::vector<uint8_t>' to the server and receives 'std::vector<uint8_t>' reply.
        const auto& replyFromServer = ipcSync(serializer.Bytes());

        Unserializer unserializer(replyFromServer);

        if constexpr (std::is_void_v<Ret>) {
          if constexpr (TupleSize) {
            UnserializeParams<TupleSize - 1>(unserializer);
          }
        } else {
          Ret ret;
          unserializer >> ret;

          if constexpr (TupleSize) {
            UnserializeParams<TupleSize - 1>(unserializer);
          }

          return ret;
        }
      }

      // Serialize all 'params' in 'serializer'.
      template<int Index>
      void SerializeParams(Serializer& serializer) {
        auto&& param = std::get<Index>(tupleWithParams_);

        serializer << param;

        if constexpr (Index < TupleSize - 1) {
          SerializeParams<Index + 1>(serializer);
        }
      }

      // Unserialize only 'out' 'params' from 'userializer' in reversed order.
      template<int Index>
      void UnserializeParams(Unserializer& unserializer) {
        // Unserialize if 'param' is 'out'.
        if constexpr (IsOutParam<std::tuple_element_t<Index, TupleWithParams>>()) {
          unserializer >> std::get<Index>(tupleWithParams_);
        }

        if constexpr (Index > 0) {
          UnserializeParams<Index - 1>(unserializer);
        }
      }

    private:
      std::string funcName_;
      TupleWithParams tupleWithParams_;
    };

  private:
    std::string funcName_;
  };


  // 'IPC_SEND' calls 'Send'
  template <typename> struct AsyncCall;

  template <typename ...Params>
  struct AsyncCall<void(*)(Params...)> {
    AsyncCall(const std::string& funcName) : funcName_(funcName) {}

    auto operator()(Params... params) {
      if constexpr (0 == sizeof...(Params)) {
        return TupleWithParamsProxy<decltype(std::tuple<>())>(funcName_, std::tuple<>());
      } else {
        const auto tupleWithParams = std::tuple<Params...>(std::forward<Params>(params)...);

        return TupleWithParamsProxy<decltype(tupleWithParams)>(funcName_, tupleWithParams);
      }
    }

    template <typename TupleWithParams>
    struct TupleWithParamsProxy {
      static constexpr auto TupleSize = std::tuple_size_v<TupleWithParams>;

      TupleWithParamsProxy(const std::string& funcName, const TupleWithParams& tupleWithParams) :
        funcName_(funcName), tupleWithParams_(tupleWithParams) {}

      // 'ipcAsync' is a pointer to the IPC transport function, it is the last argument in 'IPC_ASYNC_CALL'.
      void operator() (void(ipcAsync)(const std::vector<uint8_t>&)) {
        Serializer serializer;

        serializer << funcName_;

        // Enumerate 'params' from the tuple and serialize them in 'serializer'.
        if constexpr (TupleSize) {
          SerializeParams<0>(serializer);
        }

        // 'ipcAsync' sends 'std::vector<uint8_t>' to the server.
        ipcAsync(serializer.Bytes());
      }

      // Serialize all 'params' in 'serializer'.
      template<int Index>
      void SerializeParams(Serializer& serializer) {
        auto&& param = std::get<Index>(tupleWithParams_);

        static_assert(!IsOutParam<decltype(param)>(), "'out' parameters are not allowed in an asynchronous call");
            
        serializer << param;

        if constexpr (Index < TupleSize - 1) {
          SerializeParams<Index + 1>(serializer);
        }
      }

    private:
      std::string funcName_;
      TupleWithParams tupleWithParams_;
    };

  private:
    std::string funcName_;
  };

}

#define IPC_SEND_RECEIVE(x) IpcCall::SyncCall<decltype(&x)>(#x)
#define IPC_SEND(x) IpcCall::AsyncCall<decltype(&x)>(#x)
