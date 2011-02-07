using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Rules;
using Runtime;

namespace Gui
{
    /// <summary>
    /// Manages the party creation dialog.
    /// </summary>
    public class PartyCreation : Menu
    {
        private readonly IGameView _gameView;

        public PartyCreation(IGameView gameView)
        {
            _gameView = gameView;
        }

        public event Action OnCancel;

        public Alignment PartyAlignment { get; private set; }

        public void Show()
        {
            ShowSelectAlignment();
        }

        void ShowSelectAlignment()
        {
            CurrentMenu = _gameView.AddInterfaceItem("interface/ChoosePartyAlignment.qml");

            // Bind events to the menu
            CurrentMenu.cancelled += (Action)Cancel;
            CurrentMenu.alignmentSelected += (Action<string>)(x =>
            {
                PartyAlignment = AlignmentHelper.ConvertFromGui(x);
                ShowPartyCreation();
            });
        }

        void ShowPartyCreation()
        {
            var dialog = _gameView.AddInterfaceItem("interface/PartyVault.qml");

            dialog.characters = GetCharacters();
            dialog.closeClicked += (Action) Cancel;

            CurrentMenu = dialog;
        }
        
        private void Cancel()
        {
            CurrentMenu = null;
            InvokeOnCancel();
        }

        private void InvokeOnCancel()
        {
            var handler = OnCancel;
            if (handler != null) handler();
        }

        private IList<object> GetCharacters()
        {
            return new List<object> {
                GetCharacter()
            };
        }

        private IDictionary<string, object> GetCharacter()
        {
            return new Dictionary<string, object>
                       {
                           {"id", ""},
                           {"name", ""},
                           {"portrait", "portrait"},
                           {"gender", "gender"},
                           {"race", "race"},
                           {"alignment", ""},
                           {"classes", ""},
                           {"filename", "..."},
                           {"compatible", "compatible"}
                       };
        }

    }
    
    static class AlignmentHelper
    {
        private static readonly string[] Alignments = { "lg", "ng", "cg", "ln", "n", "cn", "le", "ne", "ce" };

        public static Alignment ConvertFromGui(string selectedAlignment)
        {
            for (var i = 0; i < Alignments.Length; ++i)
                if (Alignments[i] == selectedAlignment)
                    return (Alignment)i;

            throw new InvalidOperationException("Unknown alignment received from GUI: " + selectedAlignment);
        }
    }

}
