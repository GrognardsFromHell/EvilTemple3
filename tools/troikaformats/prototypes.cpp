#include <QString>
#include <QMap>
#include <QHash>
#include <QSet>

#include "prototypes.h"
#include "virtualfilesystem.h"
#include "util.h"

namespace Troika
{
    const QString PrototypesFile = "rules/protos.tab";

    static uint currentPrototypeId; // Only used for debug output

    class PrototypesData
    {
    public:
        VirtualFileSystem *vfs;
        QMap<uint,Prototype*> prototypes;

        QHash<QString,QStringList> extraFlags;

        void loadExtraFlags()
        {
            QFile f(":/prototypes_flags.txt");

            if (!f.open(QIODevice::Text|QIODevice::ReadOnly)) {
                qWarning("Unable to open extra flags for prototypes.");
                return;
            }

            while (!f.atEnd()) {
                QString line = f.readLine().trimmed();

                if (line.isEmpty())
                    continue;
                QStringList lineParts = line.split(' ');
                if (lineParts.size() != 2) {
                    qWarning("Invalid line: %s", qPrintable(line));
                    continue;
                }

                extraFlags[lineParts[0]] = lineParts[1].split(';');
            }
        }

        void load(QObject *parent)
        {
            loadExtraFlags();

            QByteArray data = vfs->openFile(PrototypesFile);
            QList<QByteArray> lines = data.split('\n');

            foreach (QByteArray rawline, lines)
            {
                QString line(rawline);

                if (line.trimmed().isEmpty())
                    continue;

                QStringList parts = line.split('\t', QString::KeepEmptyParts);

                // Skip lines with less than 100 parts
                if (parts.size() < 100)
                {
                    qWarning("Skipping prototype: %s", qPrintable(line));
                    continue;
                }

                bool ok;

                uint id = parts[0].toUInt(&ok);

                if (!ok) {
                    qWarning("Unable to parse id of prototype: %s", qPrintable(parts[0]));
                    continue;
                }

                currentPrototypeId = id;

                Prototype *prototype = new Prototype(id, parent);
                prototype->parse(parts);

                if (extraFlags.contains(QString("%1").arg(id)) && prototype->objectFlags.isEmpty()) {
                    prototype->objectFlags = extraFlags[QString("%1").arg(id)];
                }

                prototypes[id] = prototype;
            }
        }
    };

    Prototypes::Prototypes(VirtualFileSystem *vfs, QObject *parent) :
            QObject(parent), d_ptr(new PrototypesData)
    {
        d_ptr->vfs = vfs;
        d_ptr->load(this);
    }

    Prototypes::~Prototypes()
    {
        qDeleteAll(d_ptr->prototypes);
    }

    Prototype *Prototypes::get(int id) const
    {
        return d_ptr->prototypes[id];
    }

    Prototype::Prototype(int _id, QObject *parent) : QObject(parent), id(_id),
    additionalProperties(0)
    {
        modelId = 0;
    }

    inline bool isPartDefined(const QString &part)
    {
        return !part.trimmed().isEmpty();
    }

    inline QString convertObjectSize(const QString &objectSize) {
        static QHash<QString,QString> mapping;

        if (mapping.isEmpty()) {
            mapping["size_none"] = "None";
            mapping["size_fine"] = "Fine";
            mapping["size_diminut"] = "Diminutive";
            mapping["size_diminutive"] = "Diminutive";
            mapping["size_tiny"] = "Tiny";
            mapping["size_small"] = "Small";
            mapping["size_medium"] = "Medium";
            mapping["size_large"] = "Large";
            mapping["size_huge"] = "Huge";
            mapping["size_gargantuan"] = "Gargantuan";
            mapping["size_colossal"] = "Colossal";
        }

        Q_ASSERT(mapping.contains(objectSize));
        return mapping[objectSize];
    }

    inline QString convertObjectMaterial(const QString &material) {
        static QHash<QString,QString> mapping;

        if (mapping.isEmpty()) {
            mapping["mat_powder"] = "Powder";
            mapping["mat_fire"] = "Fire";
            mapping["mat_force"] = "Force";
            mapping["mat_gas"] = "Gas";
            mapping["mat_paper"] = "Paper";
            mapping["mat_liquid"] = "Liquid";
            mapping["mat_cloth"] = "Cloth";
            mapping["mat_glass"] = "Glass";
            mapping["mat_metal"] = "Metal";
            mapping["mat_flesh"] = "Flesh";
            mapping["mat_plant"] = "Plant";
            mapping["mat_wood"] = "Wood";
            mapping["mat_brick"] = "Brick";
            mapping["mat_stone"] = "Stone";
        }

        Q_ASSERT(mapping.contains(material));
        return mapping[material];
    }

