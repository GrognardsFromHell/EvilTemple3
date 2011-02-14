using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
using Ninject;
using Ninject.Modules;
using Rules;

namespace D20Rules
{
    public class Module : NinjectModule
    {
        public override string Name
        {
            get { return "D20Module"; }
        }

        public override void Load()
        {
            var races = Kernel.Get<Races>();
            races.Load(GetResource("D20Rules.Races.xml"));
        }

        private static Stream GetResource(string filename)
        {
            var executingAssembly = Assembly.GetExecutingAssembly();

            var stream = executingAssembly.GetManifestResourceStream(filename);
            if (stream == null)
                throw new FileNotFoundException("Unable to find resource " + filename + " in assembly " + executingAssembly.Location);
            return stream;
        }
    }
}
