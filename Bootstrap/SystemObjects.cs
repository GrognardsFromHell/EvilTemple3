using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Runtime;

namespace Bootstrap
{
    class SystemObjects
    {
        
        public static IGameView GameView { get; private set; }

        public static IGameWindow GameWindow { get; private set; }

        static void Add(string name, object systemObject)
        {
            Console.WriteLine("Registering system object " + name);

            switch (name)
            {
                case "GameView":
                    GameView = new Interop.GameView(systemObject);
                    break;

                case "GameWindow":
                    GameWindow = new Interop.GameWindow(systemObject);
                    break;

                default:
                    throw new InvalidOperationException("Unknown system object name: " + name);
            }
        }

        /// <summary>
        /// Checks that all the properties of this object have been set.
        /// </summary>
        public static bool CheckConsistency()
        {
            return GameView != null && GameWindow != null;
        }
    }
}
