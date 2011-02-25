using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace EvilTemple.Runtime
{
    /// <summary>
    /// Represents the model registry.
    /// </summary>
    public interface IModels
    {
        IModel Load(string filename);
    }
}
