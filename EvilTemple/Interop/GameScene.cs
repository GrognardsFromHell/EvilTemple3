using Runtime;

namespace EvilTemple.Interop
{
    class GameScene : IGameScene
    {

        private readonly dynamic _object;

        public GameScene(dynamic obj)
        {
            _object = obj;
        }

    }
}
