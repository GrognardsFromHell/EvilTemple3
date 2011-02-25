using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text.RegularExpressions;

namespace TroikaFormats
{
    public enum ObjectType
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
    }

    internal static class Part
    {
        public static void Optional(IList<string> parts, int offset, ref int? field)
        {
            if (!string.IsNullOrWhiteSpace(parts[offset]))
                field = int.Parse(parts[offset]);
        }

        public static void Optional(IList<string> parts, int offset, ref uint? field)
        {
            if (!string.IsNullOrWhiteSpace(parts[offset]))
                field = uint.Parse(parts[offset]);
        }

        public static void Optional(IList<string> parts, 
                                    int offset, 
                                    ref float? field)
        {
            if (!string.IsNullOrWhiteSpace(parts[offset]))
                field = float.Parse(parts[offset]);
        }

        public static void Flags(IList<string> parts,
                                 int offset,
                                 ISet<string> result,
                                 Func<string, string> translate)
        {
            var flagList = parts[offset].Trim();

            // Co8 Bugfix
            flagList = flagList.Replace("OIF ", "OIF_");

            var list = flagList.Split(new[] {'\x0B', ' ', ',', '\x7F'}, StringSplitOptions.RemoveEmptyEntries);

            foreach (var item in list)
            {
                var translated = translate(item);
                if (!string.IsNullOrWhiteSpace(translated))
                    result.Add(translated);
            }
        }
    }

    public abstract class AdditionalProperties
    {
        public abstract void parse(IList<string> parts);
    }

    public struct ScriptAttachment
    {
        public string EventId; // The event to which this script is attached
        public int? Parameter; // This additional parameter is only used by traps
        public uint ScriptId; // The id of the attached script
    }

    public class ScriptProperties : AdditionalProperties
    {
        private static readonly string[] EventNames = new[]
                                                          {
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

        public IList<ScriptAttachment> Scripts;

        public override void parse(IList<string> parts)
        {
            Scripts = new List<ScriptAttachment>();

            for (var i = 0; i < EventNames.Length; ++i)
            {
                var scriptPart = parts[268 + i];

                if (string.IsNullOrWhiteSpace(scriptPart))
                    continue;

                var pattern = new Regex(@"^\s*(\d+)\s+(\d+)\s+0\s+0\s+0\s*$");

                var match = pattern.Match(scriptPart);

                Trace.Assert(match.Success, "ScriptProperties::parse", scriptPart);

                var script = new ScriptAttachment
                                 {
                                     EventId = EventNames[i],
                                     ScriptId = uint.Parse(match.Groups[1].Value)
                                 };

                if (match.Groups[2].Value != "0")
                    script.Parameter = int.Parse(match.Groups[2].Value);

                Scripts.Add(script);
            }
        }
    }

    public class PortalProperties : ScriptProperties
    {
        /*
         None of these properties seem to be used.
         */
        public ISet<string> flags; // 37
        public int? keyId; // 39
        public int? lockDc; // 38
        public int? notifyNpc; // 40
    }

    public class ContainerProperties : ScriptProperties
    {
        public int? InventorySource; // 44, invensource.mes
        public int? KeyId; // 43
        public int? LockDc; // 42
        public bool Locked; // 41
        public int? NotifyNpc; // 45, Seems unused

        public override void parse(IList<string> parts)
        {
            base.parse(parts);

            Locked = parts[41].Trim() == "OCOF_LOCKED";

            Part.Optional(parts, 42, ref LockDc);
            Part.Optional(parts, 43, ref KeyId);
            Part.Optional(parts, 44, ref InventorySource);
        }
    }

    public class SceneryProperties : ScriptProperties
    {
        // 46
        private static IDictionary<string, string> _mapping;
        public ISet<string> SceneryFlags;
        private int? respawnDelay; // 47, Seems unused

        public override void parse(IList<string> parts)
        {
            base.parse(parts);

            SceneryFlags = new HashSet<string>();

            Part.Flags(parts, 46, SceneryFlags, ConvertFlag);
        }

        private static string ConvertFlag(string flag)
        {
            if (_mapping == null)
            {
                _mapping = new Dictionary<string, string>
                               {
                                   {"OSCF_NO_AUTO_ANIMATE", "NoAutoAnimate"},
                                   {"OSCF_SOUND_EXTRA_LARGE", "SoundExtraLarge"},
                               };
            }

            return _mapping[flag];
        }
    }

    public class ProjectileProperties : ScriptProperties
    {
        private IList<string> dmgFlags; // 49, Unused
        private IList<string> flags; // 48, Unused
    }

    public struct AdditionalProperty
    {
        public string Param1;
        public string Param2;
        public string Type;
    }

    /**
      Represents a spell known by a critter or imbued in an item or weapon.
      */

    public class KnownSpell
    {
        public uint Level; // Spell level
        public string Name;
        public string Source; // Class, or "domain_special" for instance
    }

    public class EntityProperties : ScriptProperties
    {
        public IList<AdditionalProperty> Properties;
        public IList<KnownSpell> Spells;

        public override void parse(IList<string> parts)
        {
            base.parse(parts);

            Properties = new List<AdditionalProperty>();

            // There are up to 20 additional properties per entry
            for (var i = 168; i < 168 + 20*3; i += 3)
            {
                if (string.IsNullOrWhiteSpace(parts[i]))
                    continue;

                var property = new AdditionalProperty {Type = parts[i]};
                if (!string.IsNullOrWhiteSpace(parts[i + 1]))
                    property.Param1 = parts[i + 1];
                if (!string.IsNullOrWhiteSpace(parts[i + 2]))
                    property.Param1 = parts[i + 2];

                Properties.Add(property);
            }

            Spells = new List<KnownSpell>();

            // Up to 20 spells
            for (var i = 312; i < 332; ++i)
            {
                if (string.IsNullOrWhiteSpace(parts[i]))
                    continue;

                // Try parsing the spell definition
                // It's: '........' \w+ \d+
                var spellPattern = new Regex(@"\s*'?(.*)'\s+(\w+)\s+(\d+)\s*");
                var match = spellPattern.Match(parts[i]);
                Trace.Assert(match.Success, parts[i]);

                var knownSpell = new KnownSpell
                                     {
                                         Name = match.Groups[1].Value,
                                         Source = match.Groups[2].Value,
                                         Level = uint.Parse(match.Groups[3].Value)
                                     };
                Spells.Add(knownSpell);
            }
        }
    }

    public class ItemProperties : EntityProperties
    {
        private static readonly IDictionary<string, string> ItemFlagMapping;

        private static readonly IDictionary<string, string> WearFlagMapping;

        private int? AiAction; // 60, Unused
        public int? ChargesLeft; // 59
        public ISet<string> EquipSlots; // 61
        private int? InventoryGroundMesh; // 54, Unused
        public int? InventoryIcon; // 53
        public ISet<string> ItemFlags; // 50
        private int? LongDescriptionId; // 56, unused
        private int? SpellIndex; // 57, unused
        private int? SpellIndexFlags; // 58, unused
        public bool TwoHanded; // 62
        public int? UnidentifiedDescriptionId; // 55
        public int? WearMeshId; // 62
        public int? Weight; // 51
        public int? Worth; // 52, in copper

        static ItemProperties()
        {
            ItemFlagMapping = new Dictionary<string, string>
                                  {
                                      {"OIF_DRAW_WHEN_PARENTED", "DrawWhenParented"},
                                      {"OIF_EXPIRES_AFTER_USE", "ExpiresAfterUse"},
                                      {"OIF_IDENTIFIED", "Identified"},
                                      {"OIF_LIGHT_LARGE", "LightLarge"},
                                      {"OIF_IS_MAGICAL", "IsMagical"},
                                      {"OIF_NEEDS_SPELL", "NeedsSpell"},
                                      {"OIF_NO_DISPLAY", "NoDisplay"},
                                      {"OIF_NO_DROP", "NoDrop"},
                                      {"OIF_NO_LOOT", "NoLoot"},
                                      {"OIF_NO_NPC_PICKUP", "NoNpcPickup"},
                                      {"OIF_USES_WAND_ANIM", "UsesWandAnim"},
                                      {"OIF_NO_PICKPOCKET", "NotPickpocketable"},
                                      {"OIF_STOLEN", "Stolen"},
                                      {"OIF_FAMILIAR", "Familiar"},
                                  };

            WearFlagMapping = new Dictionary<string, string>
                                  {
                                      {"OIF_WEAR_AMMO", "ammo"},
                                      {"OIF_WEAR_ARMOR", "armor"},
                                      {"OIF_WEAR_BARDIC_ITEM", "instrument"},
                                      {"OIF_WEAR_BOOTS", "boots"},
                                      {"OIF_WEAR_2HAND_REQUIRED", "twoHanded"},
                                      // Look below, it will be removed and converted to bool
                                      {"OIF_WEAR_BRACERS", "bracers"},
                                      {"OIF_WEAR_BUCKLER", "shield"},
                                      {"OIF_WEAR_CLOAK", "cloak"},
                                      {"OIF_WEAR_GLOVES", "gloves"},
                                      {"OIF_WEAR_HELMET", "helmet"},
                                      {"OIF_WEAR_LOCKPICKS", "lockpicks"},
                                      {"OIF_WEAR_NECKLACE", "amulet"},
                                      {"OIF_WEAR_RING", "ring"},
                                      {"OIF_WEAR_RING_PRIMARY", "ring"},
                                      {"OIF_WEAR_RING_SECONDARY", "ring"},
                                      {"OIF_WEAR_ROBES", "robes"},
                                      {"OIF_WEAR_WEAPON_PRIMARY", "primaryWeapon"},
                                      {"OIF_WEAR_WEAPON_SECONDARY", "secondaryWeapon"},
                                      {"OF_WEAR_WEAPON_SECONDARY", "primaryWeapon"},
                                  };
        }

        public override void parse(IList<string> parts)
        {
            base.parse(parts);

            ItemFlags = new HashSet<string>();
            Part.Flags(parts, 50, ItemFlags, f => ItemFlagMapping[f]);
            Part.Optional(parts, 51, ref Weight);
            Part.Optional(parts, 52, ref Worth);
            Part.Optional(parts, 53, ref InventoryIcon);
            Part.Optional(parts, 55, ref UnidentifiedDescriptionId);
            Part.Optional(parts, 59, ref ChargesLeft);

            EquipSlots = new HashSet<string>();
            Part.Flags(parts, 61, EquipSlots, f => WearFlagMapping[f]);
            // These are attempts at cleaning up the strange meaning of flags in ToEE
            TwoHanded = EquipSlots.Remove("twoHanded");

            if ((EquipSlots.Contains("primaryWeapon") ||
                 EquipSlots.Contains("secondaryWeapon")) && EquipSlots.Contains("armor"))
            {
                EquipSlots.Clear();
                EquipSlots.Add("shield");
            }
            if (EquipSlots.Contains("ring") && EquipSlots.Contains("amulet"))
            {
                EquipSlots.Clear();
                EquipSlots.Add("amulet");
            }
            Part.Optional(parts, 62, ref WearMeshId);
        }
    }

    public class WeaponProperties : ItemProperties
    {
        private int? ammoConsumption; // 66, Unused
        private string ammoType; // 65
        private int? criticalHitMultiplier; // 68
        private string damageDice; // 70
        private string damageType; // 69
        private IList<string> flags; // 63
        private int? missileAnimationId; // 67
        private int? range; // 64
        // 71, Animation type (unused)
        private int? threatRange; // 73
        private string weaponClass; // 72, used for feats

        public override void parse(IList<string> parts)
        {
            base.parse(parts);
            // TODO: Implement
        }
    }

    public class AmmoProperties : ItemProperties
    {
        private int? quantity; // 75
        private string type; // 76

        public override void parse(IList<string> parts)
        {
            base.parse(parts);
            // TODO: Implement
        }
    }

    public class ArmorProperties : ItemProperties
    {
        private int? arcaneSpellFailure; // 80
        private string armorType; // 82
        private string helmetType; // 83
        private int? maxDexterityBonus; // 79
        private int? skillCheckPenalty; // 81

        public override void parse(IList<string> parts)
        {
            base.parse(parts);
            // TODO: Implement
        }
    }

    public class MoneyProperties : ItemProperties
    {
        private uint quantity;
        private string type;

        public override void parse(IList<string> parts)
        {
            base.parse(parts);
            // TODO: Implement
        }
    }

    public class FoodProperties : ItemProperties
    {
        public override void parse(IList<string> parts)
        {
            base.parse(parts);
            // TODO: Implement
        }
    }

    public class ScrollProperties : ItemProperties
    {
        public override void parse(IList<string> parts)
        {
            base.parse(parts);
            // TODO: Implement
        }
    }

    public class KeyProperties : ItemProperties
    {
        private uint keyId;

        public override void parse(IList<string> parts)
        {
            base.parse(parts);
            // TODO: Implement
        }
    }

    public class WrittenProperties : ItemProperties
    {
        private int? startLine;
        private uint subtype;

        public override void parse(IList<string> parts)
        {
            base.parse(parts);
            // TODO: Implement
        }

        // EndLine is unused
    }

    public class BagProperties : ItemProperties
    {
        private IList<string> flags;
        private int? size;

        public override void parse(IList<string> parts)
        {
            base.parse(parts);
            // TODO: Implement
        }
    }

    public class GenericProperties : ItemProperties
    {
        public override void parse(IList<string> parts)
        {
            base.parse(parts);
            // TODO: Implement
        }
    }

    public class NaturalAttack
    {
        public int attackBonus;
        public string damageDice;
        public uint numberOfAttacks;
        public string type;
    }

    public class ClassLevel
    {
        public uint count; // min: 1
        public string name;
    }

    public class SkillLevel
    {
        public int count;
        public string name;
    }

    public class CritterProperties : EntityProperties
    {
        private static IDictionary<string, string> _mapping;
        public int? Age;
        // Height: unused
        // Weight: unused
        public string Alignment;
        public string AlignmentChoice;
        public int Charisma;
        public IList<ClassLevel> ClassLevels;
        public int Constitution;
        public string Deity;
        public int Dexterity;
        public ISet<string> Domains;
        public ISet<string> Feats;
        public ISet<string> Flags; // 99
        public string Gender;
        public string HairColor;
        public string HairType;
        public int Intelligence;

        public string LevelUpScheme; // Defines auto-leveling stuff (which feats to take, etc.)
        public IList<NaturalAttack> NaturalAttacks; // at most 4 different ones
        public int? PortraitId;
        public string Race;
        public int? Reach;
        public IList<SkillLevel> Skills;
        public string Strategy; // Which AI is used in auto fighting situations
        public int Strength;
        public int? UnknownDescription; // If not yet talked to the npc
        public int Wisdom;

        public override void parse(IList<string> parts)
        {
            base.parse(parts);

            Flags = new HashSet<string>();
            Part.Flags(parts, 99, Flags, ConvertCritterFlag);

            if (!int.TryParse(parts[101], out Strength))
            {
                Trace.TraceInformation("Strength missing. Defaulting to 10.");
                Strength = 10;
            }
            if (!int.TryParse(parts[102], out Dexterity))
            {
                Trace.TraceInformation("Dexterity. Defaulting to 10.");
                Dexterity = 10;
            }
            if (!int.TryParse(parts[103], out Constitution))
            {
                Trace.TraceInformation("Constitution. Defaulting to 10.");
                Constitution = 10;
            }
            if (!int.TryParse(parts[104], out Intelligence))
            {
                Trace.TraceInformation("Intelligence. Defaulting to 10.");
                Intelligence = 10;
            }
            if (!int.TryParse(parts[105], out Wisdom))
            {
                Trace.TraceInformation("Wisdom. Defaulting to 10.");
                Wisdom = 10;
            }
            if (!int.TryParse(parts[106], out Charisma))
            {
                Trace.TraceInformation("Charisma. Defaulting to 10.");
                Charisma = 10;
            }
            if (!string.IsNullOrWhiteSpace(parts[108]))
            {
                Race = parts[108];
                if (Race.StartsWith("race_"))
                    Race = Race.Substring(5);
            }
            if (!string.IsNullOrWhiteSpace(parts[109]))
                Gender = parts[109];
            Part.Optional(parts, 110, ref Age);
            if (!string.IsNullOrWhiteSpace(parts[113]))
                Alignment = parts[113];
            if (!string.IsNullOrWhiteSpace(parts[114]))
                Deity = parts[114];
            Domains = new HashSet<string>();
            if (!string.IsNullOrWhiteSpace(parts[115]))
                Domains.Add(parts[115]);
            if (!string.IsNullOrWhiteSpace(parts[116]))
                Domains.Add(parts[116]);
            if (!string.IsNullOrWhiteSpace(parts[117]))
                AlignmentChoice = parts[117];
            Part.Optional(parts, 123, ref PortraitId);
            Part.Optional(parts, 128, ref UnknownDescription);
            Part.Optional(parts, 130, ref Reach);

            NaturalAttacks = new List<NaturalAttack>();
            for (var i = 132; i < 132 + 4*4; i += 4)
            {
                if (string.IsNullOrWhiteSpace(parts[i])) continue;
                var naturalAttack = new NaturalAttack
                                        {
                                            numberOfAttacks = uint.Parse(parts[i]),
                                            damageDice = parts[i + 1],
                                            type = parts[i + 2]
                                        };
                int.TryParse(parts[i + 3], out naturalAttack.attackBonus);
                NaturalAttacks.Add(naturalAttack);
            }

            HairColor = parts[148];
            HairType = parts[149];

            // Read class levels (up to 5 classes)
            ClassLevels = new List<ClassLevel>();
            for (var i = 228; i < 228 + 5*2; i += 2)
            {
                if (string.IsNullOrWhiteSpace(parts[i]))
                    continue;

                var level = new ClassLevel {name = parts[i], count = uint.Parse(parts[i + 1])};
                ClassLevels.Add(level);

                Trace.Assert(level.count >= 1, parts[i + 1]);
            }

            // Read skills (up to 10)
            Skills = new List<SkillLevel>();
            for (var i = 238; i < 238 + 10*2; i += 2)
            {
                if (string.IsNullOrWhiteSpace(parts[i]))
                    continue;

                var level = new SkillLevel
                                {
                                    name = parts[i],
                                    count = int.Parse(parts[i + 1])
                                };
                Skills.Add(level);
            }

            // Read feats (up to 10)
            Feats = new HashSet<string>();
            for (var i = 258; i < 268; ++i)
            {
                if (string.IsNullOrWhiteSpace(parts[i]))
                    continue;

                Feats.Add(parts[i]);
            }

            if (!string.IsNullOrWhiteSpace(parts[332]))
                LevelUpScheme = parts[332];

            if (!string.IsNullOrWhiteSpace(parts[333]))
                Strategy = parts[333].Trim();
        }

        private static string ConvertCritterFlag(string arg)
        {
            if (_mapping == null)
            {
                _mapping = new Dictionary<string, string>
                               {
                                   {"OCF_NO_FLEE", "NoFlee"},
                                   {"OCF_UNRESSURECTABLE", "Unressurectable"},
                                   {"OCF_UNREVIVIFIABLE", "Unrevivifiable"},
                                   {"OCF_MONSTER", "Monster"},
                                   {"OCF_MUTE", "Mute"},
                                   {"OCF_UNDEAD", "Undead"},
                                   {"OCF_ANIMAL", "Animal"},
                                   {"OCF_PLANT", "Plant"},
                                   {"OCF_AIR", "Air"},
                                   {"OCF_FIRE", "Fire"},
                                   {"OCF_EARTH", "Earth"},
                                   {"OCF_IS_CONCEALED", "IsConcealed"},
                                   {"OCF_WATER", "Water"}
                               };
            }

            return _mapping[arg];
        }
    }

    public class NonPlayerCharacterProperties : CritterProperties
    {
        private int? acBonus;
        private int? additionalMeshId; // An addmesh that should always be present (used for bugbear equipment)
        private int? aiData;
        // Retail price multiplier (unused)
        // Reaction base (unused)
        private string challengeRating; // Sometimes given as 1/3, etc. Only used as table-lookup anyhow
        private IList<uint> factions;
        private IList<string> flags; // 152
        private int? fortitudeSave;
        private string hitDice;
        private string lootShareAmount;
        private int? reflexSave;
        private IList<string> subTypes;
        private string type;
        private int? willpowerSave;

        public override void parse(IList<string> parts)
        {
            base.parse(parts);
            // TODO: Implement
        }
    }

    public class PlayerCharacterProperties : NonPlayerCharacterProperties
    {
        public override void parse(IList<string> parts)
        {
            base.parse(parts);
            // TODO: Implement
        }
    }

    public class TrapProperties : ItemProperties
    {
        public override void parse(IList<string> parts)
        {
            base.parse(parts);
            // TODO: Implement
        }
    }

    public class Prototype
    {
        private static IDictionary<string, string> objectFlagMapping;
        private static IDictionary<string, ObjectType> objectTypeMapping;
        private static IDictionary<string, string> objectMaterialMapping;
        private static IDictionary<string, string> objectSizeMapping;

        public readonly uint Id;
        public readonly ISet<string> ObjectFlags;
        public AdditionalProperties AdditionalProperties;
        public int? CategoryId;
        public uint DescriptionId;
        public bool? Disabled;
        public bool? DontDraw;
        public int? HitPoints;
        public bool? Interactive;
        public int? InternalDescriptionId;
        public ushort ModelId;
        public string ObjectMaterial; // May be null
        public string ObjectSize; // May be null
        public float? Radius;
        public float? RenderHeight;
        public float? Rotation;
        public float? RunSpeedFactor;
        public float? Scale;
        public int? SoundId;
        public ObjectType Type; // Type is mandatory
        public bool? Unlit;
        public float? WalkSpeedFactor;

        public Prototype(uint id)
        {
            Id = id;
            ObjectFlags = new HashSet<string>();
        }

        public void Parse(IList<string> parts)
        {
            Trace.Assert(!string.IsNullOrWhiteSpace(parts[1]));
            Type = ConvertObjectType(parts[1]);

            if (!string.IsNullOrWhiteSpace(parts[6]))
            {
                var scaleText = parts[6];
                scaleText = scaleText.Replace(".", ""); // Sometimes the scale is incorrectly defined as .25,
                // instead of 25 (at least i hope that)
                Scale = float.Parse(scaleText);
            }

            Part.Flags(parts, 20, ObjectFlags, translateObjectFlag);

            if (ObjectFlags.Remove("DontDraw"))
                DontDraw = true;
            if (ObjectFlags.Remove("Off"))
                Disabled = true;
            if (ObjectFlags.Remove("ClickThrough"))
                Interactive = false;
            if (ObjectFlags.Remove("DontLight"))
                Unlit = false;

            // 21: Spell Flags -> Unused
            Part.Optional(parts, 22, ref InternalDescriptionId);

            DescriptionId = uint.Parse(parts[23]);

            if (!string.IsNullOrWhiteSpace(parts[24]))
                ObjectSize = convertObjectSize(parts[24]);

            Part.Optional(parts, 25, ref HitPoints);

            if (!string.IsNullOrWhiteSpace(parts[27]))
                ObjectMaterial = convertObjectMaterial(parts[27]);

            Part.Optional(parts, 29, ref SoundId);

            Part.Optional(parts, 30, ref CategoryId);
            if (CategoryId.HasValue)
                CategoryId = CategoryId + (int) Type*1000;

            Part.Optional(parts, 31, ref Rotation);

            if (Rotation.HasValue)
                Rotation = Util.rad2deg(Constants.LegacyBaseRotation + Rotation.Value);

            Part.Optional(parts, 32, ref WalkSpeedFactor);

            Part.Optional(parts, 33, ref RunSpeedFactor);

            Trace.Assert(!string.IsNullOrWhiteSpace(parts[34])); // Model id is required.
            ModelId = ushort.Parse(parts[34]);

            Part.Optional(parts, 35, ref Radius);

            Part.Optional(parts, 36, ref RenderHeight);

            switch (Type)
            {
                case ObjectType.Portal:
                    AdditionalProperties = new PortalProperties();
                    break;
                case ObjectType.Container:
                    AdditionalProperties = new ContainerProperties();
                    break;
                case ObjectType.Scenery:
                    AdditionalProperties = new SceneryProperties();
                    break;
                case ObjectType.Projectile:
                    AdditionalProperties = new ProjectileProperties();
                    break;
                case ObjectType.Weapon:
                    AdditionalProperties = new WeaponProperties();
                    break;
                case ObjectType.Ammo:
                    AdditionalProperties = new AmmoProperties();
                    break;
                case ObjectType.Armor:
                    AdditionalProperties = new ArmorProperties();
                    break;
                case ObjectType.Money:
                    AdditionalProperties = new MoneyProperties();
                    break;
                case ObjectType.Food:
                    AdditionalProperties = new FoodProperties();
                    break;
                case ObjectType.Scroll:
                    AdditionalProperties = new ScrollProperties();
                    break;
                case ObjectType.Key:
                    AdditionalProperties = new KeyProperties();
                    break;
                case ObjectType.Written:
                    AdditionalProperties = new WrittenProperties();
                    break;
                case ObjectType.Bag:
                    AdditionalProperties = new BagProperties();
                    break;
                case ObjectType.Generic:
                    AdditionalProperties = new GenericProperties();
                    break;
                case ObjectType.PlayerCharacter:
                    AdditionalProperties = new PlayerCharacterProperties();
                    break;
                case ObjectType.NonPlayerCharacter:
                    AdditionalProperties = new NonPlayerCharacterProperties();
                    break;
                case ObjectType.Trap:
                    AdditionalProperties = new TrapProperties();
                    break;
                default:
                    throw new ArgumentException("Invalid object type encountered: " + Type);
            }

            AdditionalProperties.parse(parts);
        }

        private static string convertObjectMaterial(string s)
        {
            if (objectMaterialMapping == null)
            {
                objectMaterialMapping = new Dictionary<string, string>
                                            {
                                                {"mat_powder", "Powder"},
                                                {"mat_fire", "Fire"},
                                                {"mat_force", "Force"},
                                                {"mat_gas", "Gas"},
                                                {"mat_paper", "Paper"},
                                                {"mat_liquid", "Liquid"},
                                                {"mat_cloth", "Cloth"},
                                                {"mat_glass", "Glass"},
                                                {"mat_metal", "Metal"},
                                                {"mat_flesh", "Flesh"},
                                                {"mat_plant", "Plant"},
                                                {"mat_wood", "Wood"},
                                                {"mat_brick", "Brick"},
                                                {"mat_stone", "Stone"},
                                            };
            }
            return objectMaterialMapping[s];
        }


        private static string convertObjectSize(string s)
        {
            if (objectSizeMapping == null)
            {
                objectSizeMapping = new Dictionary<string, string>
                                        {
                                            {"size_none", "None"},
                                            {"size_fine", "Fine"},
                                            {"size_diminut", "Diminutive"},
                                            {"size_diminutive", "Diminutive"},
                                            {"size_tiny", "Tiny"},
                                            {"size_small", "Small"},
                                            {"size_medium", "Medium"},
                                            {"size_large", "Large"},
                                            {"size_huge", "Huge"},
                                            {"size_gargantuan", "Gargantuan"},
                                            {"size_colossal", "Colossal"},
                                        };
            }
            return objectSizeMapping[s];
        }

        private static string translateObjectFlag(string objectFlag)
        {
            if (objectFlagMapping == null)
            {
                objectFlagMapping = new Dictionary<string, string>();
                objectFlagMapping["OF_CLICK_THROUGH"] = "ClickThrough";
                objectFlagMapping["OF_DONTDRAW"] = "DontDraw";
                objectFlagMapping["OF_DONTLIGHT"] = "DontLight";
                objectFlagMapping["OF_INVISIBLE"] = "Invisible";
                objectFlagMapping["OF_INVULNERABLE"] = "Invulnerable";
                objectFlagMapping["OF_NO_BLOCK"] = "NoBlock";
                objectFlagMapping["OF_NOHEIGHT"] = "NoHeight";
                objectFlagMapping["OF_OFF"] = "Off";
                objectFlagMapping["OF_RANDOM_SIZE"] = "RandomSize";
                objectFlagMapping["OF_SEE_THROUGH"] = "SeeThrough";
                objectFlagMapping["OF_SHOOT_THROUGH"] = "ShootThrough";
                objectFlagMapping["OF_WADING"] = "Wading";
                objectFlagMapping["OF_WATER_WALKING"] = "WaterWalking";
                objectFlagMapping["OF_HEIGHT_SET"] = "";
                objectFlagMapping["OF_RADIUS_SET"] = "";
                objectFlagMapping["OF_PROVIDES_COVER"] = "ProvidesCover";
            }

            Trace.Assert(objectFlagMapping.ContainsKey(objectFlag), "convertObjectFlag", objectFlag);
            return objectFlagMapping[objectFlag];
        }

        private static ObjectType ConvertObjectType(string s)
        {
            if (objectTypeMapping == null)
            {
                objectTypeMapping = new Dictionary<string, ObjectType>
                                        {
                                            {"obj_t_portal", ObjectType.Portal},
                                            {"obj_t_container", ObjectType.Container},
                                            {"obj_t_scenery", ObjectType.Scenery},
                                            {"obj_t_projectile", ObjectType.Projectile},
                                            {"obj_t_weapon", ObjectType.Weapon},
                                            {"obj_t_ammo", ObjectType.Ammo},
                                            {"obj_t_armor", ObjectType.Armor},
                                            {"obj_t_money", ObjectType.Money},
                                            {"obj_t_food", ObjectType.Food},
                                            {"obj_t_scroll", ObjectType.Scroll},
                                            {"obj_t_key", ObjectType.Key},
                                            {"obj_t_written", ObjectType.Written},
                                            {"obj_t_generic", ObjectType.Generic},
                                            {"obj_t_pc", ObjectType.PlayerCharacter},
                                            {"obj_t_npc", ObjectType.NonPlayerCharacter},
                                            {"obj_t_trap", ObjectType.Trap},
                                            {"obj_t_bag", ObjectType.Bag},
                                        };
            }
            return objectTypeMapping[s];
        }

        public override string ToString()
        {
            return string.Format("Id: {0}, DescriptionId: {1}", Id, DescriptionId);
        }
    }
}