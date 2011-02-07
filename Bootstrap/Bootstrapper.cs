using System;
using Gui;

namespace Bootstrap
{

    public sealed class Bootstrapper
    {

        /// <summary>
        /// This method is called by the startup code from C++. The actual game will be launched, after this method
        /// returns. This is due to internal workings of Mono and this method may be removed in the future.
        /// </summary>
        /// <param name="main">Arguments passed to the main program.</param>
        /// <returns>Always 0.</returns>
        public static int Main(string[] main)
        {
            return 0;
        }

        public static void Startup()
        {
            Console.WriteLine("Application startup hook called.");

            if (!SystemObjects.CheckConsistency())
                throw new InvalidOperationException("Not all system objects have been set by the C++ code.");

            var mainMenu = new MainMenu(SystemObjects.GameView);
            mainMenu.OnExitGame += ExitGame;
            mainMenu.ShowMainMenu();

            /*IGameWindow gw = new GameWindow(gameWindow);
            gw.WindowTitle = "EvilTemple";

            var b = new Bootstrapper();
            gw.DrawFrameEvent += b.DrawFrame;*/
            /*
            Console.WriteLine("Registering Event on obj: " + obj);
            obj.newGameClicked += (NewGameClicked)((x, y) => Console.WriteLine("asd: " + x + " y: " + y));

            List<object> testF = obj.testFunction("yadaadada");
            Console.WriteLine("Result from function call: " + testF[0] + testF[1]);

            Console.WriteLine("Initial: " + obj.testProp);

            obj.testProp = "New Value";

            var color = Color.FromArgb(255, 0, 0, 128);

            obj.color = color;*/
        }

        private static void ExitGame(MainMenu menu)
        {
            SystemObjects.GameWindow.Close();
        }

        public static void DrawFrame()
        {
        }

    }

}
