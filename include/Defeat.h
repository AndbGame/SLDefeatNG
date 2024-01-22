#pragma once

#include <DefeatSpinLock.h>
#include <DefeatUtils.h>
#include <DefeatPapyrus.h>
#include <DefeatForms.h>


#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

namespace SexLabDefeat {
    
    namespace PapyrusInterface {
        using StringSetVar = ObjectVariable<std::set<std::string_view>>;
        using StringSetVarPtr = std::unique_ptr<StringSetVar>;

        using StringVar = ObjectVariable<std::string_view>;
        using StringVarPtr = std::unique_ptr<StringVar>;

        using BoolVar = ObjectVariable<bool>;
        using BoolVarPtr = std::unique_ptr<BoolVar>;

        using FloatVar = ObjectVariable<float>;
        using FloatVarPtr = std::unique_ptr<FloatVar>;
    }

    class DefeatIManager;

    class DefeatConfig {
    public:
        DefeatConfig() = default;
        ~DefeatConfig() = default;

        struct Configuration {
            PapyrusInterface::FloatVarPtr PvicRaped;
            PapyrusInterface::FloatVarPtr NVNRapedFollower;
            PapyrusInterface::FloatVarPtr NVNRaped;
            PapyrusInterface::BoolVarPtr EveryonePvic;
            PapyrusInterface::BoolVarPtr HuntCrea;

            PapyrusInterface::BoolVarPtr SexualityPvic;
            PapyrusInterface::BoolVarPtr SexualityNVN;
            PapyrusInterface::BoolVarPtr MaleHunterPvic;
            PapyrusInterface::BoolVarPtr FemaleHunterPvic;
            PapyrusInterface::BoolVarPtr HuntFCrea;
            PapyrusInterface::BoolVarPtr MaleOnGal;
            PapyrusInterface::BoolVarPtr GalOnGal;
            PapyrusInterface::BoolVarPtr MaleOnMale;
            PapyrusInterface::BoolVarPtr GalOnMale;
            PapyrusInterface::BoolVarPtr CreaOnFemale;
            PapyrusInterface::BoolVarPtr CreaFemaleOnFemale;
            PapyrusInterface::BoolVarPtr CreaFemaleOnMale;
            PapyrusInterface::BoolVarPtr CreaOnMale;
            PapyrusInterface::BoolVarPtr BeastImmunity;


            PapyrusInterface::BoolVarPtr KDWayThreshold;
            PapyrusInterface::BoolVarPtr KDHealthBlock;
            PapyrusInterface::FloatVarPtr ChanceOnHitPvic;
            PapyrusInterface::FloatVarPtr ThresholdPvic;
            PapyrusInterface::FloatVarPtr ThresholdPvicMin;
            PapyrusInterface::FloatVarPtr KnockOutHPvic;
            PapyrusInterface::FloatVarPtr SStruggleHealthPvic;

            PapyrusInterface::BoolVarPtr KDWayStamina;
            PapyrusInterface::BoolVarPtr KDStaminaBlock;
            PapyrusInterface::BoolVarPtr KDWayStaminaOB;
            PapyrusInterface::FloatVarPtr ChanceOnHitPvicS;
            PapyrusInterface::FloatVarPtr ExhaustionPvic;
            PapyrusInterface::FloatVarPtr KnockOutSPvic;
            PapyrusInterface::FloatVarPtr SStruggleExhaustionPvic;


            PapyrusInterface::BoolVarPtr KDWayPowerAtk;
            PapyrusInterface::FloatVarPtr KDWayPowerAtkCOH;
            PapyrusInterface::FloatVarPtr PowerAtkStagger;
            PapyrusInterface::FloatVarPtr KnockOutPPvic;
            PapyrusInterface::FloatVarPtr SStrugglePowerPvic;

            PapyrusInterface::BoolVarPtr bResistQTE;

            struct {
                PapyrusInterface::BoolVarPtr DeviousFrameworkON;
                PapyrusInterface::BoolVarPtr KDWayVulnerabilityUseDFW;

