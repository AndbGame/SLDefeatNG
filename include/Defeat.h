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

        using IntVar = ObjectVariable<std::int32_t>;
        using IntVarPtr = std::unique_ptr<IntVar>;
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
    using LastHitAggressorsType = std::map<RE::FormID, clock::time_point>;

    class ActorExtraData {
    public:
        bool ignoreActorOnHit = true;
        int sexLabGender = 0;
        int sexLabSexuality = 0;
        bool sexLabAllowed = 0;
        std::string sexLabRaceKey = "";
        float DFWVulnerability = 0;
    };

    enum DefeatActorStates {
        NONE,
        ACTIVE,
        DISACTIVE,

        IN_SCENE_STATE,

        VICTIM_KNONKOUT_STATE,
        VICTIM_STANDING_STRUGGLE_STATE,
        VICTIM_KNONKDOWN_STATE,
        VICTIM_TRAUMA_STATE,
        VICTIM_EXHAUSTED_STATE,
        VICTIM_SURRENDER_STATE,
        VICTIM_YIELD_STATE,
        VICTIM_ESCAPE_STATE,
        VICTIM_TIED_STATE,

        ASSAULT_STATE,
        ASSAULT_RAPE_STATE,
        ASSAULT_ROB_STATE,
        ASSAULT_KILL_STATE,
    };
    int format_as(DefeatActorStates f);

    enum class DefeatActorStateFlags : uint8_t {
        NONE = 0,
        KNOCK_ALLOWED = 1 << 0, // UNUSED
        STATE_TRANSITION = 1 << 1,
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
        LastHitAggressorsType lastHitAggressors = {};
        bool inCombat = false;
        bool isSurrender = false;
        DefeatActorStates state = DefeatActorStates::ACTIVE;
        SKSE::stl::enumeration<DefeatActorStateFlags, std::uint8_t> flags = DefeatActorStateFlags::NONE;
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
    int format_as(HitResult f);

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
            std::string_view DefeatActive = "DefeatActive";
            std::string_view DefeatAggPlayer = "DefeatAggPlayer";

            std::string_view SexLabActive = "SexLabActive";
            std::string_view ActorTypeNPC = "ActorTypeNPC";
        } KeywordId;

        struct {
            std::list<RE::TESFaction*> Factions;
            std::list<std::string_view> Keywords;
        } Ignore;

        struct {
            RE::TESFaction* CurrentFollowerFaction;
            RE::TESFaction* CurrentHireling;
            RE::TESFaction* DefeatFaction;
            RE::TESFaction* SexLabAnimatingFaction;
            RE::BGSListForm* EvilFactionList;
        } Faction;

        struct {
            RE::TESIdleForm* BleedoutStart;
        } Idle;
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
            PapyrusInterface::IntVarPtr NVNKDtype;
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

            PapyrusInterface::BoolVarPtr EveryoneNVN;
            PapyrusInterface::BoolVarPtr NPCLastEnemy;
            PapyrusInterface::BoolVarPtr AllowNPC;              // NPCs as victims
            PapyrusInterface::BoolVarPtr AllowCagg;             // Followers as aggressors
            PapyrusInterface::BoolVarPtr AllowCvic;             // Followers as victims
            PapyrusInterface::FloatVarPtr ThresholdNPCvsNPC;
            PapyrusInterface::FloatVarPtr ThresholdFollower;
            PapyrusInterface::FloatVarPtr ChanceOnHitNPC;
            PapyrusInterface::FloatVarPtr COHFollower;

            PapyrusInterface::BoolVarPtr OnOffPlayerAggressor;
            PapyrusInterface::BoolVarPtr OnOffPlayerVictim;
            PapyrusInterface::BoolVarPtr OnOffNVN;

            PapyrusInterface::BoolVarPtr HitInterrupt;          // Interrupt SL scene on Hit
            PapyrusInterface::BoolVarPtr CombatInterrupt;       // Interrupt SL scene on Combat start

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
                //RE::BSFixedString OnSLDefeatPlayerKnockDownEventName = "OnSLDefeatPlayerKnockDown"; // TODO: removed
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
        struct {
            int UpdateCombatControllerSettings = 0;
        } Hooks;
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
            // TODO: move code to DefeatActorManager
            assert(actor != nullptr);
            return actor->GetFormID() == getTESFormId();
        };

        bool isSame(DefeatActorType actor) const;

        virtual bool isIgnored() { return false; };
        virtual bool isDefeated() { return false; };

        virtual bool isPlayer() { return false; };
        virtual bool isSurrender() { return _data.isSurrender; }

        virtual bool inCombat() { return _data.inCombat; }
        virtual void setInCombat() = 0;
        virtual void setNotInCombat() = 0;

        bool IsDamageImmune() {}

        bool hasHitImmunity() const { return clock::now() < _data.hitImmunityExpiration; }
        virtual void setHitImmunityFor(std::chrono::milliseconds ms) = 0;

        RE::FormID getLastHitAggressorFormId() const { return _data.lastHitAggressor; }
        virtual DefeatActorType getLastHitAggressor() = 0;
        LastHitAggressorsType getLastHitAggressors() const { return _data.lastHitAggressors; };
        virtual void setLastHitAggressor(DefeatActorType lastHitAggressor) = 0;
        virtual void clearLastHitAggressors() = 0;

        DefeatActorStates getState() const { return _data.state; };
        virtual void setState(DefeatActorStates state) = 0;
        virtual bool tryExchangeState(DefeatActorStates oldState, DefeatActorStates newState) = 0;
        bool isStateTransition() { return _data.flags.any(DefeatActorStateFlags::STATE_TRANSITION); };
        virtual void setStateTransition(bool val) = 0;

        float getDynamicDefeat() const { return _data.dynamicDefeat; }
        virtual float incrementDynamicDefeat(float val) = 0;
        virtual float decrementDynamicDefeat(float val) = 0;
        virtual float resetDynamicDefeat() = 0;

        virtual float getVulnerability() { return _data.vulnerability; }
        virtual void setVulnerability(float vulnerability) = 0;

        virtual void requestExtraData(DefeatActorType actor, std::function<void()> callback,
                                      milliseconds timeoutMs) = 0;
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

    protected:
        DefeatActorDataType _data;
    };
        
    /***************************************************************************************************
     * IDefeatActorManager
     ****************************************************************************************************/
    class IDefeatActorManager abstract {
    public:
        virtual DefeatPlayerActorType getPlayer() = 0;
        virtual DefeatActorType getDefeatActor(RE::FormID formID) = 0;
        virtual DefeatActorType getDefeatActor(RE::Actor* actor) = 0;
        virtual DefeatActorType getDefeatActor(IDefeatActorType actor) = 0;
        virtual RE::Actor* getTESActor(DefeatActor* actor) = 0;
    };

    /***************************************************************************************************
     * IDefeatCombatManager
     ****************************************************************************************************/
    class IDefeatCombatManager {
    public:
        virtual void onActorEnteredToCombatState(RE::Actor* actor, RE::Actor* target_actor) = 0;
        virtual void onActorEnteredToNonCombatState(RE::Actor* actor) = 0;
        virtual void onHitHandler(RawHitEvent event) = 0;
        virtual void onActorEnterBleedout(RE::Actor* target_actor) = 0;
    };

    /***************************************************************************************************
     * DefeatScene
     ****************************************************************************************************/
    struct DefeatSceneResult {
        enum ResultStatus {
            SUCCESS,
            GENERAL_ERROR,
            VICTIM_NOT_SUITABLE,
            AGGRESSOR_NOT_FOUND,
            AGGRESSOR_NOT_READY } status = ResultStatus::SUCCESS;
        std::string reason = "";
    };

    class IDefeatScene {
    public:
        virtual DefeatSceneResult start() = 0;
        virtual std::string_view getUID() = 0;
    };

    using IDefeatSceneType = std::shared_ptr<IDefeatScene>;

    class IDefeatSceneManager {
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
        virtual IDefeatSceneManager* getSceneManager() = 0;
        virtual IDefeatActorManager* getActorManager() = 0;
        virtual DefeatConfig* getConfig() = 0;

        virtual bool hasTraceLog() = 0;
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