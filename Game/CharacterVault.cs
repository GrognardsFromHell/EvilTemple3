using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Rules;

namespace Game
{
    public class CharacterVault
    {

        private readonly string _pregeneratedPath;

        private readonly string _userPath;

        /// <summary>
        /// Constructs a character vault object.
        /// </summary>
        /// <param name="pregeneratedPath">The path to the directory that contains pregenerated characters.</param>
        /// <param name="userPath">The path to the directory containing user generated characters.</param>
        public CharacterVault(string pregeneratedPath, string userPath)
        {
            _pregeneratedPath = pregeneratedPath;
            _userPath = userPath;
        }

        /// <summary>
        /// Gets all characters stored in the character vault.
        /// </summary>
        public IList<PlayerCharacter> Characters { get { return ReadCharacters(); } }

        /// <summary>
        /// Adds a new character to the vault and returns the corresponding filename.
        /// </summary>
        /// <param name="character">The character to be added.</param>
        /// <returns>The filename generated in the user directory.</returns>
        public string Add(PlayerCharacter character)
        {
            return "";
        }

        /// <summary>
        ///  Removes a character from the vault given its filename. Only user generated characters may be removed.
        /// </summary>
        /// <param name="filename">Filename relative to the user generated directory.</param>
        public void Remove(string filename)
        {
            
        }

        private IList<PlayerCharacter> ReadCharacters()
        {
            return new List<PlayerCharacter>();
        }
    }
    
}
