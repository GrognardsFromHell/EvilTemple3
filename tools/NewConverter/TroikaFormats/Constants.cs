namespace TroikaFormats
{
    /// <summary>
    ///   Constants commonly used within the Troika data files.
    /// </summary>
    public class Constants
    {
        /// <summary>
        ///   The number of pixels per world coordinate unit.
        /// </summary>
        public const float PixelPerWorldTile = 28.2842703f;

        /// <summary>
        ///   The original game applies an additional base rotation to everything in order to align it
        ///   with the isometric grid. This is the radians value of that rotation.
        /// </summary>
        public const float LegacyBaseRotation = 0.77539754f;
    }
}