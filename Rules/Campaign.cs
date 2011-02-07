using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Rules
{
   /// <summary>
    /// A campaign describes the currently running game with all attached rule objects, like the party,
    /// players or visited levels.
    /// </summary>
    public class Campaign
    {

        public Party Party { get; set; }
        
    }
}