                PapyrusInterface::BoolVarPtr KDWayVulnerability;
                PapyrusInterface::BoolVarPtr KDVulnerabilityBlock;
                PapyrusInterface::BoolVarPtr KDWayVulnerabilityOB;
                PapyrusInterface::FloatVarPtr ChanceOnHitPvicVulnerability;
                PapyrusInterface::FloatVarPtr VulnerabilityPvic;
                PapyrusInterface::FloatVarPtr KnockOutVulnerabilityPvic;
                PapyrusInterface::FloatVarPtr SStruggleVulnerabilityPvic;

                PapyrusInterface::BoolVarPtr KDWayDynamic;
                PapyrusInterface::FloatVarPtr KnockOutDynamicPvic;
                PapyrusInterface::FloatVarPtr SStruggleDynamicPvic;
                PapyrusInterface::FloatVarPtr DynamicDefeatOnHitBase;
                PapyrusInterface::FloatVarPtr DynamicDefeatOnHitOneHand;
                PapyrusInterface::FloatVarPtr DynamicDefeatOnHitTwoHand;
                PapyrusInterface::FloatVarPtr DynamicDefeatOnHitBow;
                PapyrusInterface::FloatVarPtr DynamicDefeatOnHitSpell;
                PapyrusInterface::FloatVarPtr DynamicDefeatVulnerabilityMult;
                PapyrusInterface::FloatVarPtr DynamicDefeatPowerAttackMult;
                PapyrusInterface::FloatVarPtr DynamicDefeatLowStaminaMult;
                PapyrusInterface::FloatVarPtr DynamicDefeatLowStaminaThreshold;
                PapyrusInterface::FloatVarPtr DynamicDefeatLowHealthMult;
                PapyrusInterface::FloatVarPtr DynamicDefeatLowHealthThreshold;
                PapyrusInterface::FloatVarPtr DynamicDefeatBackHitMult;
                PapyrusInterface::FloatVarPtr DynamicDefeatBlockReduction;
                PapyrusInterface::FloatVarPtr DynamicDefeatDepleteOverTime;

            } LRGPatch;

            PapyrusInterface::StringSetVarPtr RaceAllowedPvic;
            PapyrusInterface::StringSetVarPtr RaceAllowedNVN;

            struct {
                PapyrusInterface::BoolVarPtr UseCreatureGender;
            } SexLab;
        } Config;

        void readIniConfig();
        void Setup(SexLabDefeat::DefeatIManager* defeatManager);
        void Reset();
        void LoadScriptObjects();

        PapyrusInterface::ObjectPtr getDefeatMCMScript();
        PapyrusInterface::ObjectPtr getDefeatConfigScript();
        PapyrusInterface::ObjectPtr getSslSystemConfigScript();

        bool CFG_PAPYUNHOOK = true;
        int CFG_LOGGING = 2;
        int HIT_SPAM_GUARD_EXPIRATION_MS = 500;
        float KD_FAR_MAX_DISTANCE = 1500.0;
    private:
        boost::property_tree::ptree _iniConfig;
        std::map<std::string, std::variant<std::string, int, float, bool>> _config;

        SexLabDefeat::DefeatIManager* _defeatManager;

        PapyrusInterface::ObjectPtr DefeatMCMScr;
        PapyrusInterface::ObjectPtr defeatconfig;
        PapyrusInterface::ObjectPtr sslSystemConfig;
    };

    class DefeatWidget : public SpinLock {
    public:
        enum State { QTE_METER, DYNAMIC_WIDGET, NONE };

        PapyrusInterface::StringVarPtr widgetRoot;
        PapyrusInterface::BoolVarPtr widgetReady;

        DefeatWidget();
        ~DefeatWidget();

        bool isInitialized = false;

        [[nodiscard]] bool setVisible(bool inUITask = false);
        [[nodiscard]] bool setInvisible(bool inUITask = false);
        bool getLastVisible();
        [[nodiscard]] bool setPercent(float value, bool inUITask = false);
        float getLastPercent();

