namespace Rules.Prototypes
{
    public class CritterPrototype : BaseObjectPrototype
    {
        public new static readonly CritterPrototype Default = new CritterPrototype();

        public CritterPrototype()
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
            Model = "meshes/PCs/PC_Human_Male/PC_Human_Male.model";
        }

        public int BaseCharisma { get; set; }
        public int BaseConstitution { get; set; }
        public int BaseDexterity { get; set; }
        public int BaseIntelligence { get; set; }
        public int BaseStrength { get; set; }
        public int BaseWisdom { get; set; }
        public bool Concealed { get; set; }
        public uint ExperiencePoints { get; set; }
        public Gender Gender { get; set; }
        public int Height { get; set; }
        public bool KillsOnSight { get; set; }
        public string Model { get; set; }
        public Portrait Portrait { get; set; }
        public Race Race { get; set; }
        public byte RunFactor { get; set; }
        public int Weight { get; set; }
        public ushort MovementRange { get; set; }
    }

}
