using System;
using System.Diagnostics;
using NUnit.Framework;

namespace TroikaFormats.Tests
{
    class PrototypesTest
    {

        [Test]
        public void SmokeTest()
        {
            var vfs = DefaultArchives.Create(@"D:\Temple of Elemental Evil");

            var sw = new Stopwatch();
            sw.Start();
            var prototypes = new Prototypes(vfs);
            sw.Stop();

            Console.WriteLine("Loaded Prototypes in " + sw.ElapsedMilliseconds + "ms");
        }

    }
}
