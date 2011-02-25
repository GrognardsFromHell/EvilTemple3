using System;
using System.Collections.Generic;
using EvilTemple.Runtime;

namespace Game
{

    public class EventBus : IEventBus
    {

        private readonly IDictionary<Type, object> _handlers;

        public EventBus()
        {
            _handlers = new Dictionary<Type, object>();
        }

        public void Register<T>(MessageHandler<T> h) where T : IMessage
        {
            getHandlers<T>().Add(h);
        }

        public void Unregister<T>(MessageHandler<T> h) where T : IMessage
        {
            getHandlers<T>().Remove(h);
        }

        public void Send<T>(T m) where T : IMessage
        {
            object handlerBaseList;
            if (!_handlers.TryGetValue(typeof(T), out handlerBaseList)) return;

            var handlerList = (IList<MessageHandler<T>>)handlerBaseList;

            var handlerCopy = new MessageHandler<T>[handlerList.Count];
            handlerList.CopyTo(handlerCopy, 0);

            /*
             * We operate on a copy of the handler list here, since 
             * the actual handlers may register new handlers on the 
             * same type of message.
             */
            foreach (var handler in handlerCopy)
                handler(m);
        }

        private IList<MessageHandler<T>> getHandlers<T>() where T : IMessage
        {
            object handlerList;
            if (_handlers.TryGetValue(typeof(T), out handlerList))
                return (IList<MessageHandler<T>>)handlerList;

            // Lazily create a list
            handlerList = new List<MessageHandler<T>>();
            _handlers.Add(typeof(T), handlerList);
            return (IList<MessageHandler<T>>)handlerList;
        }

    }
}
