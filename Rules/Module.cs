using Ninject.Modules;

namespace Rules
{
    public class Module : NinjectModule
    {

        public override string Name
        {
            get { return "Rules"; }
        }

        public override void Load()
        {
            Bind<Races>().ToSelf().InSingletonScope();
        }
    }
}
