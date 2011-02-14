using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Ninject.Modules;
using Runtime;

namespace Game
{
    public class GameModule : NinjectModule
    {
        public override void Load()
        {
            Bind<IEventBus>().ToConstant(new EventBus());
            Bind<CharacterVault>().ToSelf();
        }
    }
}
