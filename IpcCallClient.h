#pragma once 

#include <utility>

#include "IpcCallData.h"

namespace IpcCall {
    // 'IPC_CALL' calls 'ClientCall'.
    template <typename> struct ClientCall;

    template <typename Ret, typename ...Params>
    struct ClientCall<Ret(*)(Params...)> {
        ClientCall(const std::string& funcName) : funcName_(funcName) {}

        auto operator()(Params... params) {
            if constexpr (sizeof...(Params)) {
                const auto tupleWithParams = std::tuple<Params...>(std::forward<Params>(params)...);

                return TupleWithParamsProxy<decltype(tupleWithParams)>(funcName_, tupleWithParams);
            } else {
                return TupleWithParamsProxy<decltype(std::tuple<>())>(funcName_, std::tuple<>());
            }
        }

        template <typename TupleWithParams>
        struct TupleWithParamsProxy {
            static constexpr auto TupleSize = std::tuple_size_v<TupleWithParams>;

            TupleWithParamsProxy(const std::string& funcName, const TupleWithParams& tupleWithParams): 
                funcName_(funcName), tupleWithParams_(tupleWithParams) {}

            // 'ipc' is pointer to IPC transport function, it is the last argument in 'IPC_CALL'.
            Ret operator() (std::vector<uint8_t>(ipc)(const std::vector<uint8_t>&)) {
                Serializer serializer;

                serializer << funcName_;

                // Enumerate 'params' from the tuple and serialize them in 'serializer'.
                if constexpr (TupleSize) {
                    SerializeParams<0>(serializer);
                }

                // 'ipc' sends 'std::vector<uint8_t>' to the server and receives 'std::vector<uint8_t>' reply.
                const auto& replyFromServer = ipc(serializer.Bytes());

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

                if constexpr (Index > 0 ) {
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
}

#define IPC_CALL(x) IpcCall::ClientCall<decltype(&x)>(#x)
