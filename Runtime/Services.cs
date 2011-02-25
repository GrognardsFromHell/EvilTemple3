using Microsoft.Practices.ServiceLocation;

namespace EvilTemple.Runtime
{
    public static class Services
    {

        public static IEventBus EventBus
        {
            get { return Get<IEventBus>(); }
        }

        public static IModels Models
        {
            get { return Get<IModels>(); }
        }

        private static T Get<T>() where T : class
        {
            return ServiceLocator.Current.GetInstance<T>();
        }

    }
}
