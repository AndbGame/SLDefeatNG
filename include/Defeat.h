#pragma once

#include <DefeatSpinLock.h>
#include <DefeatUtils.h>
#include <DefeatPapyrus.h>


#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

namespace SexLabDefeat {

    struct DefeatForms {
        RE::TESQuest* DefeatRessourcesQst;
        RE::TESQuest* DefeatMCMQst;
        RE::TESQuest* DefeatPlayerQTE;

        RE::TESQuest* SexLabQuestFramework;

        RE::SpellItem* SatisfiedSPL;

        struct {
            RE::TESQuest* PlayerQST;
            RE::TESQuest* PAQst;
            RE::TESQuest* PlayerActionQst;
            RE::TESQuest* NPCsQst;
            RE::TESQuest* NPCsRefreshQst;
            RE::TESQuest* Robber;
            RE::TESQuest* DGIntimidateQuest;
            RE::TESQuest* WerewolfQst;
        } MiscQuests;

        struct {
            RE::EffectSetting* ImmunityEFF;
            RE::EffectSetting* HKActionEFF;
            RE::EffectSetting* HKFollowerActionEFF;
            RE::EffectSetting* SexCrimeEFF;
            RE::EffectSetting* NVNAssaultEFF;
        } MiscMagicEffects;

        struct {

        } Keywords;
    };


    class DefeatManager;

    class DefeatConfig {
    public:
        using StringSetVar = PapyrusInterface::ObjectVariable<std::set<std::string_view>>;
        using StringSetVarPtr = std::unique_ptr<StringSetVar>;

        DefeatConfig() = default;
        ~DefeatConfig() = default;

        struct Configuration {
            StringSetVarPtr RaceAllowedPvic;
            StringSetVarPtr RaceAllowedNVN;
        } Config;

        void readIniConfig();
        void Setup(SexLabDefeat::DefeatManager* defeatManager);
        void Reset();
        void LoadScriptObjects();

        bool CFG_PAPYUNHOOK = true;
        int CFG_LOGGING = 2;
        int HIT_SPAM_GUARD_EXPIRATION_MS = 500;
        float KD_FAR_MAX_DISTANCE = 1500.0;

        template <class T>
        T getConfig(std::string configKey, T _def) const;

        template <class T>
        T getSslConfig(std::string configKey, T _def) const;

    private:
        boost::property_tree::ptree _iniConfig;
        std::map<std::string, std::variant<std::string, int, float, bool>> _config;

        SexLabDefeat::DefeatManager* _defeatManager;

        RE::BSTSmartPointer<RE::BSScript::Object> DefeatMCMScr;
        RE::BSTSmartPointer<RE::BSScript::Object> defeatconfig;
        RE::BSTSmartPointer<RE::BSScript::Object> sslSystemConfig;
    };

    class DefeatWidget : public SpinLock {
    public:
        using StringVar = PapyrusInterface::ObjectVariable<std::string_view>;
        using StringVarPtr = std::unique_ptr<StringVar>;

        enum State { QTE_METER, DYNAMIC_WIDGET, NONE };

        StringVarPtr widgetRoot;

        DefeatWidget();
        ~DefeatWidget();

        [[nodiscard]] bool setVisible(bool inUITask = false);
        [[nodiscard]] bool setInvisible(bool inUITask = false);
        bool getLastVisible();
        [[nodiscard]] bool setPercent(float value, bool inUITask = false);
        float getLastPercent();

        State getState();
        [[nodiscard]] bool startDynamicWidget(bool inUITask = false);
        [[nodiscard]] bool stopDynamicWidget(bool inUITask = false);

    protected:
        std::string_view getWidgetRootId() const;

        RE::GPtr<RE::GFxMovieView> _hudmenu;
        RE::UI* _ui;

        float lastPercent = 0.0;
        bool lastVisible = false;
        State state = State::NONE;
    };

    class ActorExtraData {
    public:
        bool ignoreActorOnHit = true;
        int sexLabGender = 0;
        int sexLabSexuality = 0;
        bool sexLabAllowed = 0;
        std::string sexLabRaceKey = "";
        //bool defeatAllowed2PC = true;
        //bool defeatAllowed2NvN = true;
    };

    class DeferredExpiringValueCallback {
    public:
        DeferredExpiringValueCallback(std::function<void()> callback) { _callback = callback; };

        void execute() {
            _callback();
        
        };

    protected:
        std::function<void()> _callback;
    };

    template <class T>
    class DeferredExpiringValue : public SpinLock {
    public:
        DeferredExpiringValue(std::unique_ptr<DeferredExpiringValueInitializer> initializer, int expirationMs = 0,
                              int accessProlongationExpireMs = 0);
        ~DeferredExpiringValue();

