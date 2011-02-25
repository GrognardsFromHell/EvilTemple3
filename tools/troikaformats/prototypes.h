#ifndef PROTOTYPES_H
#define PROTOTYPES_H

#include "troikaformatsglobal.h"

#include <QObject>
#include <QMap>
#include <QStringList>
#include <QString>

namespace Troika
{

    class VirtualFileSystem;
    class PrototypesData;

    template<typename T>
    class Property {
    public:
        Property() : mDefined(false) {}

        T value() const { return mValue; }
        bool isDefined() const { return mDefined; }

        void setValue(T value) {
            mDefined = true;
            mValue = value;
        }

        void clear() {
            mDefined = false;
        }

        bool operator ==(const Property &value) {
            return (!mDefined && !value.mDefined)
                    || (mDefined == value.mDefined && mValue == value.mValue);
        }

    private:
        bool mDefined;
        T mValue;
    };

    class Float : public Property<float> {};
    class Integer : public Property<int> {};
    class Bool : public Property<bool> {};

    enum ObjectType
    {
        Portal = 0,
        Container,
        Scenery,
        Projectile,
        Weapon,
        Ammo,
        Armor,
        Money,
        Food,
        Scroll,
        Key,
        Written,
        Generic,
        PlayerCharacter,
        NonPlayerCharacter,
        Trap,
        Bag,
        ObjectTypeCount
    };

    class TROIKAFORMATS_EXPORT ScriptAttachment {
    public:
        QString event; // The event to which this script is attached
        uint scriptId; // The id of the attached script
        Integer parameter; // This additional parameter is only used by traps
    };

    class TROIKAFORMATS_EXPORT AdditionalProperties : public QObject
    {
    Q_OBJECT
    public:
        AdditionalProperties(QObject *parent) : QObject(parent) {}

        virtual void parse(const QStringList &parts) = 0;
    };

    class TROIKAFORMATS_EXPORT ScriptProperties : public AdditionalProperties
    {
    Q_OBJECT
    public:
        ScriptProperties(QObject *parent) : AdditionalProperties(parent) {}

        void parse(const QStringList &parts);

        QList<ScriptAttachment> scripts;
    };

    class TROIKAFORMATS_EXPORT PortalProperties : public ScriptProperties
    {
    Q_OBJECT
    public:
        PortalProperties(QObject *parent) : ScriptProperties(parent) {}

        void parse(const QStringList &parts);

        /*
         None of these properties seem to be used.
         */
        QStringList flags; // 37
        Integer lockDc; // 38
        Integer keyId; // 39
        Integer notifyNpc; // 40
    };

    class TROIKAFORMATS_EXPORT ContainerProperties : public ScriptProperties
    {
    Q_OBJECT
    public:
        ContainerProperties(QObject *parent) : ScriptProperties(parent) {}

        void parse(const QStringList &parts);

        bool locked; // 41
        Integer lockDc; // 42
        Integer keyId; // 43
        Integer inventorySource; // 44, invensource.mes
        Integer notifyNpc; // 45, Seems unused
    };

    class TROIKAFORMATS_EXPORT SceneryProperties : public ScriptProperties
    {
    Q_OBJECT
    public:
        SceneryProperties(QObject *parent) : ScriptProperties(parent) {}

        void parse(const QStringList &parts);

        QStringList flags; // 46
        Integer respawnDelay; // 47, Seems unused
    };

    class TROIKAFORMATS_EXPORT ProjectileProperties : public ScriptProperties
    {
    Q_OBJECT
    public:
        ProjectileProperties(QObject *parent) : ScriptProperties(parent) {}

        void parse(const QStringList &parts);

        QStringList flags; // 48, Unused
        QStringList dmgFlags; // 49, Unused
    };

    class TROIKAFORMATS_EXPORT AdditionalProperty
    {
    public:
        QString type;
        QString param1;
        QString param2;
    };

    /**
      Represents a spell known by a critter or imbued in an item or weapon.
      */
    class TROIKAFORMATS_EXPORT KnownSpell
    {
    public:
        QString name;
        QString source; // Class, or "domain_special" for instance
        uint level; // Spell level
    };

    class TROIKAFORMATS_EXPORT EntityProperties : public ScriptProperties
    {
    Q_OBJECT
    public:
        EntityProperties(QObject *parent) : ScriptProperties(parent) {}

