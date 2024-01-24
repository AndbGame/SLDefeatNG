#include "DefeatPapyrus.h"

namespace SexLabDefeat {
    namespace PapyrusInterface {

        void EmptyRequestCallback::operator()([[maybe_unused]] RE::BSScript::Variable a_result) {
            SKSE::log::trace("EmptyRequestCallback - {}", _info);
        }
    }
}