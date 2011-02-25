using System;
using EvilTemple.Runtime;

namespace Gui
{
    public class MainMenu : Menu
    {
        private readonly IGameView _gameView;
        private readonly PartyCreation _partyCreation;

        public event Action<MainMenu> OnExitGame;
        
        private void InvokeOnExitGame()
        {
            var handler = OnExitGame;
            if (handler != null) handler(this);
        }

        public MainMenu(IGameView gameView, PartyCreation partyCreation)
        {
            _gameView = gameView;
            _partyCreation = partyCreation;
            _partyCreation.OnCancel += ShowMainMenu;
        }

        public void ShowMainMenu()
        {
            var dialog = _gameView.AddInterfaceItem("interface/MainMenu.qml");

            // Bind to events on the menu
            dialog.newGameClicked += (Action) StartPartyCreation;
            dialog.exitGameClicked += (Action)InvokeOnExitGame;

            CurrentMenu = dialog;
        }

        void StartPartyCreation()
        {
            CurrentMenu = null;

            _partyCreation.Show();
        }

    }

}
