using System;
using Rules.Prototypes;

namespace Rules
{

    public class Critter : BaseObject
    {
        private int? _baseCharisma;
        private int? _baseConstitution;
        private int? _baseDexterity;
        private int? _baseIntelligence;
        private int? _baseStrength;
        private int? _baseWisdom;
        private bool? _concealed;
        private uint? _experiencePoints;
        private Gender? _gender;
        private int? _height;
        private bool? _killsOnSight;
        private string _model;
        private ushort? _movementRange;
        private Portrait _portrait;
        private Race _race;
        private byte? _runFactor;
        private int? _weight;

        public override BaseObjectPrototype DefaultPrototype
        {
            get { return CritterPrototype.Default; }
        }

        public override BaseObjectPrototype Prototype
        {
            set
            {
                if (!(value is CritterPrototype))
                    throw new ArgumentException("Can only set a CritterPrototype on Critters.");
                base.Prototype = value;
            }
        }

        public CritterPrototype CritterPrototype
        {
            get { return (CritterPrototype) Prototype; }
        }

        public int BaseStrength
        {
            get { return _baseStrength ?? CritterPrototype.BaseStrength; }
            set { _baseStrength = (value == CritterPrototype.BaseStrength) ? (int?) null : value; }
        }

        public int BaseDexterity
        {
            get { return _baseDexterity ?? CritterPrototype.BaseDexterity; }
            set { _baseDexterity = (value == CritterPrototype.BaseDexterity) ? (int?) null : value; }
        }

        public int BaseConstitution
        {
            get { return _baseConstitution ?? CritterPrototype.BaseConstitution; }
            set { _baseConstitution = (value == CritterPrototype.BaseConstitution) ? (int?) null : value; }
        }

        public int BaseIntelligence
        {
            get { return _baseIntelligence ?? CritterPrototype.BaseIntelligence; }
            set { _baseIntelligence = (value == CritterPrototype.BaseIntelligence) ? (int?) null : value; }
        }

        public int BaseWisdom
        {
            get { return _baseWisdom ?? CritterPrototype.BaseWisdom; }
            set { _baseWisdom = (value == CritterPrototype.BaseWisdom) ? (int?) null : value; }
        }

        public int BaseCharisma
        {
            get { return _baseCharisma ?? CritterPrototype.BaseCharisma; }
            set { _baseCharisma = (value == CritterPrototype.BaseCharisma) ? (int?) null : value; }
        }

        public int Height
        {
            get { return _height ?? CritterPrototype.Height; }
            set { _height = (value == CritterPrototype.Height) ? (int?) null : value; }
        }

        public int Weight
        {
            get { return _weight ?? CritterPrototype.Weight; }
            set { _weight = (value == CritterPrototype.Weight) ? (int?) null : value; }
        }

        public int Strength
        {
            get { return BaseStrength; }
        }

        public int Dexterity
        {
            get { return BaseDexterity; }
        }

        public int Constitution
        {
            get { return BaseConstitution; }
        }

        public int Intelligence
        {
            get { return BaseIntelligence; }
        }

        public int Wisdom
        {
            get { return BaseWisdom; }
        }

        public int Charisma
        {
            get { return BaseCharisma; }
        }

        public bool Concealed
        {
            get { return _concealed ?? CritterPrototype.Concealed; }
            set { _concealed = (value == CritterPrototype.Concealed) ? (bool?) null : value; }
        }

        public ushort MovementRange
        {
            get { return _movementRange ?? CritterPrototype.MovementRange; }
            set { _movementRange = (value == CritterPrototype.MovementRange) ? (ushort?) null : value; }
        }

        public byte RunFactor
        {
            get { return _runFactor ?? CritterPrototype.RunFactor; }
            set { _runFactor = (value == CritterPrototype.RunFactor) ? (byte?) null : value; }
        }

        public bool KillsOnSight
        {
            get { return _killsOnSight ?? CritterPrototype.KillsOnSight; }
            set { _killsOnSight = (value == CritterPrototype.KillsOnSight) ? (bool?) null : value; }
        }

        public uint ExperiencePoints
        {
            get { return _experiencePoints ?? CritterPrototype.ExperiencePoints; }
            set { _experiencePoints = (value == CritterPrototype.ExperiencePoints) ? (uint?) null : value; }
        }

        public Portrait Portrait
        {
            get { return _portrait ?? CritterPrototype.Portrait; }
            set { _portrait = (value == CritterPrototype.Portrait) ? null : value; }
        }

        public Gender Gender
        {
            get { return _gender ?? CritterPrototype.Gender; }
            set
            {
                if (value == Gender) return;
                _gender = (value == CritterPrototype.Gender) ? (Gender?) null : value;
                InvokeOnPropertyChanged("Gender");
            }
        }

        public Race Race
        {
            get { return _race ?? CritterPrototype.Race; }
            set { _race = (value == CritterPrototype.Race) ? null : value; }
        }

        public string Model
        {
            get { return _model ?? CritterPrototype.Model; }
            set { _model = (value == CritterPrototype.Model) ? null : value; }
        }
    }
}