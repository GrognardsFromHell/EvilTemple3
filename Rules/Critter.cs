using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Rules
{
    public class Critter : BaseObject
    {

        public bool Concealed { get; set; }

        public ushort MovementRange { get; set; }

        public byte RunFactor { get; set; }

        public bool KillsOnSight { get; set; }

        public uint ExperiencePoints { get; set; }

        public string Portrait { get; set; }

        public Gender Gender { get; set; }

        public Critter()
        {
            MovementRange = 250;
            RunFactor = 3;
            DrawBehindWalls = true;
            KillsOnSight = false;
        }

    }
}
