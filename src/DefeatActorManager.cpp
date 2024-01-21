#include "DefeatActorManager.h"

namespace SexLabDefeat {
    DefeatConfig* DefeatActorManager::getConfig() { return _defeatManager->getConfig(); }
    DefeatForms DefeatActorManager::getForms() const {
        return _defeatManager->Forms;
    }
    DefeatManager::SoftDependencyType DefeatActorManager::getSoftDependency() const {
        return _defeatManager->SoftDependency;
    }
}