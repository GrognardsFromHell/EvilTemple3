using System.Collections.Generic;

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

        public static Dictionary<string, object> objectDict;

    }
}