        void getCallback(std::function<void()> callback);
        bool isActualValue();
        T getValue() { return _value; };
        void initializeValue(T val);

    protected:
        void accessTouch();
        void processCallbackStack();

        T _value = {};
        std::queue<std::shared_ptr<DeferredExpiringValueCallback>> _callbackQueue;
        std::unique_ptr<DeferredExpiringValueInitializer> _initializer;

        std::chrono::milliseconds _expiration;
        std::chrono::milliseconds _accessProlongationExpireMs;
        std::chrono::high_resolution_clock::time_point _expirationTime;
        std::chrono::high_resolution_clock::time_point _minTime;

    };

    class DefeatActor;

    using DefeatActorType = std::shared_ptr<DefeatActor>;
    using HitSource = RE::FormID;
    using HitProjectile = RE::FormID;

    class DefeatActor : public SpinLock {
    public:
        enum States { NONE, ACTIVE, DISACTIVE };

        DefeatActor(RE::Actor* actor, DefeatManager* defeatManager);
        ~DefeatActor();

        RE::FormID getActorFormId() const { return _actorFormId; };
        RE::Actor* getActor();

        bool isSame(RE::Actor* actor);
        bool isPlayer();
        float getDistanceTo(DefeatActorType target);
        float getHeadingAngle(DefeatActorType target);
        float getActorValuePercentage(RE::ActorValue av);
        RE::TESForm* getEquippedSource(HitSource source);
        bool wornHasAnyKeyword(std::list<std::string> kwds);

        bool hasHitImmunity();
        void addHitImmunity(int ms);
        void setLastHitAggressor(DefeatActorType lastHitAggressor);
        bool isSurrender();
        bool isCreature();
        bool isFollower();
        bool notInFlyingState();
        bool isSatisfied();
        bool isKDImmune();
        bool isKDAllowed();
        bool isTied();
        float getDynamicDefeat();
        SpinLock* getDynamicDefeatSpinLock();
        void resetDynamicDefeat();
        void incrementDynamicDefeat(float val);
        void decrementDynamicDefeat(float val);
        States getState();
        void setState(States state);

        float getVulnerability();
        void setVulnerability(float vulnerability);

        /* DeferredExpiringValue */
        bool isIgnoreActorOnHit();

        int getSexLabGender();
        int getSexLabSexuality();
        bool isSexLabAllowed();
        std::string getSexLabRaceKey();

        bool isDefeatAllowed2PC();
        bool isDefeatAllowed2NvN();
        /* / DeferredExpiringValue */

        bool isFemale();
        bool IsStraight();
        bool IsGay();
        bool IsBisexual();

        bool CheckAggressor(DefeatActorType aggressor);
        bool isDefeatAllowedByAgressor(DefeatActorType aggressor);
        bool IsSexualAssaulterByAggressor(DefeatActorType aggressor);
        bool hasSexInterestByAggressor(DefeatActorType aggressor);
        bool hasSexCombinationWithAggressor(DefeatActorType aggressor);

        std::atomic<bool> isSheduledDeplateDynamicDefeat = false;

        DeferredExpiringValue<ActorExtraData>* extraData;

    protected:
        RE::FormID _actorFormId;
        RE::Actor* _actor;

        float _vulnerability = 0;

        DefeatActorType _lastHitAggressor;
        bool _isSurrender = false;
        States _state = States::ACTIVE;

        std::chrono::high_resolution_clock::time_point hitImmunityExpiration;
        std::chrono::high_resolution_clock::time_point _minTime;
        SexLabDefeat::DefeatManager* _defeatManager;
        float _dynamicDefeat = 0;
        SpinLock* _dynamicDefeatSpinLock = nullptr;
    };

    class DefeatActorManager : public SpinLock {
    public:
        DefeatActorManager(DefeatManager* defeatManager) /* : SpinLock()*/ {
            _defeatManager = defeatManager;
        };
        ~DefeatActorManager() = default;

        void reset();

        DefeatActorType getPlayer() { return _player; }

        DefeatActorType getActor(RE::Actor* actor);

        /* Pre Checks functions */
        bool validForAggressorRole(RE::Actor* actor);
        bool validForAggressorRoleOverPlayer(RE::Actor* actor);
        bool validPlayerForVictimRole(RE::Actor* actor);
        /* / Pre Checks functions  */

    protected:
        std::map<RE::FormID, DefeatActorType> _actorMap;

        DefeatActorType _player;
        SexLabDefeat::DefeatManager* _defeatManager;
    };

    struct RawHitEvent {
        RE::TESObjectREFR* target;
        RE::TESObjectREFR* aggressor;
        RE::FormID source;
        RE::FormID projectile;
        bool isPowerAttack = false;
        bool isSneakAttack = false;
        bool isBashAttack = false;
        bool isHitBlocked = false;
    };