    inline void optionalPart(const QStringList &parts, int offset, Integer &field) {
        if (isPartDefined(parts[offset])) {
            bool ok;
            int value = parts[offset].toInt(&ok);
            if (!ok) {
                qWarning("Optional integer field @ %d has invalid value: %s.", offset, qPrintable(parts[offset]));
            } else {
                field.setValue(value);
            }
        }
    }

    inline void optionalPart(const QStringList &parts, int offset, Float &field) {
        if (isPartDefined(parts[offset])) {
            bool ok;
            float value = parts[offset].toFloat(&ok);
            if (!ok) {
                qWarning("Optional float field @ %d has invalid value: %s.", offset, qPrintable(parts[offset]));
            } else {
                field.setValue(value);
            }
        }
    }

    inline QStringList splitFlagList(const QStringList &parts, int offset) {
        QString flagList = parts[offset].trimmed();

        // Co8 bugfix
        flagList.replace("OIF ", "OIF_");

        QStringList list = flagList.split(QRegExp("[\x0B\\ \\,\x7F]"), QString::SkipEmptyParts);

        // remove duplicates
        for (int i = 0; i < list.size(); ++i) {
            for (int j = i + 1; j < list.size(); ++j) {
                if (list[i] == list[j]) {
                    list.removeAt(j--);
                }
            }
        }

        return list;
    }

    typedef QString (*TranslatorFn)(const QString &objectFlag);

    static QString translateObjectFlag(const QString &objectFlag)
    {
        static QHash<QString,QString> mapping;

        if (mapping.isEmpty()) {
            mapping["OF_CLICK_THROUGH"] = "ClickThrough";
            mapping["OF_DONTDRAW"] = "DontDraw";
            mapping["OF_DONTLIGHT"] = "DontLight";
            mapping["OF_INVISIBLE"] = "Invisible";
            mapping["OF_INVULNERABLE"] = "Invulnerable";
            mapping["OF_NO_BLOCK"] = "NoBlock";
            mapping["OF_NOHEIGHT"] = "NoHeight";
            mapping["OF_OFF"] = "Off";
            mapping["OF_RANDOM_SIZE"] = "RandomSize";
            mapping["OF_SEE_THROUGH"] = "SeeThrough";
            mapping["OF_SHOOT_THROUGH"] = "ShootThrough";
            mapping["OF_WADING"] = "Wading";
            mapping["OF_WATER_WALKING"] = "WaterWalking";
            mapping["OF_HEIGHT_SET"] = "";
            mapping["OF_RADIUS_SET"] = "";
            mapping["OF_PROVIDES_COVER"] = "ProvidesCover";
        }

        Q_ASSERT_X(mapping.contains(objectFlag), "convertObjectFlag", qPrintable(objectFlag));
        return mapping[objectFlag];
    }

    inline void readFlagList(const QStringList &parts, int offset, QStringList &result, TranslatorFn translate)
    {
        QStringList flagList = splitFlagList(parts, offset);
        for (int i = 0; i < flagList.size(); ++i) {
            flagList[i] = translate(flagList[i]);
            if (flagList[i].isEmpty())
                flagList.removeAt(i--);
        }
        result = flagList;
    }


    inline void readFlagSet(const QStringList &parts, int offset, QStringList &result, TranslatorFn translate)
    {
        QStringList flagList = splitFlagList(parts, offset);
        for (int i = 0; i < flagList.size(); ++i) {
            flagList[i] = translate(flagList[i]);
            if (flagList[i].isEmpty())
                flagList.removeAt(i--);
        }
        result = QStringList(flagList.toSet().toList());
    }

