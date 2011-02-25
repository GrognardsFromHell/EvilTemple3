using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
using EvilTemple.Runtime;

namespace EvilTemple
{
    class Paths : IPaths
    {
        public string UserDataPath
        {
            get { return ""; }
        }

        public string GeneratedDataPath
        {
            get { return @"C:\Users\Sebastian\AppData\Local\EvilTemple\"; }
        }

        public string InstallationPath
        {
            get { var assembly = Assembly.GetExecutingAssembly();
                var binDir = Path.GetDirectoryName(assembly.Location);
                binDir += Path.DirectorySeparatorChar + ".." + Path.DirectorySeparatorChar;
                return Path.GetFullPath(binDir);
            }
        }
    }
}
