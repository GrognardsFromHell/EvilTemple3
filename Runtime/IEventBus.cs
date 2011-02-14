using System;
using System.Collections.Generic;

namespace Runtime
{

    public delegate void MessageHandler<in T>(T message) where T : IMessage;

    public interface IMessage
    {
    }

    public interface IEventBus
    {
        void Register<T>(MessageHandler<T> h) where T : IMessage;

        void Unregister<T>(MessageHandler<T> h) where T : IMessage;

        void Send<T>(T m) where T : IMessage;
    }

    public static class EventBusExtensions
    {
        private static readonly IDictionary<Type, object> Singletons = new Dictionary<Type, object>();

        public static void Send<T>(this IEventBus eventBus) where T : IMessage
        {
            object singletonObj;
            if (!Singletons.TryGetValue(typeof(T), out singletonObj))
            {
                singletonObj = typeof (T).GetConstructor(new Type[0]).Invoke(new object[0]);
                Singletons.Add(typeof (T), singletonObj);
            }

            eventBus.Send((T)singletonObj);
        }
    }

}
