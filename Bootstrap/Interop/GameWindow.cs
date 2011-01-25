using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Runtime;

namespace Bootstrap.Interop
{
    class GameWindow : IGameWindow
    {

        private readonly dynamic _gameWindow;

        public GameWindow(dynamic gameWindow)
        {
            _gameWindow = gameWindow;
        }

        public string WindowTitle
        {
            get { return _gameWindow.windowTitle; }
            set { _gameWindow.windowTitle = value; }
        }

        public int Width
        {
            get { return _gameWindow.width; }
            set { _gameWindow.width = value; }
        }

        public int Height
        {
            get { return _gameWindow.height; }
            set { _gameWindow.height = value; }
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
            add { _gameWindow.drawFrameEvent += value; }
            remove { _gameWindow.drawFrameEvent -= value; }
        }
    }
}
