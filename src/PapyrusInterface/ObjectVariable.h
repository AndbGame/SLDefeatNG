#pragma once

#include "../DefeatSpinLock.h"
#include "DefeatPapyrus.h"

namespace SexLabDefeat {

    namespace PapyrusInterface {
        using Variable = RE::BSScript::Variable;
        using ScriptObjectCallbackReceiver = std::function<ObjectPtr()>;

        struct ObjectVariableConfig {
            ObjectVariableConfig(bool _isProprty = true, bool _isPersistent = false)
                : isProprty(_isProprty), isPersistent(_isPersistent){};

            bool isProprty = true;
            bool isPersistent = false;
        };

        template <class T>
        class ObjectVariable : public SpinLock {
        public:
            enum State { NOT_INITIALIZED, INVALID, FETCHED, NOT_EXIST };
            ObjectVariable(ScriptObjectCallbackReceiver scriptObjectClb, std::string_view a_var,
                           ObjectVariableConfig config);
            ~ObjectVariable() = default;

            T get(T _def = {});
            void invalidate();
            State getState();

        protected:
            T retrieve(T _def);
            T getFromVM(T _def);
            T value;
            State _state = State::NOT_INITIALIZED;
            ObjectVariableConfig _config;
            ScriptObjectCallbackReceiver _scriptObjectClb;
            RE::BSFixedString _varName;
        };

    }
}