    class HitEvent {
    public:
        HitEvent() = default;
        ~HitEvent() = default;
        DefeatActorType target;
        DefeatActorType aggressor;
        HitSource source;
        HitProjectile projectile;
        bool isPowerAttack = false;
        bool isSneakAttack = false;
        bool isBashAttack = false;
        bool isHitBlocked = false;
    };

    enum HitResult { SKIP, KNONKOUT, STANDING_STRUGGLE, KNONKDOWN };

    using HitEventType = std::shared_ptr<HitEvent>;

    class DynamicDefeatDepleter;

    class DefeatCombatManager {
    public:
        struct HitSpamKey {
            RE::FormID actor;
            RE::FormID source;
        };
        struct ProjectileSpamHash {
            std::size_t operator()(const HitSpamKey& k) const {
                return std::hash<std::uint32_t>()(k.actor) ^ (std::hash<std::uint32_t>()(k.source) << 1);
            }
        };

        struct HitSpamEqual {
            bool operator()(const HitSpamKey& lhs, const HitSpamKey& rhs) const {
                return lhs.actor == rhs.actor && lhs.source == rhs.source;
            }
        };

        RE::BSFixedString* OnSLDefeatPlayerKnockDownEventName;

        DefeatCombatManager(DefeatActorManager* defeatActorManager,
                            DefeatManager* defeatManager);
        ~DefeatCombatManager();

        DefeatManager* getDefeatManager() { return _defeatManager; };

        void onHitHandler(RawHitEvent event);

        void calculatePlayerHit(HitEventType event);
        HitResult KDWay(HitEventType event);
        HitResult KDWayWound(HitEventType event);
        HitResult KDWayExhaustion(HitEventType event);
        HitResult KDWayVulnerability(HitEventType event);
        HitResult KDWayDynamic(HitEventType event);
        float KDWayDynamicCalculation(HitEventType event);
        HitResult KDWayPowerAtk(HitEventType event);

        void shedulePlayerDeplateDynamicDefeat();
        std::atomic<bool> _playerDeplateDynamicDefeatStopThread = true;
        void interruptPlayerDeplateDynamicDefeat();

        bool KDOnlyBack(bool opt, HitEventType event);
        bool registerAndCheckHitGuard(RE::TESObjectREFR* actor, RE::FormID source, RE::FormID projectile);


    protected:
        SexLabDefeat::DefeatActorManager* _defeatActorManager;
        SexLabDefeat::DefeatManager* _defeatManager;
        //std::unordered_map<HitSpamKey, std::chrono::high_resolution_clock::time_point, ProjectileSpamHash, HitSpamEqual>
        //    projectileSpamGuard;
        std::unordered_map<HitSpamKey, std::chrono::high_resolution_clock::time_point, ProjectileSpamHash, HitSpamEqual>
            hitSpamGuard;
        std::chrono::milliseconds _hitGuardExpiration;
        SpinLock* hitSpamGuardSpinLock;

        void onPlayerHitHandler(RawHitEvent event, SexLabDefeat::DefeatActorType defActor);
    };
    
    class DefeatManager {
    public:
        enum GameState {
            NONE,
            PRE_LOAD,
            IN_GAME,
        };

        struct {
            bool ZaZ = false;
        } SoftDependency;

        DefeatManager(SexLabDefeat::DefeatConfig* defeatConfig);

        void load();
        void reset();
        void reInitializeWidget() const;
        GameState getGameState();
        void setGameState(GameState state);
        void ActorEnterdToCombatState(RE::Actor* target_actor);

        bool randomChanse(float chanse, float min = 1, float max = 100);

        DefeatForms Forms;

        HitEventType createHitEvent(RE::Actor* target_actor, RE::Actor* aggr_actor, RawHitEvent rawHitEvent);
        void setWidget(SexLabDefeat::DefeatWidget* widget);
        SexLabDefeat::DefeatWidget* getWidget();
        void setActorState(RE::Actor* target_actor, DefeatActor::States state);

        SexLabDefeat::DefeatCombatManager* getCombatManager() { return _defeatCombatManager; };
        SexLabDefeat::DefeatActorManager* getActorManager() { return _defeatActorManager; };
        SexLabDefeat::DefeatConfig* getConfig() { return _defeatConfig; };

    protected:
        void initializeDependency();
        void initializeForms();

        std::atomic<GameState> _gameState;
        SexLabDefeat::DefeatConfig* _defeatConfig;
        SexLabDefeat::DefeatCombatManager* _defeatCombatManager;
        SexLabDefeat::DefeatActorManager* _defeatActorManager;
        SexLabDefeat::DefeatWidget* _defeatWidget = nullptr;
    };

    void installHooks(SexLabDefeat::DefeatManager* defeatManager);
    void installEventSink(SexLabDefeat::DefeatManager* defeatManager);
}