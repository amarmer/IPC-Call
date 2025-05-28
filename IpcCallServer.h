#pragma once

#include <memory>
#include <utility>
#include <stdexcept>

#include "IpcCallData.h"

namespace IpcCall {
    struct Server {
        template <typename Ret, typename F, typename Tuple, int Index, typename ...Args>
        static void UnserializeCallSerialize(F f, Serializer& serializer, Unserializer& unserializer, Args&&...args) {
            if constexpr (Index < std::tuple_size_v<Tuple>) {
                using param_t = decltype(std::get<Index>(std::declval<Tuple>()));

                std::decay_t<param_t> arg;
                unserializer >> arg;

                UnserializeCallSerialize<Ret, F, Tuple, Index + 1, Args...>(f, serializer, unserializer, std::forward<Args>(args)..., arg);

                // If 'out' parameter, serialized in reversed order. 
                if constexpr (IsOutParam<param_t>())
                {
                    serializer << arg;
                } 
            } else {
                if constexpr (std::is_void_v<Ret>) {
                    f(std::forward<Args>(args)...);
                } else {
                    serializer << f(std::forward<Args>(args)...);
                }
            }
        }

        struct IFunction
        {
            virtual bytes_t Call(Unserializer& unserializer) const = 0;
            virtual ~IFunction() = default;
        };

        template <typename F>
        struct Function: public IFunction
        {
            Function(F f):f_(f) {}

            bytes_t Call(Unserializer& unserializer) const override {
                Serializer serializer;

                return Call(f_, serializer, unserializer);
            }

            template <typename Ret, typename ...Params>
            bytes_t Call(Ret(*)(Params...), Serializer& serializer, Unserializer& unserializer) const
            {
                UnserializeCallSerialize<Ret, F, std::tuple<Params...>, 0>(f_, serializer, unserializer);

                return serializer.Bytes();
            }

        private:
            F f_;
        };

        struct Functions {
            static Functions& Instance() {
                static Functions s_functions;
                return s_functions;
            }

            template <typename Ret, typename ...Params>
            bool RegisterFunc(const std::string& funcName, Ret(*f)(Params...))
            {
                mapNameFunction_[funcName] = std::make_unique<Function<decltype(f)>>(f);

                return true;
            }

            IFunction* FindFunction(const std::string& funcName) {
                auto it = mapNameFunction_.find(funcName);
                if (it == mapNameFunction_.end()) {
                    return nullptr;
                }

                return it->second.get();
            }

        private:
            std::map<std::string, std::unique_ptr<IFunction>> mapNameFunction_;
        };

        // It should be called by the server IPC transport with the 'bytes' that is received from the client.
        static std::vector<uint8_t> Call(const std::vector<uint8_t>& bytes) {
            Unserializer unserializer(bytes);

            std::string funcName;
            unserializer >> funcName;

            auto pFunction = Functions::Instance().FindFunction(funcName);

            if (pFunction == nullptr) {
                throw std::runtime_error("IPC function '" + funcName + "' is no registered");
            } 

            return pFunction->Call(unserializer);
        }
    };
}

#define IPC_CALL_REGISTER(f) static auto f##IpcRegisterFunction = IpcCall::Server::Functions::Instance().RegisterFunc(#f, f)
