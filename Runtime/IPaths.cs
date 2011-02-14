using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Runtime
{
    public interface IPaths
    {

        string UserDataPath { get; }

        string GeneratedDataPath { get; }

        string InstallationPath { get; }

    }
}