    inline ObjectType convertObjectType(const QString &objectType)
    {
        static QHash<QString,ObjectType> mapping;

        if (mapping.isEmpty()) {
            mapping["obj_t_portal"] = Portal;
            mapping["obj_t_container"] = Container;
            mapping["obj_t_scenery"] = Scenery;
            mapping["obj_t_projectile"] = Projectile;
            mapping["obj_t_weapon"] = Weapon;
            mapping["obj_t_ammo"] = Ammo;
            mapping["obj_t_armor"] = Armor;
            mapping["obj_t_money"] = Money;
            mapping["obj_t_food"] = Food;
            mapping["obj_t_scroll"] = Scroll;
            mapping["obj_t_key"] = Key;
            mapping["obj_t_written"] = Written;
            mapping["obj_t_generic"] = Generic;
            mapping["obj_t_pc"] = PlayerCharacter;
            mapping["obj_t_npc"] = NonPlayerCharacter;
            mapping["obj_t_trap"] = Trap;
            mapping["obj_t_bag"] = Bag;
        }

        Q_ASSERT(mapping.contains(objectType));
        return mapping[objectType];
    }

    void Prototype::parse(const QStringList &parts)
    {
        bool ok;

        Q_ASSERT(isPartDefined(parts[1]));
        type = convertObjectType(parts[1]);

        if (isPartDefined(parts[6])) {
            QString scaleText = parts[6];
            scaleText.replace(".", ""); // Sometimes the scale is incorrectly defined as .25,
            // instead of 25 (at least i hope that)
            scale.setValue(scaleText.toFloat());
        }

        readFlagList(parts, 20, objectFlags, translateObjectFlag);
        if (objectFlags.removeAll("DontDraw") > 0)
            dontDraw.setValue(true);
        if (objectFlags.removeAll("Off") > 0)
            disabled.setValue(true);
        if (objectFlags.removeAll("ClickThrough") > 0)
            interactive.setValue(false);
        if (objectFlags.removeAll("DontLight") > 0)
            unlit.setValue(false);

        // 21: Spell Flags -> Unused

        optionalPart(parts, 22, internalDescriptionId);

        descriptionId = parts[23].toInt(&ok);
        Q_ASSERT(ok);

        if (isPartDefined(parts[24]))
            objectSize = convertObjectSize(parts[24]);

        optionalPart(parts, 25, hitPoints);

        if (isPartDefined(parts[27]))
            objectMaterial = convertObjectMaterial(parts[27]);

        optionalPart(parts, 29, soundId);

        optionalPart(parts, 30, categoryId);
        if (categoryId.isDefined())
            categoryId.setValue(categoryId.value() + type * 1000);

        optionalPart(parts, 31, rotation);
        if (rotation.isDefined()) {
            rotation.setValue(rad2deg(LegacyBaseRotation + rotation.value()));
        }

        optionalPart(parts, 32, walkSpeedFactor);

        optionalPart(parts, 33, runSpeedFactor);

        Q_ASSERT(isPartDefined(parts[34])); // Model id is required.
        modelId = parts[34].toInt();

        optionalPart(parts, 35, radius);

        optionalPart(parts, 36, renderHeight);

        switch (type) {
        case Portal:
            additionalProperties = new PortalProperties(this);
            break;
        case Container:
            additionalProperties = new ContainerProperties(this);
            break;
        case Scenery:
            additionalProperties = new SceneryProperties(this);
            break;
        case Projectile:
            additionalProperties = new ProjectileProperties(this);
            break;
        case Weapon:
            additionalProperties = new WeaponProperties(this);
            break;
        case Ammo:
            additionalProperties = new AmmoProperties(this);
            break;
        case Armor:
            additionalProperties = new ArmorProperties(this);
            break;
        case Money:
            additionalProperties = new MoneyProperties(this);
            break;
        case Food:
            additionalProperties = new FoodProperties(this);
            break;
        case Scroll:
            additionalProperties = new ScrollProperties(this);
            break;
        case Key:
            additionalProperties = new KeyProperties(this);
            break;
        case Written:
            additionalProperties = new WrittenProperties(this);
            break;
        case Bag:
            additionalProperties = new BagProperties(this);
            break;
        case Generic:
            additionalProperties = new GenericProperties(this);
            break;
        case PlayerCharacter:
            additionalProperties = new PlayerCharacterProperties(this);
            break;
        case NonPlayerCharacter:
            additionalProperties = new NonPlayerCharacterProperties(this);
            break;
        case Trap:
            additionalProperties = new TrapProperties(this);
            break;
        default:
            qFatal("Invalid object type encountered: %d", type);
        };

        additionalProperties->parse(parts);
    }

