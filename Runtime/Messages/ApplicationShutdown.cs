using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace EvilTemple.Runtime.Messages
{
    /// <summary>
    /// This message is sent right before the application is shut down.
    /// </summary>
    public sealed class ApplicationShutdown : IMessage
    {
    }
}