        void parse(const QStringList &parts);

        QList<AdditionalProperty> properties;
        QList<KnownSpell> spells;
    };

    class TROIKAFORMATS_EXPORT ItemProperties : public EntityProperties
    {
    Q_OBJECT
    public:
        ItemProperties(QObject *parent) : EntityProperties(parent) {}

        void parse(const QStringList &parts);

        QStringList flags; // 50
        Integer weight; // 51
        Integer worth; // 52, in copper
        Integer inventoryIcon; // 53
        Integer inventoryGroundMesh; // 54, Unused
        Integer unidentifiedDescriptionId; // 55
        Integer longDescriptionId; // 56, unused
        Integer spellIndex; // 57, unused
        Integer spellIndexFlags; // 58, unused
        Integer chargesLeft; // 59
        Integer aiAction; // 60, Unused
        QStringList equipSlots; // 61
        bool twoHanded; // 62
        Integer wearMeshId; // 62
    };

    class TROIKAFORMATS_EXPORT WeaponProperties : public ItemProperties
    {
    Q_OBJECT
    public:
        WeaponProperties(QObject *parent) : ItemProperties(parent) {}

        void parse(const QStringList &parts);

        QStringList flags; // 63
        Integer range; // 64
        QString ammoType; // 65
        Integer ammoConsumption; // 66, Unused
        Integer missileAnimationId; // 67
        Integer criticalHitMultiplier; // 68
        QString damageType; // 69
        QString damageDice; // 70
        // 71, Animation type (unused)
        QString weaponClass; // 72, used for feats
        Integer threatRange; // 73

    };

    class TROIKAFORMATS_EXPORT AmmoProperties : public ItemProperties
    {
    Q_OBJECT
    public:
        AmmoProperties(QObject *parent) : ItemProperties(parent) {}

        void parse(const QStringList &parts);

        Integer quantity; // 75
        QString type; // 76
    };

    class TROIKAFORMATS_EXPORT ArmorProperties : public ItemProperties
    {
    Q_OBJECT
    public:
        ArmorProperties(QObject *parent) : ItemProperties(parent) {}

        void parse(const QStringList &parts);

        Integer maxDexterityBonus; // 79
        Integer arcaneSpellFailure; // 80
        Integer skillCheckPenalty; // 81
        QString armorType; // 82
        QString helmetType; // 83
    };

    class TROIKAFORMATS_EXPORT MoneyProperties : public ItemProperties
    {
    Q_OBJECT
    public:
        MoneyProperties(QObject *parent) : ItemProperties(parent) {}

        void parse(const QStringList &parts);

        uint quantity;
        QString type;
    };

    class TROIKAFORMATS_EXPORT FoodProperties : public ItemProperties
    {
    Q_OBJECT
    public:
        FoodProperties(QObject *parent) : ItemProperties(parent) {}

        void parse(const QStringList &parts);
    };

    class TROIKAFORMATS_EXPORT ScrollProperties : public ItemProperties
    {
    Q_OBJECT
    public:
        ScrollProperties(QObject *parent) : ItemProperties(parent) {}

        void parse(const QStringList &parts);
    };

    class TROIKAFORMATS_EXPORT KeyProperties : public ItemProperties
    {
    Q_OBJECT
    public:
        KeyProperties(QObject *parent) : ItemProperties(parent) {}

        void parse(const QStringList &parts);

        uint keyId;
    };

    class TROIKAFORMATS_EXPORT WrittenProperties : public ItemProperties
    {
    Q_OBJECT
    public:
        WrittenProperties(QObject *parent) : ItemProperties(parent) {}

        void parse(const QStringList &parts);

        // Flags are unused
        uint subtype;
        Integer startLine;
        // EndLine is unused
    };

    class TROIKAFORMATS_EXPORT BagProperties : public ItemProperties
    {
    Q_OBJECT
    public:
        BagProperties(QObject *parent) : ItemProperties(parent) {}

        void parse(const QStringList &parts);

        QStringList flags;
        Integer size;
    };

    class TROIKAFORMATS_EXPORT GenericProperties : public ItemProperties
    {
    Q_OBJECT
    public:
        GenericProperties(QObject *parent) : ItemProperties(parent) {}