    void PortalProperties::parse(const QStringList &parts)
    {
        ScriptProperties::parse(parts);
    }

    void ContainerProperties::parse(const QStringList &parts)
    {
        ScriptProperties::parse(parts);

        locked = parts[41].trimmed() == "OCOF_LOCKED";

        optionalPart(parts, 42, lockDc);
        optionalPart(parts, 43, keyId);
        optionalPart(parts, 44, inventorySource);
    }

    inline QString convertSceneryFlag(const QString &objectType)
    {
        static QHash<QString,QString> mapping;

        if (mapping.isEmpty()) {
            mapping["OSCF_NO_AUTO_ANIMATE"] = "NoAutoAnimate";
            mapping["OSCF_SOUND_EXTRA_LARGE"] = "SoundExtraLarge";
        }

        Q_ASSERT(mapping.contains(objectType));
        return mapping[objectType];
    }

    void SceneryProperties::parse(const QStringList &parts)
    {
        ScriptProperties::parse(parts);

        readFlagList(parts, 46, flags, convertSceneryFlag);
    }

    void ProjectileProperties::parse(const QStringList &parts)
    {
        ScriptProperties::parse(parts);
    }

    inline QString convertWeaponFlag(const QString &weaponFlag)
    {
        static QHash<QString,QString> mapping;

        if (mapping.isEmpty()) {
            mapping["OWF_DEFAULT_THROWS"] = "DefaultThrows";
            mapping["OWF_MAGIC_STAFF"] = "MagicStaff";
            mapping["OWF_RANGED_WEAPON"] = "RangedWeapon";
            mapping["OWF_THROWABLE"] = "Throwable";
            mapping["OWF_TWO_HANDED"] = "TwoHanded";
        }

        Q_ASSERT(mapping.contains(weaponFlag));
        return mapping[weaponFlag];
    }

    inline QString convertDamageType(const QString &damageType)
    {
        return damageType;
    }

    inline QString convertWeaponClass(const QString &weaponClass)
    {
        return weaponClass;
    }

    void WeaponProperties::parse(const QStringList &parts)
    {
        ItemProperties::parse(parts);

        readFlagList(parts, 63, flags, convertWeaponFlag);
        optionalPart(parts, 64, range);
        if (isPartDefined(parts[65]))
            ammoType = parts[65].trimmed(); // Further processing?
        optionalPart(parts, 67, missileAnimationId);
        optionalPart(parts, 68, criticalHitMultiplier);
        if (isPartDefined(parts[69]))
            damageType = convertDamageType(parts[69]);
        if (isPartDefined(parts[70]))
            damageDice = parts[70];
        if (isPartDefined(parts[72]))
            weaponClass = convertWeaponClass(parts[72]);
        optionalPart(parts, 73, threatRange);
    }

    void AmmoProperties::parse(const QStringList &parts)
    {
        ItemProperties::parse(parts);

        optionalPart(parts, 74, quantity);
        type = parts[75];
    }

    void ArmorProperties::parse(const QStringList &parts)
    {
        ItemProperties::parse(parts);

        optionalPart(parts, 79, maxDexterityBonus);
        optionalPart(parts, 80, arcaneSpellFailure);
        optionalPart(parts, 81, skillCheckPenalty);
        armorType = parts[82];
        helmetType = parts[83];
    }

    void MoneyProperties::parse(const QStringList &parts)
    {
        ItemProperties::parse(parts);

        bool ok;
        quantity = parts[85].toUInt(&ok);
        Q_ASSERT(ok);
        type = parts[86];
    }

    void FoodProperties::parse(const QStringList &parts)
    {
        ItemProperties::parse(parts);
    }

    void ScrollProperties::parse(const QStringList &parts)
    {
        ItemProperties::parse(parts);
    }

    void KeyProperties::parse(const QStringList &parts)
    {
        ItemProperties::parse(parts);

        bool ok;
        keyId = parts[89].toUInt(&ok);
        Q_ASSERT(ok);
    }

    void WrittenProperties::parse(const QStringList &parts)
    {
        ItemProperties::parse(parts);

        bool ok;
        subtype = parts[91].toUInt(&ok);
        Q_ASSERT(ok);

        optionalPart(parts, 92, startLine);
    }

