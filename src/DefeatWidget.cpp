#include "Defeat.h"

namespace SexLabDefeat {

    DefeatWidget::DefeatWidget(std::string widgetRoot) {
        _widgetRoot = widgetRoot;

        _ui = RE::UI::GetSingleton();
        auto loc_hud = _ui->menuMap.find("HUD Menu");
        RE::GPtr<RE::IMenu> loc_hudmenu = loc_hud->second.menu;
        RE::GPtr<RE::GFxMovieView> loc_movie = loc_hudmenu->uiMovie;

        _hudmenu = loc_hudmenu->uiMovie;
    }

    DefeatWidget::~DefeatWidget() { _hudmenu.reset(); }

    void DefeatWidget::setVisible(bool inUITask) {
        spinLock();
        lastVisible = true;
        spinUnlock();
        auto task = [this] {
            SKSE::log::trace("setVisible");

            const std::string method = (_widgetRoot + ".setAlpha");

            RE::GFxValue loc_arg1;
            loc_arg1.SetNumber(100.0);

            RE::GFxValue* loc_argptr1 = NULL;
            loc_argptr1 = &loc_arg1;

            _hudmenu->InvokeNoReturn(method.c_str(), loc_argptr1, 1);
            //_hudmenu->Advance(0.0001f);
        };
        if (inUITask) {
            task();
        } else {
            SKSE::GetTaskInterface()->AddUITask(task);
        }
    }

    void DefeatWidget::setInvisible(bool inUITask) {
        spinLock();
        lastVisible = false;
        spinUnlock();
        auto task = [this] {
            SKSE::log::trace("setInvisible");

            const std::string method = (_widgetRoot + ".setAlpha");

            RE::GFxValue loc_arg1;
            loc_arg1.SetNumber(0.0);

            RE::GFxValue* loc_argptr1 = NULL;
            loc_argptr1 = &loc_arg1;

            _hudmenu->InvokeNoReturn(method.c_str(), loc_argptr1, 1);
            //_hudmenu->Advance(0.0001f);
        };
        if (inUITask) {
            task();
        } else {
            SKSE::GetTaskInterface()->AddUITask(task);
        }
    }

    bool DefeatWidget::getLastVisible() {
        spinLock();
        auto ret = lastVisible;
        spinUnlock();
        return ret;
    }

    void DefeatWidget::setPercent(float value, bool inUITask) {
        spinLock();
        lastPercent = value;
        spinUnlock();
        auto task = [this, value] {
            SKSE::log::trace("setPercent {}%", static_cast<int>(value * 100));

            const std::string method = (_widgetRoot + ".setPercent");

            RE::GFxValue loc_arg1;
            loc_arg1.SetNumber(value);

            RE::GFxValue* loc_argptr1 = NULL;
            loc_argptr1 = &loc_arg1;

            _hudmenu->InvokeNoReturn(method.c_str(), loc_argptr1, 1);
            //_hudmenu->Advance(0.0001f);
        };
        if (inUITask) {
            task();
        } else {
            SKSE::GetTaskInterface()->AddUITask(task);
        }
    }
    float DefeatWidget::getLastPercent() {
        spinLock();
        auto ret = lastPercent;
        spinUnlock();
        return ret;
    }

    DefeatWidget::State DefeatWidget::getState() {
        spinLock();
        auto ret = state;
        spinUnlock();
        return ret;
    }

    void DefeatWidget::startDynamicWidget(bool inUITask) {
        spinLock();
        setVisible(inUITask);
        state = DefeatWidget::State::DynamicWidget;
        spinUnlock();
    }

    void DefeatWidget::stopDynamicWidget(bool inUITask) {
        spinLock();
        if (state == DefeatWidget::State::DynamicWidget) {
            setInvisible(inUITask);
            setPercent(0, inUITask);
            state = DefeatWidget::State::None;
        }
        spinUnlock();
    }
}
