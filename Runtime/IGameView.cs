namespace EvilTemple.Runtime
{
    public interface IGameView
    {

        IScene Scene { get; }

        /// <summary>
        /// Creates and adds an interface item to the game view.
        /// </summary>
        /// <param name="url">A URL pointing to the QML file.</param>
        /// <returns>The added item.</returns>
        dynamic AddInterfaceItem(string url);

        /// <summary>
        /// Plays a movie as an overlay over the game.
        /// </summary>
        /// <param name="filename">The filename of the movie, relative to the data root.</param>
        /// <returns>True if the movie started playing successfully, false otherwise.</returns>
        bool PlayMovie(string filename);

        /// <summary>
        /// Close the game window.
        /// </summary>
        void Close();

    }
}
