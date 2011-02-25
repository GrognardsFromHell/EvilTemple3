using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace EvilTemple.Runtime
{

    public interface IScene
    {
        int ObjectsDrawn { get; }

        ISceneNode CreateNode();

        void Add(ISceneNode node);

        void Remove(ISceneNode node);

        void Clear();

        void AddOverlayText(ref Vec4 position, string text, ref Vec4 color, float lifetime);
    }

    public interface ISceneNode
    {
    }

}
