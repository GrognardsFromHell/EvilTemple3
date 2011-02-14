using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Runtime;

namespace Bootstrap.Interop
{

    public class Paths : IPaths
    {

        private readonly dynamic _paths;

        public Paths(dynamic paths)
        {
            _paths = paths;
        }

        public string UserDataPath
        {
            get { return _paths.userDataPath; }
        }

        public string GeneratedDataPath
        {
            get { return _paths.generatedDataPath; }
        }

        public string InstallationPath
        {
            get { return _paths.installationPath; }
        }
    }

}
