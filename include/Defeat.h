#pragma once

#include <DefeatSpinLock.h>
#include <DefeatUtils.h>
#include <DefeatPapyrus.h>

#include <DefeatActorManager.h>

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

    class DefeatManager;

    class DefeatConfig {
    public:
        DefeatConfig() = default;
        ~DefeatConfig() = default;
        DefeatConfig(DefeatConfig const&) = delete;
        void operator=(DefeatConfig const& x) = delete;

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
        void Setup(SexLabDefeat::DefeatManager* defeatManager);
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

        SexLabDefeat::DefeatManager* _defeatManager;

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

        DefeatCombatManager(DefeatActorManager* defeatActorManager,
                            DefeatManager* defeatManager);
        ~DefeatCombatManager();
        DefeatCombatManager(DefeatCombatManager const&) = delete;
        void operator=(DefeatCombatManager const& x) = delete;

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
        DefeatActorManager* _defeatActorManager;
        DefeatManager* _defeatManager;
        //std::unordered_map<HitSpamKey, std::chrono::high_resolution_clock::time_point, ProjectileSpamHash, HitSpamEqual>
        //    projectileSpamGuard;
        std::unordered_map<HitSpamKey, std::chrono::high_resolution_clock::time_point, ProjectileSpamHash, HitSpamEqual>
            hitSpamGuard;
        std::chrono::milliseconds _hitGuardExpiration;
        SpinLock* hitSpamGuardSpinLock;

        void onPlayerHitHandler(RawHitEvent event, DefeatPlayerActorType defActor);
    };

    void installHooks(SexLabDefeat::DefeatManager* defeatManager);
    void installEventSink(SexLabDefeat::DefeatManager* defeatManager);
}