using System;
using System.Runtime.CompilerServices;
using Runtime;

namespace Bootstrap
{

    class GameWindow : IGameWindow
    {
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private static extern void SetWindowTitle(string title);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private static extern string GetWindowTitle();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private static extern void AddDrawFrameEventHandler(DrawFrameHandler handler);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private static extern void RemoveDrawFrameEventHandler(DrawFrameHandler handler);
        
        public string WindowTitle
        {
            get { return GetWindowTitle(); }
            set { SetWindowTitle(value); }
        }

        public int Width
        {
            get { throw new NotImplementedException(); }
            set { throw new NotImplementedException(); }
        }

        public int Height
        {
            get { throw new NotImplementedException(); }
            set { throw new NotImplementedException(); }
        }

        public bool Foreground
        {
            get { throw new NotImplementedException(); }
        }

        public bool Minimized
        {
            get { throw new NotImplementedException(); }
        }

        public bool Maximized
        {
            get { throw new NotImplementedException(); }
            set { throw new NotImplementedException(); }
        }

        public bool FullScreen
        {
            get { throw new NotImplementedException(); }
            set { throw new NotImplementedException(); }
        }

        public event DrawFrameHandler DrawFrameEvent
        {
            add
            {
                Console.WriteLine("Adding event handler.");
                // Whatever EventHandler object we have here, needs to passed on to C++
                AddDrawFrameEventHandler(value);
            }
            remove
            {
                Console.WriteLine("Remove event handler.");
                RemoveDrawFrameEventHandler(value);
            }
        }

    }
}