    inline QString convertBagFlag(const QString &bagFlag)
    {
        static QHash<QString,QString> mapping;

        if (mapping.isEmpty()) {
            mapping["OBF_HOLDING_500"] = "Holding500";
            mapping["OBF_HOLDING_1000"] = "Holding1000";
            mapping["OBF_HOLDING"] = "Holding";
        }

        Q_ASSERT(mapping.contains(bagFlag));
        return mapping[bagFlag];
    }

    void BagProperties::parse(const QStringList &parts)
    {
        ItemProperties::parse(parts);

        readFlagList(parts, 94, flags, convertBagFlag);
        optionalPart(parts, 95, size);
    }

    void GenericProperties::parse(const QStringList &parts)
    {
        ItemProperties::parse(parts);
    }

    void PlayerCharacterProperties::parse(const QStringList &parts)
    {
        NonPlayerCharacterProperties::parse(parts);
    }

    inline QString convertNpcFlag(const QString &npcFlag)
    {
        static QHash<QString,QString> mapping;

        if (mapping.isEmpty()) {
            mapping["ONF_KOS"] = "KillOnSight";
            mapping["ONF_EXTRAPLANAR"] = "ExtraPlanar";
            mapping["ONF_BOSS_MONSTER"] = "BossMonster";
            mapping["ONF_NO_ATTACK"] = "NoAttack";
            mapping["ONF_NO_EQUIP"] = "NoEquip";
            mapping["ONF_WANDERS_IN_DARK"] = "WandersInDark";
            mapping["ONF_WANDERS"] = "Wanders";
            mapping["ONF_USE_ALERTPOINTS"] = "UseAlertPoints";
            mapping["ONF_LOGBOOK_IGNORES"] = "LogBookIgnores";
            mapping["ONF_ALOOF"] = "Aloof";
        }

        Q_ASSERT_X(mapping.contains(npcFlag), "convertNpcFlag", qPrintable(npcFlag));
        return mapping[npcFlag];
    }

    void NonPlayerCharacterProperties::parse(const QStringList &parts)
    {
        CritterProperties::parse(parts);

        readFlagList(parts, 152, flags, convertNpcFlag);
        optionalPart(parts, 153, aiData);

        if (isPartDefined(parts[154])) {
            QStringList factionStrings = parts[154].split(QRegExp("(\\s+|\\,)"), QString::SkipEmptyParts);
            foreach (QString faction, factionStrings) {
                bool ok;
                factions.append(faction.toUInt(&ok));
                Q_ASSERT_X(ok, "read faction", qPrintable(parts[154]));
            }
        }

        // 155 Retail price multiplier (unused)
        // 156 Reaction base (unused)
        if (isPartDefined(parts[157]))
            challengeRating = parts[157].trimmed();

        optionalPart(parts, 158, reflexSave);
        optionalPart(parts, 159, fortitudeSave);
        optionalPart(parts, 160, willpowerSave);
        optionalPart(parts, 161, acBonus);

        if (isPartDefined(parts[162]))
            hitDice = parts[162];

        if (isPartDefined(parts[163])) {
            type = parts[163];
            if (type.startsWith("mc_type_"))
                type = type.right(type.length() - 8);
        }

        if (isPartDefined(parts[164])) {
            subTypes = parts[164].split('\x00b');
            for (int i = 0; i < subTypes.size(); ++i) {
                if (subTypes[i].startsWith("mc_subtype_"))
                    subTypes[i] = subTypes[i].right(subTypes[i].length() - 11);
            }
        }

        if (isPartDefined(parts[165]))
            lootShareAmount = parts[165];

        optionalPart(parts, 311, additionalMeshId);
    }

    void TrapProperties::parse(const QStringList &parts)
    {
        ItemProperties::parse(parts);
    }

