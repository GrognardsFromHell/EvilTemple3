using System;
using System.Collections.Generic;
using System.Drawing;
using Bootstrap;
using Bootstrap.Interop;
using Runtime;

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

        private delegate void NewGameClicked(string f, int otherArg);

        public void ApplicationStartup(dynamic gameWindow)
        {
            IGameWindow gw = new GameWindow(gameWindow);
            gw.WindowTitle = "EvilTemple";

            var b = new Bootstrapper();
            gw.DrawFrameEvent += b.DrawFrame;
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

        public void DrawFrame(dynamic obj)
        {
            
        }

    }

}
