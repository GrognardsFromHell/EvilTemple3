using System;
using Rules;
using Runtime;

namespace Gui
{
    public class MainMenu : Menu
    {

        private readonly IGameView _gameView;

        public event Action<MainMenu> OnExitGame;

        private void InvokeOnExitGame()
        {
            var handler = OnExitGame;
            if (handler != null) handler(this);
        }

        public MainMenu(IGameView gameView)
        {
            _gameView = gameView;
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

            var partyCreation = new PartyCreation(_gameView);
            partyCreation.OnCancel += ShowMainMenu;
            partyCreation.Show();
        }

    }

}
