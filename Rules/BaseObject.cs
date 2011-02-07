using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Rules
{
    public class BaseObject
    {
        public uint Scale { get; set; }

        public uint Rotation { get; set; }

        public bool Interactive { get; set; }

        public bool DrawBehindWalls { get; set; }
        
        public uint HitPoints { get; set; }

        public uint TemporaryHitPoints { get; set; }

        public uint DamageTaken { get; set; }

        public uint SubdualDamageTaken { get; set; }

        public string IndividualName { get; set; }

        public string Name {
            get { return IndividualName; }
        }
        
        public BaseObject()
        {
            Scale = 100;
            Rotation = 0;
            Interactive = true;
            DrawBehindWalls = false;
            HitPoints = 1;
        }

        
        
    }
}
