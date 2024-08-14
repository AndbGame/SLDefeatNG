#pragma once

#include <Defeat.h>
#include "DefeatConfig.h"
#include "DefeatActor.h"

namespace SexLabDefeat {
    class DefeatManager;

    class TDefeatScene {
    protected:
        DefeatManager* manager;
        static DefeatSceneResult makeDefeatSceneResult(DefeatSceneResult::ResultStatus status, std::string reason = "");
    };

    class DefeatInternalScene : public IDefeatScene, public virtual TDefeatScene {
    public:
        DefeatInternalScene(std::string_view uid, DefeatManager* manager) : uid(uid) {
            this->manager = manager;
            SKSE::log::trace("Scene - '{}' constructed", getUID());
        };
        ~DefeatInternalScene(){
            SKSE::log::trace("Scene - '{}' deleted", getUID());
        };

        std::string_view getUID() override { return uid; };

        DefeatSceneResult start() override;

    protected:
        std::string_view uid;

        virtual DefeatSceneResult resolve();
    };

    class TDefeatSceneWithVictim: public virtual TDefeatScene {
    public:
        IDefeatActorType getVictim() { return victim; }

    protected:
        IDefeatActorType victim;
        DefeatSceneResult resolveVictim();
        virtual DefeatSceneResult selectVictim();
    };

    class TDefeatSceneWithAggressors : public virtual TDefeatSceneWithVictim, public virtual TDefeatScene {
    public:
        std::list<IDefeatActorType> getAggressors() { return aggressors; }

    protected:
        std::list<IDefeatActorType> aggressors = {};
        DefeatSceneResult resolveAgressors();
        virtual DefeatSceneResult resolveAgressor(DefeatActorType agg);
        virtual DefeatSceneResult selectAgressor(DefeatActorType agg);
    };

    class DefeatWaitingScene : public DefeatInternalScene {
    public:
        DefeatWaitingScene(std::chrono::duration<float, std::milli> duration,
                           DefeatManager* manager)
            : DefeatInternalScene("waiting", manager), duration(duration) {}

    protected:
        std::chrono::duration<float, std::milli> duration;
    };

    class DefeatNoneScene : public DefeatInternalScene {
    public:
        DefeatNoneScene(DefeatManager* manager)
            : DefeatInternalScene("none", manager) {}
    };

    class DefeatKnockdownScene : public DefeatInternalScene,
                            public virtual TDefeatSceneWithVictim,
                            public virtual TDefeatSceneWithAggressors {
    public:
        DefeatKnockdownScene(DefeatActorType victim, DefeatManager* manager)
            : DefeatInternalScene("knockdown", manager) {
            this->victim = victim;
        }

    protected:
        DefeatSceneResult resolve() override;
        DefeatSceneResult resolveAgressor(DefeatActorType agg) override;
    };

    class DefeatRapeScene : public DefeatInternalScene,
                            public virtual TDefeatSceneWithVictim,
                            public virtual TDefeatSceneWithAggressors {
    public:
        DefeatRapeScene(DefeatActorType victim, DefeatManager* manager) : DefeatInternalScene("rape", manager) {
            this->victim = victim;
        }

    protected:
        DefeatSceneResult resolve() override;
        DefeatSceneResult selectVictim() override;
        DefeatSceneResult resolveAgressor(DefeatActorType agg) override;
        DefeatSceneResult selectAgressor(DefeatActorType agg) override;
    };

    class DefeatRobScene : public DefeatInternalScene,
                           public virtual TDefeatSceneWithVictim,
                           public virtual TDefeatSceneWithAggressors {
    public:
        DefeatRobScene(DefeatActorType victim, IDefeatActorType aggressor, DefeatManager* manager)
            : DefeatInternalScene("rob", manager) {
            this->victim = victim;
        }
    };

    class DefeatKillScene : public DefeatInternalScene,
                            public virtual TDefeatSceneWithVictim,
                            public virtual TDefeatSceneWithAggressors {
    public:
        DefeatKillScene(DefeatActorType victim, IDefeatActorType aggressor, DefeatManager* manager)
            : DefeatInternalScene("kill", manager) {
            this->victim = victim;
        }
    };

    class DefeatSceneManager : public IDefeatSceneManager {
    public:
        DefeatSceneManager(DefeatManager* manager)
            : manager(manager){};
        std::shared_ptr<DefeatRapeScene> querySceneForVictimRape(DefeatActorType actor);
        std::shared_ptr<DefeatKnockdownScene> querySceneForVictimKnockdown(DefeatActorType actor);

    protected:
        DefeatManager* manager;
    };
}