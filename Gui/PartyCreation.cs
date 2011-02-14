using System;
using System.Collections.Generic;
using Game;
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
        
        private readonly CharacterVault _vault;

        private readonly CreateCharacter _createCharacter;

        public PartyCreation(IGameView gameView, CharacterVault vault, CreateCharacter createCharacter)
        {
            _gameView = gameView;
            _vault = vault;
            _createCharacter = createCharacter;
            createCharacter.OnCancel += Show;
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
            dialog.createCharacterClicked += (Action) RequestCharacterCreation;

            CurrentMenu = dialog;
        }

        private void RequestCharacterCreation()
        {
            CurrentMenu = null;

            _createCharacter.Show();
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

        private IList<IDictionary<string, string>> GetCharacters()
        {
            return new List<IDictionary<string, string>> {
                GetCharacter()
            };
        }

        private IDictionary<string, string> GetCharacter()
        {
            Console.WriteLine(_vault.Characters);

            return new Dictionary<string, string>
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
