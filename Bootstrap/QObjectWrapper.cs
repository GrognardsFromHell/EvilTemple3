using System;
using System.Collections.Generic;
using System.Linq;
using System.Linq.Expressions;
using System.Runtime.CompilerServices;
using System.Text;
using System.Dynamic;

namespace Bootstrap
{
    /// <summary>
    /// Wraps a QObject using a dynamic object, allowing the use of Slots/Signals without actually writing
    /// a special wrapper-class per QObject subclass.
    /// </summary>
    public sealed class QObjectWrapper : DynamicObject, IDisposable
    {
        private IntPtr _handle;

        public QObjectWrapper(IntPtr handle)
        {
            Console.WriteLine("Constructor for QObjectWrapper(" + handle + ")");
            _handle = handle;
        }

        ~QObjectWrapper()
        {
            Dispose();
        }

        public void Dispose()
        {
            if (_handle != IntPtr.Zero)
            {
                FreeHandle(_handle);
                _handle = IntPtr.Zero;
                GC.SuppressFinalize(this);
            }
        }

        public override bool TryInvokeMember(InvokeMemberBinder binder, object[] args, out object result)
        {
            // Named parameters are unsupported by C++
            if (binder.CallInfo.ArgumentNames.Count > 0)
                throw new InvalidOperationException("Named parameters are unsupported for calls to Qt.");

            return InvokeMember(_handle, binder.Name, args, out result);
        }

        public override bool TryGetMember(GetMemberBinder binder, out object result)
        {
            return GetProperty(_handle, binder.Name, out result);
        }

        public override bool TrySetMember(SetMemberBinder binder, object value)
        {
            return SetProperty(_handle, binder.Name, value);
        }

        #region Internal Calls
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void FreeHandle(IntPtr handle);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool InvokeMember(IntPtr handle, string name, object[] args, out object result);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool GetProperty(IntPtr handle, string name, out object result);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool SetProperty(IntPtr handle, string name, object value);
        #endregion
    }
}