        State getState();
        [[nodiscard]] bool startDynamicWidget(bool inUITask = false);
        [[nodiscard]] bool stopDynamicWidget(bool inUITask = false);

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

    class ActorExtraData {
    public:
        bool ignoreActorOnHit = true;
        int sexLabGender = 0;
        int sexLabSexuality = 0;
        bool sexLabAllowed = 0;
        std::string sexLabRaceKey = "";
        float DFWVulnerability = 0;
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

        DefeatActor(RE::Actor* actor, DefeatIManager* defeatManager);
        ~DefeatActor();

        RE::FormID getActorFormId() const { return _actorFormId; };
        RE::Actor* getActor();
        void setActor(RE::Actor* actor);

        bool isSame(RE::Actor* actor) const;
        virtual bool isPlayer() { return false; };
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

        virtual float getVulnerability();
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
        DefeatIManager* _defeatManager;
        float _dynamicDefeat = 0;
        SpinLock* _dynamicDefeatSpinLock = nullptr;
    };

    class DefetPlayerActor : public DefeatActor {
    public:
        DefetPlayerActor(RE::Actor* actor, DefeatIManager* defeatManager);
        bool isPlayer() override { return true; };
        float getVulnerability() override;

        PapyrusInterface::ObjectPtr getLRGDefeatPlayerVulnerabilityScript() const;

    protected:
        PapyrusInterface::FloatVarPtr _LRGVulnerabilityVar = nullptr;
    };

    class DefeatActorManager : public SpinLock {
    public:
        DefeatActorManager(DefeatIManager* defeatManager) /* : SpinLock()*/ {
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
        SexLabDefeat::DefeatIManager* _defeatManager;
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
        DefeatActorType target = nullptr;
        DefeatActorType aggressor = nullptr;
        HitSource source;
        HitProjectile projectile;
        bool isPowerAttack = false;
        bool isSneakAttack = false;
        bool isBashAttack = false;
        bool isHitBlocked = false;
    };

    enum HitResult { SKIP, KNONKOUT, STANDING_STRUGGLE, KNONKDOWN };

    using HitEventType = HitEvent;

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

        DefeatCombatManager(DefeatActorManager* defeatActorManager, DefeatIManager* defeatManager);
        ~DefeatCombatManager();

        DefeatIManager* getDefeatManager() { return _defeatManager; };

        void onActorEnteredToCombatState(RE::Actor* target_actor);
        void onHitHandler(RawHitEvent event);
        HitEventType createHitEvent(RE::Actor* target_actor, RE::Actor* aggr_actor,
                                            RawHitEvent rawHitEvent);

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
        SexLabDefeat::DefeatIManager* _defeatManager;
        //std::unordered_map<HitSpamKey, std::chrono::high_resolution_clock::time_point, ProjectileSpamHash, HitSpamEqual>
        //    projectileSpamGuard;
        std::unordered_map<HitSpamKey, std::chrono::high_resolution_clock::time_point, ProjectileSpamHash, HitSpamEqual>
            hitSpamGuard;
        std::chrono::milliseconds _hitGuardExpiration;
        SpinLock* hitSpamGuardSpinLock;

        void onPlayerHitHandler(RawHitEvent event, SexLabDefeat::DefeatActorType defActor);
    };
    
    class DefeatIManager abstract {
    public:
        enum GameState {
            NONE,
            PRE_LOAD,
            IN_GAME,
        };

        struct {
            bool ZaZ = false;
            bool DeviousFramework = false;
            bool LRGPatch = false;
        } SoftDependency;

        DefeatForms Forms;

        virtual GameState getGameState() = 0;
        virtual void setGameState(GameState state) = 0;
                
        virtual void requestActorExtraData(DefeatActorType target) = 0;

        virtual DefeatWidget* getWidget() = 0;
        virtual DefeatCombatManager* getCombatManager() = 0;
        virtual DefeatActorManager* getActorManager() = 0;
        virtual DefeatConfig* getConfig() = 0;
    };
}