using System;
using System.IO;
using System.Reflection;
using CommonServiceLocator.NinjectAdapter;
using EngineInterop;
using Microsoft.Practices.ServiceLocation;
using Ninject;
using Ninject.Modules;
using Ninject.Syntax;
using EvilTemple.Runtime;
using EvilTemple.Runtime.Messages;

namespace EvilTemple
{
    internal static class Startup
    {
        [STAThread]
        public static void Main(string[] args)
        {
            IKernel kernel = new StandardKernel();
            var locator = new NinjectServiceLocator(kernel);
            ServiceLocator.SetLocatorProvider(() => locator);

            using (var resourceManager = new ResourceManager())
            {
                LoadResources(resourceManager);
                
                using (var engine = new Engine(args))
                {
                    // Add several objects provided only by the engine
                    AddEngineObjects(kernel, engine);

                    // Initialize all the other modules that depend on the engine
                    InitializeModules(kernel);

                    var eventBus = kernel.Get<IEventBus>();
                    eventBus.Send<ApplicationStartup>();

                    // Subscribe to the events the engine provides
                    engine.OnDrawFrame += OnDrawFrame;

                    // Run the engine main loop
                    engine.Run();

                    eventBus.Send<ApplicationShutdown>();
                }
            }
        }

        private static void LoadResources(ResourceManager resourceManager)
        {
            var paths = new Paths();

            resourceManager.OverrideDataPath = Path.Combine(paths.InstallationPath, "data");
            
            foreach (var archive in Directory.EnumerateFiles(paths.GeneratedDataPath, "*.zip"))
            {
                if (!resourceManager.AddArchive(archive))
                {
                    Console.WriteLine("Unable to add archive: " + archive);
                }
            }
        }

        private static void AddEngineObjects(IBindingRoot kernel, Engine engine)
        {
            kernel.Bind<IGameView>().ToConstant(engine.GameView);
            kernel.Bind<IScene>().ToConstant(engine.Scene);
            kernel.Bind<IModels>().ToConstant(engine.Models);
            kernel.Bind<IPaths>().ToConstant(new Paths());
        }

        private static void InitializeModules(IKernel kernel)
        {
            LoadModule("Rules.dll", kernel);
            LoadModule("D20Rules.dll", kernel);
            LoadModule("Game.dll", kernel);
            LoadModule("Gui.dll", kernel);
        }

        private static void LoadModule(string filename, IKernel kernel)
        {
            var startupAssembly = Assembly.GetExecutingAssembly();
            var path = Path.Combine(Path.GetDirectoryName(startupAssembly.Location), filename);

            var guiAssembly = Assembly.LoadFile(path);
            var types = guiAssembly.GetTypes();
            foreach (var type in types)
            {
                if (!type.IsClass || type.IsAbstract)
                    continue;

                if (!typeof(INinjectModule).IsAssignableFrom(type))
                    continue;

                Console.WriteLine("Found Ninject module " + type + " in " + filename);

                var obj = type.GetConstructor(new Type[0]).Invoke(new object[0]);
                var module = obj as INinjectModule;
                kernel.Load(module);
            }
        }

        private static void OnDrawFrame()
        {
        }
    }
}