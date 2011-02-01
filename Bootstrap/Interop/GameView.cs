using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Runtime;

namespace Bootstrap.Interop
{
    class GameView : IGameView
    {

        private readonly dynamic _object;

        public GameView(dynamic wrappedObject) 
        {
            _object = wrappedObject;
        }

        public dynamic AddInterfaceItem(string url)
        {
            return _object.addGuiItem(url);
        }

    }
}