    inline QString convertItemFlag(const QString &objectType)
    {
        static QHash<QString,QString> mapping;

        if (mapping.isEmpty()) {
            mapping["OIF_DRAW_WHEN_PARENTED"] = "DrawWhenParented";
            mapping["OIF_EXPIRES_AFTER_USE"] = "ExpiresAfterUse";
            mapping["OIF_IDENTIFIED"] = "Identified";
            mapping["OIF_LIGHT_LARGE"] = "LightLarge";
            mapping["OIF_IS_MAGICAL"] = "IsMagical";
            mapping["OIF_NEEDS_SPELL"] = "NeedsSpell";
            mapping["OIF_NO_DISPLAY"] = "NoDisplay";
            mapping["OIF_NO_DROP"] = "NoDrop";
            mapping["OIF_NO_LOOT"] = "NoLoot";
            mapping["OIF_NO_NPC_PICKUP"] = "NoNpcPickup";
            mapping["OIF_USES_WAND_ANIM"] = "UsesWandAnim";
            mapping["OIF_NO_PICKPOCKET"] = "NotPickpocketable";
            mapping["OIF_STOLEN"] = "Stolen";
            mapping["OIF_FAMILIAR"] = "Familiar";
        }

        Q_ASSERT(mapping.contains(objectType));
        return mapping[objectType];
    }

    inline QString convertWearFlag(const QString &objectType)
    {
        static QHash<QString,QString> mapping;

        if (mapping.isEmpty()) {
            mapping["OIF_WEAR_AMMO"] = "ammo";
            mapping["OIF_WEAR_ARMOR"] = "armor";
            mapping["OIF_WEAR_BARDIC_ITEM"] = "instrument";
            mapping["OIF_WEAR_BOOTS"] = "boots";
            mapping["OIF_WEAR_2HAND_REQUIRED"] = "twoHanded"; // Look below, it will be removed and converted to bool
            mapping["OIF_WEAR_BRACERS"] = "bracers";
            mapping["OIF_WEAR_BUCKLER"] = "shield";
            mapping["OIF_WEAR_CLOAK"] = "cloak";
            mapping["OIF_WEAR_GLOVES"] = "gloves";
            mapping["OIF_WEAR_HELMET"] = "helmet";
            mapping["OIF_WEAR_LOCKPICKS"] = "lockpicks";
            mapping["OIF_WEAR_NECKLACE"] = "amulet";
            mapping["OIF_WEAR_RING"] = "ring";
            mapping["OIF_WEAR_RING_PRIMARY"] = "ring";
            mapping["OIF_WEAR_RING_SECONDARY"] = "ring";
            mapping["OIF_WEAR_ROBES"] = "robes";
            mapping["OIF_WEAR_WEAPON_PRIMARY"] = "primaryWeapon";
            mapping["OIF_WEAR_WEAPON_SECONDARY"] = "secondaryWeapon";
            mapping["OF_WEAR_WEAPON_SECONDARY"] = "primaryWeapon";
        }

        Q_ASSERT(mapping.contains(objectType));
        return mapping[objectType];
    }

    void ItemProperties::parse(const QStringList &parts)
    {
        EntityProperties::parse(parts);

        readFlagList(parts, 50, flags, convertItemFlag);
        optionalPart(parts, 51, weight);
        optionalPart(parts, 52, worth);
        optionalPart(parts, 53, inventoryIcon);
        optionalPart(parts, 55, unidentifiedDescriptionId);
        optionalPart(parts, 59, chargesLeft);
        readFlagSet(parts, 61, equipSlots, convertWearFlag);
        // These are attempts at cleaning up the strange meaning of flags in ToEE
        if (equipSlots.contains("twoHanded")) {
            twoHanded = true;
            equipSlots.removeAll("twoHanded");
        }
        if ((equipSlots.contains("primaryWeapon") ||
                equipSlots.contains("secondaryWeapon")) && equipSlots.contains("armor")) {
            equipSlots.clear();
            equipSlots << "shield";
        }
        if (equipSlots.contains("ring") && equipSlots.contains("amulet")) {
            equipSlots.clear();
            equipSlots << "amulet";
        }
        optionalPart(parts, 62, wearMeshId);
    }

