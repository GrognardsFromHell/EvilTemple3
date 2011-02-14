using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;

namespace Bootstrap
{
    /// <summary>
    /// This class exists to work around a Mono deficiency, currently making it impossible to create
    /// generic type instantiations. Every generic type instantiation we require has to be present
    /// on this class, as a field.
    /// </summary>
    public class GenericHelper
    {
        public static List<object> objectList;

        public static IList<object> objectIList;

        public static Dictionary<string, object> objectDict;

        public static IDictionary<string, object> objectIDict;

        static void UnwrapList(IList<object> list, IntPtr nativeListHandle)
        {
            Console.WriteLine("Unwrapping list. " + nativeListHandle.ToString("X"));
            foreach (var value in list)
                AddVariantListItem(nativeListHandle, value);
        }

        static void UnwrapDictionary(IDictionary<string, object> obj, IntPtr unwrapHandle)
        {
            Console.WriteLine("Unwrapping map. " + unwrapHandle.ToString("X"));
            foreach (var kvp in obj)
            {
                AddVariantMapItem(unwrapHandle, kvp.Key, kvp.Value);
            }
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void AddVariantListItem(IntPtr handle, object value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void AddVariantMapItem(IntPtr handle, string key, object value);

    }
}