        void parse(const QStringList &parts);
    };

    class TROIKAFORMATS_EXPORT NaturalAttack
    {
    public:
        uint numberOfAttacks;
        QString damageDice;
        QString type;
        int attackBonus;
    };

    class TROIKAFORMATS_EXPORT ClassLevel {
    public:
        QString name;
        uint count; // min: 1
    };

    class TROIKAFORMATS_EXPORT SkillLevel {
    public:
        QString name;
        int count;
    };

    class TROIKAFORMATS_EXPORT CritterProperties : public EntityProperties
    {
    Q_OBJECT
    public:
        CritterProperties(QObject *parent) : EntityProperties(parent) {}

        void parse(const QStringList &parts);

        QStringList flags; // 99
        // 2nd flag field unused
        int strength, dexterity, constitution, intelligence, wisdom, charisma;
        // levels unused
        QString race;
        QString gender;
        Integer age;
        // Height: unused
        // Weight: unused
        QString alignment;
        QString deity;
        QStringList domains;
        QString alignmentChoice;
        Integer portraitId;
        Integer unknownDescription; // If not yet talked to the npc
        Integer reach;
        QList<NaturalAttack> naturalAttacks; // at most 4 different ones
        QString hairColor;
        QString hairType;

        QList<ClassLevel> classLevels;
        QList<SkillLevel> skills;
        QStringList feats;

        QString levelUpScheme; // Defines auto-leveling stuff (which feats to take, etc.)
        QString strategy; // Which AI is used in auto fighting situations
    };

    class TROIKAFORMATS_EXPORT NonPlayerCharacterProperties : public CritterProperties
    {
    Q_OBJECT
    public:
        NonPlayerCharacterProperties(QObject *parent) : CritterProperties(parent) {}

        void parse(const QStringList &parts);

        QStringList flags; // 152
        Integer aiData;
        QList<uint> factions;
        // Retail price multiplier (unused)
        // Reaction base (unused)
        QString challengeRating; // Sometimes given as 1/3, etc. Only used as table-lookup anyhow
        Integer reflexSave;
        Integer fortitudeSave;
        Integer willpowerSave;
        Integer acBonus;
        QString hitDice;
        QString type;
        QStringList subTypes;
        QString lootShareAmount;
        Integer additionalMeshId; // An addmesh that should always be present (used for bugbear equipment)
    };

    class TROIKAFORMATS_EXPORT PlayerCharacterProperties : public NonPlayerCharacterProperties
    {
    Q_OBJECT
    public:
        PlayerCharacterProperties(QObject *parent) : NonPlayerCharacterProperties(parent) {}

        void parse(const QStringList &parts);
    };

    class TROIKAFORMATS_EXPORT TrapProperties : public ItemProperties
    {
    Q_OBJECT
    public:
        TrapProperties(QObject *parent) : ItemProperties(parent) {}

        void parse(const QStringList &parts);
    };

    class TROIKAFORMATS_EXPORT Prototype : public QObject
    {
    Q_OBJECT
    public:
        explicit Prototype(int id, QObject *parent = 0);

        void parse(const QStringList &parts);

        uint id;
        ObjectType type; // Type is mandatory
        Float scale;
        QStringList objectFlags;
        Integer internalDescriptionId;
        uint descriptionId;
        QString objectSize; // May be null
        Integer hitPoints;
        QString objectMaterial; // May be null
        Integer soundId;
        Integer categoryId;
        Float rotation;
        Float walkSpeedFactor;
        Float runSpeedFactor;
        quint16 modelId;
        Float radius;
        Float renderHeight;
        Bool dontDraw;
        Bool disabled;
        Bool interactive;
        Bool unlit;

        AdditionalProperties *additionalProperties;

        Q_DISABLE_COPY(Prototype)
    };

    class TROIKAFORMATS_EXPORT Prototypes : public QObject
    {
    Q_OBJECT
    public:
        explicit Prototypes(VirtualFileSystem *vfs, QObject *parent = 0);
        ~Prototypes();

        Prototype *get(int id) const;

        const QMap<uint, Prototype*> &prototypes() const;

    signals:

    public slots:

    private:
        QScopedPointer<PrototypesData> d_ptr;
        Q_DISABLE_COPY(Prototypes);

    };

}

#endif // PROTOTYPES_H
