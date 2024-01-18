#include "Defeat.h"

namespace SexLabDefeat {
    namespace PapyrusInterface {

        template class ObjectVariable<bool>;
        template class ObjectVariable<float>;
        template class ObjectVariable<int>;
        template class ObjectVariable<std::string_view>;
        template class ObjectVariable<std::set<std::string_view>>;

        void EmptyRequestCallback::operator()(
            [[maybe_unused]] RE::BSScript::Variable a_result) {
            SKSE::log::trace("EmptyRequestCallback - {}", _info);
        }

        template <typename T>
        ObjectVariable<T>::ObjectVariable(ScriptObjectCallbackReceiver scriptObjectClb, std::string_view a_var,
                                          ObjectVariableConfig config) {
            _scriptObjectClb = scriptObjectClb;
            _varName = a_var;
            _config = config;
        }

        template <typename T>
        T ObjectVariable<T>::get(T _def) {
            spinLock();
            T ret = retrieve(_def);
            spinUnlock();
            return ret;
        }

        template <class T>
        inline void ObjectVariable<T>::invalidate() {
            spinLock();
            _state = State::INVALID;
            spinUnlock();
        }

        template <class T>
        ObjectVariable<T>::State ObjectVariable<T>::getState() {
            spinLock();
            auto ret = _state;
            spinUnlock();
            return ret;
        }

        template <typename T>
        T ObjectVariable<T>::retrieve(T _def) {
            if (_config.isPersistent) {
                if (_state == State::FETCHED) {
                    return value;
                }
            }
            if (_state == State::NOT_EXIST) {
                return _def;
            }
            value = getFromVM(_def);
            return value;
        }

        template <typename T>
        T ObjectVariable<T>::getFromVM(T _def) {
            auto _scriptObject = _scriptObjectClb();
            if (_scriptObject == nullptr) {
                _state = State::NOT_EXIST;
                SKSE::log::critical("getFromVM - Script Object is nullptr");
                return _def;
            }
            Variable* var;
            if (_config.isProprty) {
                var = _scriptObject->GetProperty(_varName);
            } else {
                var = _scriptObject->GetVariable(_varName);
            }
            if (var == nullptr) {
                _state = State::NOT_EXIST;
                SKSE::log::critical("getFromVM - {} '{}' not found in Script Object",
                                    (_config.isProprty ? "Property" : "Variable"),
                                    _varName);
                return _def;
            }

            T val = T();
            bool correctType = false;
            if constexpr (std::is_same_v<T, bool>) {
                if (var->IsBool()) {
                    val = var->GetBool();
                    correctType = true;
                    SKSE::log::trace("getFromVM - var '{}' is '{}'", _varName, val);
                }
            } else if constexpr (std::is_same_v<T, float>) {
                if (var->IsFloat()) {
                    val = var->GetFloat();
                    correctType = true;
                    SKSE::log::trace("getFromVM - var '{}' is '{}'", _varName, val);
                }
            } else if constexpr (std::is_same_v<T, int>) {
                if (var->IsInt()) {
                    val = var->GetSInt();
                    correctType = true;
                    SKSE::log::trace("getFromVM - var '{}' is '{}'", _varName, val);
                }
            } else if constexpr (std::is_same_v<T, std::string_view>) {
                if (var->IsString()) {
                    val = var->GetString();
                    correctType = true;
                    SKSE::log::trace("getFromVM - var '{}' is '{}'", _varName, val);
                }
            } else if constexpr (std::is_same_v<T, std::set<std::string_view>>) {
                if (var->IsArray()) {
                    auto data = var->GetArray();
                    if (data != nullptr && data.get() != nullptr) {
                        for (RE::BSScript::Variable* it = data->begin(); it != data->end(); it++) {
                            if (it != nullptr && it->IsString()) {
                                val.insert(it->GetString());
                                //SKSE::log::trace("getFromVM - var '{}' is array[]='{}'", _varName, it->GetString());
                            }
                        }
                    }
                    correctType = true;
                }
            }
            if (!correctType) {
                SKSE::log::critical("getFromVM - Incottect type of variable", _varName);
                _state = State::NOT_EXIST;
                return _def;
            } else {
                _state = State::FETCHED;
                return val;
            }
        }
    }
}