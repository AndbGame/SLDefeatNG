#pragma once

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

namespace SexLabDefeat {

    using time_point = std::chrono::high_resolution_clock::time_point;
    using milliseconds = std::chrono::milliseconds;
    using clock = std::chrono::high_resolution_clock;

    static const std::chrono::high_resolution_clock::time_point emptyTime =
        std::chrono::high_resolution_clock::time_point::min();

    namespace PapyrusInterface {

        template <class T> class ObjectVariable;

        using ObjectPtr = RE::BSTSmartPointer<RE::BSScript::Object>;

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
    class IDefeatActorManager;
    class IDefeatWidget;
    class DefeatActor;
    class IDefeatActor;
    class DefeatPlayerActor;
    using DefeatActorType = std::shared_ptr<DefeatActor>;
    using IDefeatActorType = std::shared_ptr<IDefeatActor>;
    using DefeatPlayerActorType = std::shared_ptr<DefeatPlayerActor>;

    class ActorExtraData {
    public:
        bool ignoreActorOnHit = true;
        int sexLabGender = 0;
        int sexLabSexuality = 0;
        bool sexLabAllowed = 0;
        std::string sexLabRaceKey = "";
        float DFWVulnerability = 0;
    };

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
    
    using HitSource = RE::FormID;
    using HitProjectile = RE::FormID;

    enum HitResult { SKIP, KNOCKOUT, STANDING_STRUGGLE, KNOCKDOWN };

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

    struct SoftDependencyType {
        bool ZaZ = false;
        bool DeviousFramework = false;
        bool BaboDialogue = false;
        bool LRGPatch = false;
    };

    /***************************************************************************************************
     * DefeatForms
     ****************************************************************************************************/
    struct DefeatForms {
        RE::TESQuest* DefeatPlayerQST = nullptr;
        RE::TESQuest* DefeatRessourcesQst = nullptr;
        RE::TESQuest* DefeatMCMQst = nullptr;
        RE::TESQuest* DefeatPlayerQTE = nullptr;

        RE::TESQuest* SexLabQuestFramework = nullptr;

        RE::SpellItem* SatisfiedSPL = nullptr;

        struct {
            RE::TESQuest* PAQst = nullptr;
            RE::TESQuest* PlayerActionQst = nullptr;
            RE::TESQuest* NPCsQst = nullptr;
            RE::TESQuest* NPCsRefreshQst = nullptr;
            RE::TESQuest* Robber = nullptr;
            RE::TESQuest* DGIntimidateQuest = nullptr;
            RE::TESQuest* WerewolfQst = nullptr;
        } MiscQuests;

        struct {
            RE::EffectSetting* ImmunityEFF = nullptr;
            RE::EffectSetting* HKActionEFF = nullptr;
            RE::EffectSetting* HKFollowerActionEFF = nullptr;
            RE::EffectSetting* SexCrimeEFF = nullptr;
            RE::EffectSetting* NVNAssaultEFF = nullptr;
        } MiscMagicEffects;

        struct {
            RE::TESQuest* DefeatVulnerability = nullptr;
            RE::BGSListForm* DynDefIgnoredWeaponList = nullptr;
        } LRGPatch;

        struct {
        } Keywords;

        struct {
            std::list<RE::TESFaction*> Factions;
        } Ignore;
    };

    /***************************************************************************************************
     * DefeatConfig
     ****************************************************************************************************/
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

            PapyrusInterface::BoolVarPtr OnOffPlayerVictim;

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
        milliseconds HIT_SPAM_GUARD_EXPIRATION_MS = 500ms;
        float KD_FAR_MAX_DISTANCE = 1500.0;
    private:
        boost::property_tree::ptree _iniConfig;
        std::map<std::string, std::variant<std::string, int, float, bool>> _config;

        SexLabDefeat::IDefeatManager* _defeatManager;

        PapyrusInterface::ObjectPtr DefeatMCMScr;
        PapyrusInterface::ObjectPtr defeatconfig;
        PapyrusInterface::ObjectPtr sslSystemConfig;
    };

