using Runtime;

namespace EvilTemple.Interop
{
    class GameView : IGameView
    {

        private readonly dynamic _object;

        public GameView(dynamic wrappedObject) 
        {
            _object = wrappedObject;
        }

        public dynamic AddInterfaceItem(string url)
        {
            return _object.addGuiItem(url);
        }

        public bool PlayMovie(string filename)
        {
            return _object.playMovie(filename);
        }
        
        public IGameScene Scene
        {
            get
            {
                return new GameScene(_object.scene);
            }
        }

    }
}
