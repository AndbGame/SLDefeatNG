#include "DefeatWidget.h"

namespace SexLabDefeat {

    DefeatWidget::DefeatWidget() {
        if (!initialize()) {
            SKSE::log::error("Widget initialization failed");
        }
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

            if (value > 0 && state == DefeatWidget::State::DYNAMIC_WIDGET && setVisible(true)) {
                // Force visible... workaround
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
        bool ret = true;
        spinLock();
        if (state == DefeatWidget::State::DYNAMIC_WIDGET) {
            if (setInvisible(inUITask) && setPercent(0, inUITask)) {
                state = DefeatWidget::State::NONE;
            } else {
                ret = false;
            }
        }
        spinUnlock();
        return ret;
    }
    bool DefeatWidget::initialize() {
        _ui = RE::UI::GetSingleton(); 
        if (_ui == nullptr) {
            SKSE::log::trace("DefeatWidget::initialize UI nullptr");
            return false;
        }
        auto loc_hud = _ui->menuMap.find("HUD Menu");
        if (loc_hud == _ui->menuMap.end()) {
            SKSE::log::trace("DefeatWidget::initialize 'HUD Menu' not exist");
            return false;
        }
        RE::GPtr<RE::IMenu> loc_hudmenu = loc_hud->second.menu;
        if (loc_hudmenu == nullptr) {
            SKSE::log::trace("DefeatWidget::initialize loc_hudmenu nullptr");
            return false;
        }
        RE::GPtr<RE::GFxMovieView> loc_movie = loc_hudmenu->uiMovie;
        if (loc_movie == nullptr) {
            SKSE::log::trace("DefeatWidget::initialize loc_movie nullptr");
            return false;
        }

        _hudmenu = loc_hudmenu->uiMovie;
        return true;
    }

    std::string_view DefeatWidget::getWidgetRootId() {
        if (!_rootId.empty()) {
            return _rootId;
        }
        auto _widgetReady = widgetReady->get();
        if (!_widgetReady) {
            return ""sv;
        }
        auto _widgetRoot = widgetRoot->get();
        if (_widgetRoot.empty()) {
            widgetRoot->invalidate();
        } else {
            _rootId = _widgetRoot;
        }
        return _widgetRoot;
    }
}
