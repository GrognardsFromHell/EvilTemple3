using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Runtime;

namespace Bootstrap
{
    class SystemObjects
    {

        private static readonly Dictionary<string, dynamic> _systemObjects = new Dictionary<string, dynamic>();

        public static IGameView GameView { get; private set; }

        static void Add(string name, object systemObject)
        {
            Console.WriteLine("Registering system object " + name);

            switch (name)
            {
                case "GameView":
                    GameView = new Interop.GameView(systemObject);
                    break;

                default:
                    throw new InvalidOperationException("Unknown system object name: " + name);
            }
        }
    }
}
