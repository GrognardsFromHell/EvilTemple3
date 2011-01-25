using System;
using System.Runtime.CompilerServices;

namespace Bootstrap
{
    public sealed class EventSubscriptionHelper
    {
        private readonly IntPtr _handle;

        private readonly string _name;

        public EventSubscriptionHelper(IntPtr handle, string name)
        {
            _handle = handle;
            _name = name;
            Console.WriteLine("Constructing event subscription helper for " + handle + " " + name);
        }

        public static EventSubscriptionHelper operator +(EventSubscriptionHelper helper, Delegate handler)
        {
            ConnectToSignal(helper._handle, helper._name, handler);
            return helper;
        }

        public static EventSubscriptionHelper operator -(EventSubscriptionHelper helper, Delegate handler)
        {
            DisconnectFromSignal(helper._handle, helper._name, handler);
            return helper;
        }

        #region Internal Calls
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void ConnectToSignal(IntPtr handle, string signalName, Delegate handler);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void DisconnectFromSignal(IntPtr handle, string signalName, Delegate handler);
        #endregion
    }
}