    /***************************************************************************************************
    * IDefeatActor
    ****************************************************************************************************/
    class IDefeatActor {
    public:

        RE::FormID getTESFormId() const { return _data.TESFormId; }

        bool isSame(RE::Actor* actor) const {
            assert(actor != nullptr);
            return actor->GetFormID() == getTESFormId();
        };

        bool isSame(IDefeatActor* actor) const { return actor->getTESFormId() == getTESFormId(); };

        virtual bool isIgnored() { return false; };

        virtual bool isPlayer() { return false; };
        virtual bool isSurrender() { return _data.isSurrender; }

        bool hasHitImmunity() const { return clock::now() < _data.hitImmunityExpiration; }
        virtual void setHitImmunityFor(std::chrono::milliseconds ms) = 0;

        RE::FormID getLastHitAggressorFormId() const { return _data.lastHitAggressor; }
        virtual void setLastHitAggressor(DefeatActorType lastHitAggressor) = 0;

        DefeatActorStates getState() const { return _data.state; };
        virtual void setState(DefeatActorStates state) = 0;

        float getDynamicDefeat() const { return _data.dynamicDefeat; }
        virtual float incrementDynamicDefeat(float val) = 0;
        virtual float decrementDynamicDefeat(float val) = 0;
        virtual float resetDynamicDefeat() = 0;

        virtual float getVulnerability() { return _data.vulnerability; }
        virtual void setVulnerability(float vulnerability) = 0;

        virtual void requestExtraData(RE::Actor* TesActor, std::function<void()> callback, milliseconds timeoutMs) = 0;
        virtual void setExtraData(ActorExtraData data) = 0;
        bool isExtraDataExpired() const { return clock::now() > _data.extraDataExpiration; }
        virtual void setExtraDataExpirationFor(std::chrono::milliseconds ms) = 0;

        float getDFWVulnerability() const { return _data.DFWVulnerability; }
        virtual void setDFWVulnerability(float vulnerability) = 0;

        bool isIgnoreActorOnHit() const { return _data.ignoreActorOnHit; };
        virtual void setIgnoreActorOnHit(bool val) = 0;

        int getSexLabGender() const { return _data.sexLabGender; };
        virtual void setSexLabGender(int val) = 0;

        int getSexLabSexuality() const { return _data.sexLabSexuality; };
        virtual void setSexLabSexuality(int val) = 0;

        virtual bool isSexLabAllowed() { return _data.sexLabAllowed; }
        virtual void setSexLabAllowed(bool val) = 0;

        std::string getSexLabRaceKey() const { return _data.sexLabRaceKey; }
        virtual void setSexLabRaceKey(std::string val) = 0;

        bool isFemale() const { return getSexLabGender() == 1; }
        bool IsStraight() const { return getSexLabSexuality() >= 65; }
        bool IsGay() const { return getSexLabSexuality() <= 35; }
        bool IsBisexual() const {
            auto ratio = getSexLabSexuality();
            return (ratio < 65 && ratio > 35);
        }

        virtual bool isSheduledDeplateDynamicDefeat() = 0;
        virtual bool sheduleDeplateDynamicDefeat() = 0;
        virtual void stopDeplateDynamicDefeat() = 0;
        virtual bool registerAndCheckHitGuard(DefeatActorType aggressor, RE::FormID source, RE::FormID projectile) = 0;

        virtual IDefeatActorManager* getActorManager() = 0;

    protected:
        DefeatActorDataType _data;
    };

    /***************************************************************************************************
     * DefeatActor
     ****************************************************************************************************/
    class DefeatActor : public IDefeatActor {
        friend class DefeatActorManager;
        friend class IDefeatActorManager;

    public:
        DefeatActor(DefeatActorDataType data, RE::Actor* actor, IDefeatActorType impl);
        ~DefeatActor() {}

        bool isCreature();
        // TODO:
        bool isFollower() { return false; }
        bool isSatisfied();
        bool isKDImmune();
        bool isKDAllowed();
        bool isTied();
        bool isSexLabAllowed() override;
        bool isDefeatAllowed2PC();
        bool isDefeatAllowed2NvN();

