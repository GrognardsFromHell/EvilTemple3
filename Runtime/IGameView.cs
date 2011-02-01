using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Runtime
{
    public interface IGameView
    {

        /// <summary>
        /// Creates and adds an interface item to the game view.
        /// </summary>
        /// <param name="url">A URL pointing to the QML file.</param>
        /// <returns>The added item.</returns>
        dynamic AddInterfaceItem(string url);

    }
}
