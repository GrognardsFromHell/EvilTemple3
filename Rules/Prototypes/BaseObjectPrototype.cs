using System.Xml.Serialization;

namespace Rules.Prototypes
{
    public class BaseObjectPrototype
    {
        public string Id { get; set; }

        public uint Scale { get; set; }

        public uint Rotation { get; set; }

        public bool Interactive { get; set; }

        public bool DrawBehindWalls { get; set; }

        public uint HitPoints { get; set; }

        public uint TemporaryHitPoints { get; set; }

        public uint DamageTaken { get; set; }

        public uint SubdualDamageTaken { get; set; }

        public string IndividualName { get; set; }

        public BaseObjectPrototype()
        {
            Scale = 100;
            Rotation = 0;
            Interactive = true;
            DrawBehindWalls = false;
            HitPoints = 1;
        }

        public readonly static BaseObjectPrototype Default = new BaseObjectPrototype();
    }

}
