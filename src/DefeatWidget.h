#pragma once

#include <Defeat.h>

#include "DefeatSpinLock.h"
#include "PapyrusInterface\ObjectVariable.h"

namespace SexLabDefeat {

    class DefeatWidget : public IDefeatWidget, public SpinLock {
    public:

        PapyrusInterface::StringVarPtr widgetRoot;
        PapyrusInterface::BoolVarPtr widgetReady;

        DefeatWidget();
        ~DefeatWidget();

        bool isInitialized = false;

        [[nodiscard]] bool setVisible(bool inUITask = false) override;
        [[nodiscard]] bool setInvisible(bool inUITask = false) override;
        bool getLastVisible() override;
        [[nodiscard]] bool setPercent(float value, bool inUITask = false) override;
        float getLastPercent() override;

        State getState() override;
        [[nodiscard]] bool startDynamicWidget(bool inUITask = false) override;
        [[nodiscard]] bool stopDynamicWidget(bool inUITask = false) override;

    protected:
        bool initialize();

        std::string_view getWidgetRootId();
        std::string_view _rootId;

        RE::GPtr<RE::GFxMovieView> _hudmenu = nullptr;
        RE::UI* _ui;

        float lastPercent = 0.0;
        bool lastVisible = false;
        State state = State::NONE;
    };
}