using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace EvilTemple.Runtime.Messages
{
    /// <summary>
    /// This message is sent, after all modules are 
    /// loaded and before the main loop is started.
    /// </summary>
    public sealed class ApplicationStartup : IMessage
    {
    }
}
