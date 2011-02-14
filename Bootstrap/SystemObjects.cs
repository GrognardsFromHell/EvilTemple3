using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Bootstrap.Interop;
using Ninject.Modules;
using Runtime;

namespace Bootstrap
{
    class SystemObjects
    {
        
        public static IGameView GameView { get; private set; }

        public static IGameWindow GameWindow { get; private set; }

        public static IPaths Paths { get; private set; }

        static void Add(string name, object systemObject)
        {
            Console.WriteLine("Registering system object " + name);

            switch (name)
            {
                case "GameView":
                    GameView = new GameView(systemObject);
                    break;

                case "GameWindow":
                    GameWindow = new GameWindow(systemObject);
                    break;

                case "Paths":
                    Paths = new Paths(systemObject);
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
            return GameView != null && GameWindow != null && Paths != null;
        }

        /// <summary>
        /// Creates a Ninject module that contains the currently registered system objects.
        /// </summary>
        /// <returns>A Ninject module that provides the system objects.</returns>
        public static INinjectModule CreateModule()
        {
            CheckConsistency();
            return new Module();
        }

        private class Module : NinjectModule
        {
            public override void Load()
            {
                Bind<IGameView>().ToConstant(GameView);
                Bind<IGameWindow>().ToConstant(GameWindow);
                Bind<IPaths>().ToConstant(Paths);
            }
        }
    }
}
