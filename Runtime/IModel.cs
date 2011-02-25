using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace EvilTemple.Runtime
{
    /// <summary>
    /// Represents a three dimensional model.
    /// </summary>
    public interface IModel
    {
        float Radius { get; }

        float RadiusSquared { get; }

        IList<string> Animations { get; }

        bool HasAnimation(string animationId);

        IAnimation GetAnimation(string animationId);
    }
}