    inline QString convertCritterFlag(const QString &critterFlag)
    {
        static QHash<QString,QString> mapping;

        if (mapping.isEmpty()) {
            mapping["OCF_NO_FLEE"] = "NoFlee";
            mapping["OCF_UNRESSURECTABLE"] = "Unressurectable";
            mapping["OCF_UNREVIVIFIABLE"] = "Unrevivifiable";
            mapping["OCF_MONSTER"] = "Monster";
            mapping["OCF_MUTE"] = "Mute";
            mapping["OCF_UNDEAD"] = "Undead";
            mapping["OCF_ANIMAL"] = "Animal";
            mapping["OCF_PLANT"] = "Plant";
            mapping["OCF_AIR"] = "Air";
            mapping["OCF_FIRE"] = "Fire";
            mapping["OCF_EARTH"] = "Earth";
            mapping["OCF_IS_CONCEALED"] = "IsConcealed";
            mapping["OCF_WATER"] = "Water";
        }

        Q_ASSERT_X(mapping.contains(critterFlag), "convertCritterFlag", qPrintable(critterFlag));
        return mapping[critterFlag];
    }

    void CritterProperties::parse(const QStringList &parts)
    {
        EntityProperties::parse(parts);

        readFlagList(parts, 99, flags, convertCritterFlag);
        bool ok;
        strength = parts[101].toInt(&ok);
        if (!ok) {
            qDebug("Strength missing for critter prototype %d. Defaulting to 10.", currentPrototypeId);
            strength = 10;
        }
        dexterity = parts[102].toInt(&ok);
        if (!ok) {
            qDebug("Dexterity missing for critter prototype %d. Defaulting to 10.", currentPrototypeId);
            dexterity = 10;
        }
        constitution = parts[103].toInt(&ok);
        if (!ok) {
            qDebug("Constitution missing for critter prototype %d. Defaulting to 10.", currentPrototypeId);
            constitution = 10;
        }
        intelligence = parts[104].toInt(&ok);
        if (!ok) {
            qDebug("Intelligence missing for critter prototype %d. Defaulting to 10.", currentPrototypeId);
            intelligence = 10;
        }
        wisdom = parts[105].toInt(&ok);
        if (!ok) {
            qDebug("Wisdom missing for critter prototype %d. Defaulting to 10.", currentPrototypeId);
            wisdom = 10;
        }
        charisma = parts[106].toInt(&ok);
        if (!ok) {
            qDebug("Charisma missing for critter prototype %d. Defaulting to 10.", currentPrototypeId);
            charisma = 10;
        }
        if (isPartDefined(parts[108])) {
            race = parts[108];
            if (race.startsWith("race_"))
                race = race.right(race.length() - 5);
        }
        if (isPartDefined(parts[109]))
            gender = parts[109];
        optionalPart(parts, 110, age);
        if (isPartDefined(parts[113]))
            alignment = parts[113];
        if (isPartDefined(parts[114]))
            deity = parts[114];
        if (isPartDefined(parts[115]))
            domains.append(parts[115]);
        if (isPartDefined(parts[116]))
            domains.append(parts[116]);
        if (isPartDefined(parts[117]))
            alignmentChoice = parts[117];
        optionalPart(parts, 123, portraitId);
        optionalPart(parts, 128, unknownDescription);
        optionalPart(parts, 130, reach);

        for (int i = 132; i < 132 + 4 * 4; i += 4) {
            if (isPartDefined(parts[i])) {
                NaturalAttack naturalAttack;
                naturalAttack.numberOfAttacks = parts[i].toUInt(&ok);
                Q_ASSERT(ok);
                naturalAttack.damageDice = parts[i+1];
                naturalAttack.type = parts[i+2];
                naturalAttack.attackBonus = parts[i+3].toInt(&ok);
                if (!ok)
                    naturalAttack.attackBonus = 0;
                naturalAttacks.append(naturalAttack);
            }
        }

        hairColor = parts[148];
        hairType = parts[149];

        // Read class levels (up to 5 classes)
        for (int i = 228; i < 228 + 5 * 2; i += 2) {
            if (!isPartDefined(parts[i]))
                continue;

            ClassLevel level;
            level.name = parts[i];
            level.count = parts[i+1].toUInt(&ok);
            classLevels.append(level);

            Q_ASSERT_X(ok && level.count >= 1, "CritterProperties::parse", qPrintable(parts[i+1]));
        }

        // Read skills (up to 10)
        for (int i = 238; i < 238 + 10 * 2; i += 2) {
            if (!isPartDefined(parts[i]))
                continue;

            SkillLevel level;
            level.name = parts[i];
            level.count = parts[i+1].toInt(&ok);
            skills.append(level);

            Q_ASSERT_X(ok, "CritterProperties::parse", qPrintable(parts[i+1]));
        }

        // Read feats (up to 10)
        for (int i = 258; i < 268; ++i) {
            if (!isPartDefined(parts[i]))
                continue;

            feats.append(parts[i]);
        }

        if (isPartDefined(parts[332]))
            levelUpScheme = parts[332];

        if (isPartDefined(parts[333]))
            strategy = parts[333].trimmed();
    }

