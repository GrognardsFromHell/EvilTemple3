namespace EvilTemple.Runtime
{
    public interface IAnimation
    {
        string Name { get; }

        uint Frames { get; }

        float FrameRate { get; }

        float DistancePerSecond { get; }

        bool Loopable { get; }

        AnimationDriveType DriveType { get; }

        AnimationEvent[] Events { get; }
    }

    public enum AnimationDriveType {
        Time = 0,
        Distance,
        Rotation,
    }
    
    public struct AnimationEvent
    {
        public uint Frame;
        public AnimationEventType Type;
        public string Content;
    }

    public enum AnimationEventType
    {
        Script = 0,
        Action = 1
    }

}
