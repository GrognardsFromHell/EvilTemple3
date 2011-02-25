using System;

namespace EvilTemple.Runtime
{

    public delegate void DrawFrameHandler();

    /// <summary>
    /// The window used to display graphical game data. There can only be one instance of this, and it
    /// is already passed in by the startup code.
    /// </summary>
    public interface IGameWindow
    {
        /// <summary>
        /// The window title.
        /// </summary>
        string WindowTitle { get; set; }

        /// <summary>
        /// Width of the window in pixels. This may include the window border.
        /// </summary>
        int Width { get; set; }

        /// <summary>
        /// The window height in pixels. This may include 
        /// </summary>
        int Height { get; set; }

        /// <summary>
        /// Indicates that the window is currently in the foreground and accepts user input.
        /// </summary>
        bool Foreground { get; }

        /// <summary>
        /// Indicates that the window has been minimized to the task bar.
        /// </summary>
        bool Minimized { get; }

        /// <summary>
        /// Indicates that the window is maximized.
        /// </summary>
        bool Maximized { get; set; }

        /// <summary>
        /// Indicates that the window is running in full-screen mode, without a border.
        /// </summary>
        bool FullScreen { get; set; }

        /// <summary>
        /// Closes the game window and shuts down the application.
        /// </summary>
        void Close();
    }

}
