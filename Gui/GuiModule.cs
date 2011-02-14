using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Ninject.Modules;
using Ninject;
using Runtime;
using Runtime.Messages;

namespace Gui
{
    public class GuiModule : NinjectModule
    {
        public override void Load()
        {
            Bind<MainMenu>().ToSelf();
            Bind<PartyCreation>().ToSelf();
            Bind<CreateCharacter>().ToSelf();

            Kernel.Get<IEventBus>().Register<ApplicationStartup>(ShowMainMenu);
        }

        private void ShowMainMenu(ApplicationStartup message)
        {
            var gameWindow = Kernel.Get<IGameWindow>();

            var mainMenu = Kernel.Get<MainMenu>();
            mainMenu.OnExitGame += delegate { gameWindow.Close(); };
            mainMenu.ShowMainMenu();
        }
    }
}
