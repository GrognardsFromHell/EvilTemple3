using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using TroikaFormats;

namespace TFHarness
{
    class Program
    {
        static void Main(string[] args)
        {
            var vfs = DefaultArchives.Create(@"D:\Temple of Elemental Evil");

            var sw = new Stopwatch();
            sw.Start();
            var prototypes = new Prototypes(vfs);
            sw.Stop();

            Console.WriteLine(sw.ElapsedMilliseconds);

            var prototype = prototypes[1];
            Console.WriteLine(prototype);

            Console.ReadKey();
        }
    }
}
