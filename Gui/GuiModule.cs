using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Ninject.Modules;
using Ninject;
using EvilTemple.Runtime;
using EvilTemple.Runtime.Messages;

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
            var gameView = Kernel.Get<IGameView>();

            var mainMenu = Kernel.Get<MainMenu>();
            mainMenu.OnExitGame += delegate { gameView.Close(); };
            mainMenu.ShowMainMenu();
        }
    }
}