    void EntityProperties::parse(const QStringList &parts)
    {
        ScriptProperties::parse(parts);

        // There are up to 20 additional properties per entry
        for (int i = 168; i < 168 + 20 * 3; i += 3) {
            if (!isPartDefined(parts[i]))
                continue;

            AdditionalProperty property;
            property.type = parts[i];
            if (isPartDefined(parts[i+1]))
                property.param1 = parts[i+1];
            if (isPartDefined(parts[i+2]))
                property.param1 = parts[i+2];

            properties.append(property);
        }

        // Up to 20 spells
        for (int i = 312; i < 332; ++i) {
            if (!isPartDefined(parts[i]))
                continue;

            // Try parsing the spell definition
            // It's: '........' \\w+ \\d+
            QRegExp spellPattern("\\s*'?(.*)'\\s+(\\w+)\\s+(\\d+)\\s*");
            bool exactMatch = spellPattern.exactMatch(parts[i]);
            Q_ASSERT_X(exactMatch, "EntityProperties::match", qPrintable(parts[i]));

            bool ok;
            KnownSpell knownSpell;
            knownSpell.name = spellPattern.cap(1);
            knownSpell.source = spellPattern.cap(2);
            knownSpell.level = spellPattern.cap(3).toUInt(&ok);
            Q_ASSERT_X(ok, "EntityProperties::match (level)", qPrintable(parts[i]));
            spells.append(knownSpell);
        }
    }

    static const int eventCount = 43;

    static const QString eventNames[eventCount] = {
        "OnExamine",
        "OnUse",
        "OnDestroy",
        "OnUnlock",
        "OnGet",
        "OnDrop",
        "OnThrow",
        "OnHit",
        "OnMiss",
        "OnDialog",
        "OnFirstHeartbeat",
        "OnCatchingThief",
        "OnDying",
        "OnEnterCombat",
        "OnExitCombat",
        "OnStartCombat",
        "OnEndCombat",
        "OnBuyObject",
        "OnResurrect",
        "OnHeartbeat",
        "OnLeaderKilling",
        "OnInsertItem",
        "OnWillKos",
        "OnTakingDamage",
        "OnWieldOn",
        "OnWieldOff",
        "OnCritterHits",
        "OnNewSector",
        "OnRemoveItem",
        "OnLeaderSleeping",
        "OnBust",
        "OnDialogOverride",
        "OnTransfer",
        "OnCaughtThief",
        "OnCriticalHit",
        "OnCriticalMiss",
        "OnJoin",
        "OnDisband",
        "OnNewMap",
        "OnTrap",
        "OnTrueSeeing",
        "OnSpellCast",
        "OnUnlockAttempt"
    };

    void ScriptProperties::parse(const QStringList &parts)
    {
        for (int i = 0; i < eventCount; ++i) {
            if (!isPartDefined(parts[268 + i]))
                continue;

            QString scriptPart = parts[268 + i];

            QRegExp scriptPattern("\\s*(\\d+)\\s+(\\d+)\\s+0\\s+0\\s+0\\s*");

            bool exactMatch = scriptPattern.exactMatch(scriptPart);
            Q_ASSERT_X(exactMatch, "ScriptProperties::parse", qPrintable(scriptPart));

            ScriptAttachment script;
            script.event = eventNames[i];
            bool ok;
            script.scriptId = scriptPattern.cap(1).toUInt(&ok);
            Q_ASSERT_X(ok, "ScriptProperties::parse", qPrintable(scriptPart));
            if (scriptPattern.cap(2) != "0") {
                script.parameter.setValue(scriptPattern.cap(2).toUInt(&ok));
                Q_ASSERT_X(ok, "ScriptProperties::parse", qPrintable(scriptPart));
            }

            scripts.append(script);
        }
    }

    const QMap<uint, Prototype*> &Prototypes::prototypes() const
    {
        return d_ptr->prototypes;
    }

}
