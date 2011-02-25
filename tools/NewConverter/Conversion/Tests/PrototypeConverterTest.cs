using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using NUnit.Framework;
using TroikaFormats;

namespace Conversion.Tests
{
    class PrototypeConverterTest
    {

        [Test]
        public void SmokeTest()
        {
            var vfs = DefaultArchives.Create();
            var pt = new PrototypeConverter(vfs);

            var sw = new Stopwatch();
            sw.Start();
            pt.Run();
            sw.Stop();

            Console.WriteLine(sw.Elapsed);
        }

    }
}
