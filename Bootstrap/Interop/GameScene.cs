using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Runtime;

namespace Bootstrap.Interop
{
    class GameScene : IGameScene
    {

        private readonly dynamic _object;

        public GameScene(dynamic obj)
        {
            _object = obj;
        }

    }
}
