using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Rules
{
    public class Critter : BaseObject
    {

        public int BaseStrength { get; set; }

        public int BaseDexterity { get; set; }

        public int BaseConstitution { get; set; }

        public int BaseIntelligence { get; set; }

        public int BaseWisdom { get; set; }

        public int BaseCharisma { get; set; }

        public int Height { get; set; }

        public int Weight { get; set; }

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

        public bool Concealed { get; set; }

        public ushort MovementRange { get; set; }

        public byte RunFactor { get; set; }

        public bool KillsOnSight { get; set; }

        public uint ExperiencePoints { get; set; }

        public string Portrait { get; set; }

        public Gender Gender { get; set; }

        public Race Race { get; set; }

        public Critter()
        {
            MovementRange = 250;
            RunFactor = 3;
            DrawBehindWalls = true;
            KillsOnSight = false;
            BaseStrength = 10;
            BaseDexterity = 10;
            BaseConstitution = 10;
            BaseIntelligence = 10;
            BaseWisdom = 10;
            BaseCharisma = 10;
        }

    }
}