        bool isIgnored() override;

        void setHitImmunityFor(std::chrono::milliseconds ms) override { _impl->setHitImmunityFor(ms); };
        void setLastHitAggressor(DefeatActorType lastHitAggressor) override {
            _impl->setLastHitAggressor(lastHitAggressor);
        }
        float incrementDynamicDefeat(float val) override {
            return _data.dynamicDefeat = _impl->incrementDynamicDefeat(val); 
        }
        float decrementDynamicDefeat(float val) override {
            return _data.dynamicDefeat = _impl->decrementDynamicDefeat(val);
        }
        float resetDynamicDefeat() override { 
            return _data.dynamicDefeat = _impl->resetDynamicDefeat(); 
        }
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
        void setExtraData(ActorExtraData data) override { _impl->setExtraData(data); }
        bool registerAndCheckHitGuard(DefeatActorType aggressor, RE::FormID source, RE::FormID projectile) override {
            return _impl->registerAndCheckHitGuard(aggressor, source, projectile);
        };

        bool isSheduledDeplateDynamicDefeat() { return _impl->isSheduledDeplateDynamicDefeat(); }
        bool sheduleDeplateDynamicDefeat() { return _impl->sheduleDeplateDynamicDefeat(); }
        void stopDeplateDynamicDefeat() { _impl->stopDeplateDynamicDefeat(); }

        IDefeatActorManager* getActorManager() override { return _impl->getActorManager(); };

    protected:
        RE::Actor* _actor;
        IDefeatActorType _impl;
        RE::Actor* getTESActor() { return _actor; }
    };

    /***************************************************************************************************
     * DefeatPlayerActor
     ****************************************************************************************************/
    class DefeatPlayerActor : public DefeatActor {
        friend class DefeatActorManager;
        friend class IDefeatActorManager;

    public:
        DefeatPlayerActor(DefeatActorDataType data, RE::Actor* actor, IDefeatActorType impl)
            : DefeatActor(data, actor, impl){};
        bool isPlayer() override { return true; };
        bool isSurrender() override { return _impl->isSurrender(); };
        float getVulnerability() override;
    };
        
    /***************************************************************************************************
     * IDefeatActorManager
     ****************************************************************************************************/
    class IDefeatActorManager abstract {
    public:
        virtual DefeatPlayerActorType getPlayer(RE::Actor* actor = nullptr) = 0;
        virtual DefeatActorType getDefeatActor(RE::Actor* actor) = 0;

        /* Pre Checks functions */
        virtual bool isIgnored(RE::Actor* actor) { return false; };
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
    };

    /***************************************************************************************************
     * IDefeatCombatManager
     ****************************************************************************************************/
    class IDefeatCombatManager {
    public:
        virtual void onActorEnteredToCombatState(RE::Actor* target_actor) = 0;
        virtual void onHitHandler(RawHitEvent event) = 0;
    };

    /***************************************************************************************************
     * IDefeatManager
     ****************************************************************************************************/
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

        virtual IDefeatWidget* getWidget() = 0;
        virtual IDefeatCombatManager* getCombatManager() = 0;
        virtual IDefeatActorManager* getActorManager() = 0;
        virtual DefeatConfig* getConfig() = 0;
    };

    /***************************************************************************************************
     * IDefeatWidget
     ****************************************************************************************************/
    class IDefeatWidget {
    public:
        enum State { QTE_METER, DYNAMIC_WIDGET, NONE };

        [[nodiscard]] 
        virtual bool setVisible(bool inUITask = false) = 0;
        [[nodiscard]] 
        virtual bool setInvisible(bool inUITask = false) = 0;
        virtual bool getLastVisible() = 0;
        [[nodiscard]] 
        virtual bool setPercent(float value, bool inUITask = false) = 0;
        virtual float getLastPercent() = 0;

        virtual State getState() = 0;
        [[nodiscard]]
        virtual bool startDynamicWidget(bool inUITask = false) = 0;
        [[nodiscard]] 
        virtual bool stopDynamicWidget(bool inUITask = false) = 0;
    };
}