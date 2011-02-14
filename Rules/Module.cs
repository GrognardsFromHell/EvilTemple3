using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Ninject.Modules;

namespace Rules
{
    public class Module : NinjectModule
    {
        public override void Load()
        {
            Bind<Races>().ToSelf().InSingletonScope();
        }
    }
}
