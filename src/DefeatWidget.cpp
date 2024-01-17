#include "Defeat.h"

namespace SexLabDefeat {

    DefeatWidget::DefeatWidget() {

        _ui = RE::UI::GetSingleton();
        auto loc_hud = _ui->menuMap.find("HUD Menu");
        RE::GPtr<RE::IMenu> loc_hudmenu = loc_hud->second.menu;
        RE::GPtr<RE::GFxMovieView> loc_movie = loc_hudmenu->uiMovie;

        _hudmenu = loc_hudmenu->uiMovie;
    }

    DefeatWidget::~DefeatWidget() { _hudmenu.reset(); }

    bool DefeatWidget::setVisible(bool inUITask) {
        auto _widgetRoot = getWidgetRootId();
        if (_widgetRoot.empty()) {
            // SKSE::log::error("WidgetRoot not initialized");
            return false;
        }
        spinLock();
        lastVisible = true;
        spinUnlock();
        auto task = [this] {
            auto _widgetRoot = std::string(getWidgetRootId());
            if (_widgetRoot.empty() || _hudmenu.get() == nullptr) {
                SKSE::log::error("WidgetRoot not initialized");
                return;
            }
            SKSE::log::trace("DefeatWidget setVisible");

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
        return true;
    }

    bool DefeatWidget::setInvisible(bool inUITask) {
        auto _widgetRoot = getWidgetRootId();
        if (_widgetRoot.empty()) {
            // SKSE::log::error("WidgetRoot not initialized");
            return false;
        }
        spinLock();
        lastVisible = false;
        spinUnlock();
        auto task = [this] {
            auto _widgetRoot = std::string(getWidgetRootId());
            if (_widgetRoot.empty() || _hudmenu.get() == nullptr) {
                SKSE::log::error("WidgetRoot not initialized");
                return;
            }
            SKSE::log::trace("DefeatWidget setInvisible");

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
        return true;
    }

    bool DefeatWidget::getLastVisible() {
        spinLock();
        auto ret = lastVisible;
        spinUnlock();
        return ret;
    }

    bool DefeatWidget::setPercent(float value, bool inUITask) {
        auto _widgetRoot = getWidgetRootId();
        if (_widgetRoot.empty()) {
            //SKSE::log::error("WidgetRoot not initialized");
            return false;
        }
        spinLock();
        lastPercent = value;
        spinUnlock();
        auto task = [this, value] {
            auto _widgetRoot = std::string(getWidgetRootId());
            if (_widgetRoot.empty() || _hudmenu.get() == nullptr) {
                SKSE::log::error("WidgetRoot not initialized");
                return;
            }
            SKSE::log::trace("DefeatWidget setPercent {}%", static_cast<int>(value * 100));

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
        return true;
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

    bool DefeatWidget::startDynamicWidget(bool inUITask) {
        bool ret = false;
        spinLock();
        ret = setVisible(inUITask);
        if (ret) {
            state = DefeatWidget::State::DYNAMIC_WIDGET;
        }
        spinUnlock();
        return ret;
    }

    bool DefeatWidget::stopDynamicWidget(bool inUITask) {
        bool ret = false;
        spinLock();
        if (state == DefeatWidget::State::DYNAMIC_WIDGET) {
            if (setInvisible(inUITask) && setPercent(0, inUITask)) {
                state = DefeatWidget::State::NONE;
            }
        }
        spinUnlock();
        return ret;
    }
    std::string_view DefeatWidget::getWidgetRootId() const {
        auto _widgetRoot = widgetRoot->get();
        if (_widgetRoot.empty()) {
            widgetRoot->invalidate();
        }
        return _widgetRoot;
    }
}
