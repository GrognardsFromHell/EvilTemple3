using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Xml;
using System.Xml.Serialization;
using NUnit.Framework;
using Rules;

namespace RulesTests
{
    class RaceTest
    {
        private static Stream GetResource(string name)
        {
            var stream = typeof(RaceTest).Assembly.GetManifestResourceStream(name);
            if (stream == null)
                throw new FileNotFoundException("Resource not found.", name);
            return stream;
        }

        [Test]
        public void TestSerialization()
        {
            var serializer = new XmlSerializer(typeof (Race));

            var reader = new XmlTextReader(GetResource("RulesTests.RulesTest1.xml"));

            // Read to document start
            while (reader.Read())
            {
                if (reader.NodeType == XmlNodeType.Element && "Race".Equals(reader.LocalName))
                {
                    var r = (Race)serializer.Deserialize(reader.ReadSubtree());
                    Console.WriteLine(r.Id);
                    Console.WriteLine(r.Name);

                    Console.WriteLine(r.MaleCharacteristics.Height);
                }
            }
        }

    }
}
