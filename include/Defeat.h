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

    class IDefeatManager;

    struct SoftDependencyType {
        bool ZaZ = false;
        bool DeviousFramework = false;
        bool LRGPatch = false;
    };

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

            struct {
                RE::BSFixedString OnSLDefeatPlayerKnockDownEventName = "OnSLDefeatPlayerKnockDown";
            } PapyrusFunctionNames;
        } Config;

        void readIniConfig();
        void Setup(SexLabDefeat::IDefeatManager* defeatManager);
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

        SexLabDefeat::IDefeatManager* _defeatManager;

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
        RE::FormID formId;
        bool ignoreActorOnHit = true;
        int sexLabGender = 0;
        int sexLabSexuality = 0;
        bool sexLabAllowed = 0;
        std::string sexLabRaceKey = "";
        float DFWVulnerability = 0;
    };

    class DefeatActor;
    class DefeatPlayerActor;
    class IDefeatActorImpl;
    using DefeatActorType = std::shared_ptr<DefeatActor>;
    using DefeatPlayerActorType = std::shared_ptr<DefeatPlayerActor>;
    using DefeatActorImplType = std::shared_ptr<IDefeatActorImpl>;
    using DefeatPlayerActorImplType = std::shared_ptr<IDefeatActorImpl>;

    using HitSource = RE::FormID;
    using HitProjectile = RE::FormID;

    enum HitResult { SKIP, KNONKOUT, STANDING_STRUGGLE, KNONKDOWN };

    enum DefeatActorStates { NONE, ACTIVE, DISACTIVE, KNONKOUT_STATE, STANDING_STRUGGLE_STATE, KNONKDOWN_STATE };
    enum class DefeatActorStateFlags : uint8_t {
        NONE = 0,
        KNOCK_ALLOWED = 1 << 0,
        // flag2 = 1 << 1,
        // flag3 = 1 << 2,
        // flag3 = 1 << 3,
        // flag3 = 1 << 4,
        // flag3 = 1 << 5,
        // flag3 = 1 << 6,
        // flag3 = 1 << 7
    };
    struct DefeatActorDataType {
        RE::FormID TESFormId = 0;
        clock::time_point hitImmunityExpiration = SexLabDefeat::emptyTime;
        RE::FormID lastHitAggressor = 0;
        bool isSurrender = false;
        DefeatActorStates state = DefeatActorStates::ACTIVE;
        DefeatActorStateFlags flags = DefeatActorStateFlags::NONE;
        float dynamicDefeat = 0;
        float vulnerability = 0;

        /* External Papyrus Data */
        clock::time_point extraDataExpiration = SexLabDefeat::emptyTime;
        float DFWVulnerability = 0;
        bool ignoreActorOnHit = true;
        int sexLabGender = -1;
        int sexLabSexuality = -1;
        bool sexLabAllowed = false;
        std::string sexLabRaceKey = "";
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
    using HitEventType = HitEvent;


    class IDefeatActorManager : public SpinLock {
    public:
        virtual DefeatPlayerActorImplType getPlayerImpl() = 0;
        virtual DefeatPlayerActorType getPlayer(RE::Actor* actor = nullptr) = 0;
        virtual DefeatActorImplType getDefeatActorImpl(RE::Actor* actor) = 0;
        virtual DefeatActorType getDefeatActor(RE::Actor* actor) = 0;

        /* Pre Checks functions */
        virtual bool validForAggressorRole(RE::Actor* actor);
        virtual bool validForAggressorRoleOverPlayer(RE::Actor* actor);
        virtual bool validPlayerForVictimRole(RE::Actor* actor) = 0;
        /* / Pre Checks functions  */

        
        bool isDefeatAllowedByAgressor(DefeatActorType target, DefeatActorType aggressor);
        bool IsSexualAssaulAllowedByAggressor(DefeatActorType target, DefeatActorType aggressor);
        virtual bool hasSexInterestByAggressor(DefeatActorType target, DefeatActorType aggressor) = 0;
        virtual bool hasSexCombinationWithAggressor(DefeatActorType target, DefeatActorType aggressor) = 0;
        virtual bool checkAggressor(DefeatActorType target, DefeatActorType aggressor) = 0;
                
        virtual void playerKnockDownEvent(DefeatActorType target, DefeatActorType aggressor, HitResult event) = 0;

        float getDistanceBetween(DefeatActorType source, DefeatActorType target);
        float getHeadingAngleBetween(DefeatActorType source, DefeatActorType target);
        float getActorValuePercentage(DefeatActorType source, RE::ActorValue av);
        RE::TESForm* getEquippedHitSourceByFormID(DefeatActorType source, RE::FormID hitSource);
        bool wornHasAnyKeyword(DefeatActorType source, std::list<std::string> kwds);
        bool wornHasAnyKeyword(DefeatActor& source, std::list<std::string> kwds);
        bool hasKeywordString(DefeatActorType source, std::string kwd);
        bool hasKeywordString(DefeatActor& source, std::string kwd);
        bool notInFlyingState(DefeatActorType source);
        bool notInFlyingState(DefeatActor& source);
        bool hasSpell(DefeatActorType source, RE::SpellItem* spell);
        bool hasSpell(DefeatActor& source, RE::SpellItem* spell);
        bool hasMagicEffect(DefeatActorType source, RE::EffectSetting* effect);
        bool hasMagicEffect(DefeatActor& source, RE::EffectSetting* effect);
        bool isInKillMove(DefeatActorType source);
        bool isInKillMove(DefeatActor& source);
        bool isQuestEnabled(RE::TESQuest* quest);
        bool isInCombat(DefeatActorType source);

        virtual DefeatConfig* getConfig() = 0;
        virtual DefeatForms getForms() = 0;
        virtual SoftDependencyType getSoftDependency() = 0;

    protected:
        static RE::Actor* getTesActor(DefeatActorType source);
        static DefeatActorDataType getActorData(DefeatActorImplType source);
    };

    class IDefeatActor {
    public:

        RE::FormID getTESFormId() const { return _data.TESFormId; }

        bool isSame(RE::Actor* actor) const {
            assert(actor != nullptr);
            return actor->GetFormID() == getTESFormId();
        };

        bool isSame(IDefeatActor* actor) const { return actor->getTESFormId() == getTESFormId(); };

        virtual bool isPlayer() { return false; };
        bool isSurrender() { return _data.isSurrender; }

        bool hasHitImmunity() const { return clock::now() < _data.hitImmunityExpiration; }
        virtual void setHitImmunityFor(std::chrono::milliseconds ms) = 0;

        RE::FormID getLastHitAggressorFormId() { return _data.lastHitAggressor; }
        virtual void setLastHitAggressor(DefeatActorType lastHitAggressor) = 0;

        DefeatActorStates getState() { return _data.state; };
        virtual void setState(DefeatActorStates state) = 0;

        float getDynamicDefeat() { return _data.dynamicDefeat; }
        virtual void incrementDynamicDefeat(float val) = 0;
        virtual void decrementDynamicDefeat(float val) = 0;
        virtual void resetDynamicDefeat() = 0;

        virtual float getVulnerability() { return _data.vulnerability; }
        virtual void setVulnerability(float vulnerability) = 0;

        virtual void requestExtraData(RE::Actor* TesActor, std::function<void()> callback, milliseconds timeoutMs) = 0;
        virtual void responseExtraData(ActorExtraData data) = 0;
        bool isExtraDataExpired() const { return clock::now() > _data.extraDataExpiration; }
        virtual void setExtraDataExpirationFor(std::chrono::milliseconds ms) = 0;

        float getDFWVulnerability() { return _data.DFWVulnerability; }
        virtual void setDFWVulnerability(float vulnerability) = 0;

        bool isIgnoreActorOnHit() { return _data.ignoreActorOnHit; };
        virtual void setIgnoreActorOnHit(bool val) = 0;

        int getSexLabGender() { return _data.sexLabGender; };
        virtual void setSexLabGender(int val) = 0;

        int getSexLabSexuality() { return _data.sexLabSexuality; };
        virtual void setSexLabSexuality(int val) = 0;

        virtual bool isSexLabAllowed() { return _data.sexLabAllowed; }
        virtual void setSexLabAllowed(bool val) = 0;

        std::string getSexLabRaceKey() { return _data.sexLabRaceKey; }
        virtual void setSexLabRaceKey(std::string val) = 0;

        bool isFemale() { return getSexLabGender() == 1; }
        bool IsStraight() { return getSexLabSexuality() >= 65; }
        bool IsGay() { return getSexLabSexuality() <= 35; }
        bool IsBisexual() {
            auto ratio = getSexLabSexuality();
            return (ratio < 65 && ratio > 35);
        }

    protected:
        DefeatActorDataType _data;
    };

    class IDefeatActorImpl : public IDefeatActor, public SpinLock {
        friend class IDefeatActorManager;
    public:
        virtual IDefeatActorManager* getActorManager() = 0;
        virtual bool isSheduledDeplateDynamicDefeat() = 0;
        virtual bool sheduleDeplateDynamicDefeat() = 0;
        virtual void stopDeplateDynamicDefeat() = 0;
    };
    using IDefeatActorImplType = std::shared_ptr<IDefeatActorImpl>;

    class DefeatActor : public IDefeatActor {
        friend class IDefeatActorManager;

    public:
        DefeatActor(DefeatActorDataType data, RE::Actor* actor, IDefeatActorImplType impl) {
            assert(actor != nullptr);
            _data = data;
            _actor = actor;
            _impl = impl;
        };
        ~DefeatActor() {}

        bool isCreature() { return !_impl->getActorManager()->hasKeywordString(*this, "ActorTypeNPC"); }
        // TODO:
        bool isFollower() { return false; }
        bool isSatisfied() {
            return _impl->getActorManager()->hasSpell(*this, _impl->getActorManager()->getForms().SatisfiedSPL);
        }

        bool isKDImmune() {
            return _impl->getActorManager()->hasMagicEffect(
                *this, _impl->getActorManager()->getForms().MiscMagicEffects.ImmunityEFF);
        }
        bool isKDAllowed() {
            if (_impl->getActorManager()->isInKillMove(*this) || isKDImmune() ||
                _impl->getActorManager()->hasKeywordString(*this, "FavorBrawlEvent")) {
                //        SKSE::log::trace("isKDAllowed - false {} {} {}",
                //                         getActor()->IsInKillMove(), isKDImmune(),
                //                         actor->HasKeywordString("FavorBrawlEvent"));
                return false;
            }
            if (_impl->getActorManager()->isQuestEnabled(
                    _impl->getActorManager()->getForms().MiscQuests.DGIntimidateQuest)) {
                SKSE::log::trace("isKDAllowed - false DGIntimidateQuest");
                return false;
            }
            return true;
        }

        bool isTied() {
            if (_impl->getActorManager()->getSoftDependency().ZaZ) {
                return _impl->getActorManager()->wornHasAnyKeyword(
                    *this, std::list<std::string>{"zbfWornWrist", "DefeatWornDevice"});
            }
            return false;
        }
        bool isSexLabAllowed() override {
            if (!isCreature()) {
                return true;
            }
            return IDefeatActor::isSexLabAllowed();
        }

        bool isDefeatAllowed2PC() {
            bool ret = true;
            if (isCreature()) {
                std::string raceKey = getSexLabRaceKey();
                auto set = _impl->getActorManager()->getConfig()->Config.RaceAllowedPvic->get();
                if (auto search = set.find(raceKey); search == set.end()) {
                    ret = false;
                }
            }
            return ret;
        }

        bool isDefeatAllowed2NvN() {
            bool ret = true;
            if (isCreature()) {
                std::string raceKey = getSexLabRaceKey();
                auto set = _impl->getActorManager()->getConfig()->Config.RaceAllowedNVN->get();
                if (auto search = set.find(raceKey); search == set.end()) {
                    ret = false;
                }
            }
            return ret;
        }

        void setHitImmunityFor(std::chrono::milliseconds ms) override { _impl->setHitImmunityFor(ms); };

        void setLastHitAggressor(DefeatActorType lastHitAggressor) override {
            _impl->setLastHitAggressor(lastHitAggressor);
        }

        void incrementDynamicDefeat(float val) override { _impl->incrementDynamicDefeat(val); }

        void decrementDynamicDefeat(float val) override { _impl->decrementDynamicDefeat(val); }

        void resetDynamicDefeat() override { _impl->resetDynamicDefeat(); }

        void setState(DefeatActorStates state) override { _impl->setState(state); };

        void setVulnerability(float vulnerability) override { _impl->setVulnerability(vulnerability); };
        void setDFWVulnerability(float vulnerability) override { _impl->setDFWVulnerability(vulnerability); };

        void setIgnoreActorOnHit(bool val) override { _impl->setIgnoreActorOnHit(val); };

        void setSexLabGender(int val) override { _impl->setSexLabGender(val); };

        void setSexLabSexuality(int val) override { _impl->setSexLabSexuality(val); };

        void setSexLabAllowed(bool val) override { _impl->setSexLabAllowed(val); }

        void setSexLabRaceKey(std::string val) override { _impl->setSexLabRaceKey(val); }

        void setExtraDataExpirationFor(std::chrono::milliseconds ms) override { _impl->setExtraDataExpirationFor(ms); }
        void requestExtraData(RE::Actor* TesActor, std::function<void()> callback, milliseconds timeoutMs) override {
            _impl->requestExtraData(TesActor, callback, timeoutMs);
        }
        void responseExtraData(ActorExtraData data) override {
            _impl->responseExtraData(data);
        }

    protected:
        RE::Actor* _actor;
        IDefeatActorImplType _impl;


        RE::Actor* getTESActor() { return _actor; }
    };

    class DefeatPlayerActor : public DefeatActor {
        friend class DefeatActorManager;

    public:
        DefeatPlayerActor(DefeatActorDataType data, RE::Actor* actor, IDefeatActorImplType impl)
            : DefeatActor(data, actor, impl){};
        bool isPlayer() override { return true; };
        float getVulnerability() override;

        bool isSheduledDeplateDynamicDefeat() { return _impl->isSheduledDeplateDynamicDefeat(); }
        bool sheduleDeplateDynamicDefeat() { return _impl->sheduleDeplateDynamicDefeat(); }
        void stopDeplateDynamicDefeat() { _impl->stopDeplateDynamicDefeat(); }
    };

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

        DefeatCombatManager(IDefeatActorManager* defeatActorManager, IDefeatManager* defeatManager);
        ~DefeatCombatManager();

        IDefeatManager* getDefeatManager() { return _defeatManager; };

        void onActorEnteredToCombatState(RE::Actor* target_actor);
        void onHitHandler(RawHitEvent event);
        HitEventType createHitEvent(DefeatActorType target_actor, DefeatActorType aggr_actor,
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
        IDefeatActorManager* _defeatActorManager;
        IDefeatManager* _defeatManager;
        //std::unordered_map<HitSpamKey, std::chrono::high_resolution_clock::time_point, ProjectileSpamHash, HitSpamEqual>
        //    projectileSpamGuard;
        std::unordered_map<HitSpamKey, std::chrono::high_resolution_clock::time_point, ProjectileSpamHash, HitSpamEqual>
            hitSpamGuard;
        std::chrono::milliseconds _hitGuardExpiration;
        SpinLock* hitSpamGuardSpinLock;

        void onPlayerHitHandler(RawHitEvent event, DefeatPlayerActorType defActor);
    };
    
    class IDefeatManager abstract {
    public:
        enum GameState {
            NONE,
            PRE_LOAD,
            IN_GAME,
        };

        SoftDependencyType SoftDependency;

        DefeatForms Forms;

        virtual GameState getGameState() = 0;
        virtual void setGameState(GameState state) = 0;
                
        //virtual void requestActorExtraData(DefeatActorType target) = 0;

        virtual DefeatWidget* getWidget() = 0;
        virtual DefeatCombatManager* getCombatManager() = 0;
        virtual IDefeatActorManager* getActorManager() = 0;
        virtual DefeatConfig* getConfig() = 0;
    };
}