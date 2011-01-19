using System;
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
            Console.WriteLine("Called main method.");

            IGameWindow window = new GameWindow();

            window.WindowTitle = "EvilTemple";

            var b = new Bootstrapper();
            window.DrawFrameEvent += b.DrawFrame;
            return 0;
        }


        private static bool _inited = false;
        
        private void DrawFrame(dynamic obj)
        {
            if (!_inited)
            {
                Console.WriteLine("Registering Event on obj: " + obj);
                obj.newGameClicked((Action)(() => Console.WriteLine("asd")));
                _inited = true;
            }
        }

    }

}
