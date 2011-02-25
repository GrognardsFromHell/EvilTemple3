using System;
using System.Collections.Generic;
using Rules;
using EvilTemple.Runtime;

namespace Game
{
    public class CharacterVault
    {

        private readonly IPaths _paths;

        /// <summary>
        /// Constructs a character vault object.
        /// </summary>
        /// <param name="paths">Provides path information for several of the game's data files.</param>
        public CharacterVault(IPaths paths)
        {
            _paths = paths;
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
            Console.WriteLine("Reading Characters: " + _paths.GeneratedDataPath);

            return new List<PlayerCharacter>();
        }
    }
    
}
