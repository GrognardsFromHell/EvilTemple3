using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using NUnit.Framework;

namespace TroikaFormats.Tests
{
    class MessageFileTests
    {

        [Test]
        public void SmokeTest()
        {
            var vfs = DefaultArchives.Create();
            var mfs = new MessageFileSource(vfs);

            var mapList = mfs.Load("rules/MapList.mes");

            foreach (var entry in mapList)
            {
                Console.WriteLine(entry);
            }
        }

    }
}
