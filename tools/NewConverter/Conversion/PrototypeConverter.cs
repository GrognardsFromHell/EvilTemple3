using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Xml;
using System.Xml.Serialization;
using Rules.Prototypes;
using TroikaFormats;
using Prototypes = TroikaFormats.Prototypes;

namespace Conversion
{
    public class PrototypeConverter
    {

        private readonly IVirtualFileSystem _vfs;

        private readonly IDictionary<string, StringBuilder> _files;

        private XmlSerializer _serializer;

        public PrototypeConverter(IVirtualFileSystem vfs)
        {
            _vfs = vfs;
            _files = new Dictionary<string, StringBuilder>();
        }

        public void Run()
        {
            var sw = new Stopwatch();
            sw.Start();
            _serializer = new XmlSerializer(typeof (BaseObjectPrototype));
            Trace.TraceInformation("Created XML Serializer for Prototypes in {0}", sw.Elapsed);

            var prototypes = new Prototypes(_vfs);

            var keys = prototypes.PrototypeMap.Keys.OrderBy(x => x);

            var xmlWriters = new Dictionary<ObjectType, XmlWriter>();

            foreach (var key in keys)
            {
                var prototype = prototypes[key];

                var writer = GetWriter(xmlWriters, prototype.Type);
                
                WritePrototype(prototype, writer);
            }

            CloseWriters(xmlWriters);

            // Dump files?
            foreach (var entry in _files)
            {
                Console.WriteLine();
                Console.WriteLine("FILENAME: " + entry.Key);
                Console.WriteLine();
                Console.WriteLine(entry.Value);
            }
        }

        private void WritePrototype(Prototype troikaPrototype, XmlWriter writer)
        {
            var prototype = CreatePrototype(troikaPrototype.Type);
            prototype.Id = troikaPrototype.Id.ToString();

            _serializer.Serialize(writer, prototype);
        }

        private BaseObjectPrototype CreatePrototype(ObjectType type)
        {
            switch (type)
            {
                case ObjectType.Portal:
                    return new ItemPrototype();
                case ObjectType.Container:
                    return new ContainerPrototype();
                case ObjectType.Scenery:
                    return new SceneryPrototype();
                case ObjectType.Projectile:
                    return new ItemPrototype();
                case ObjectType.Weapon:
                    return new ItemPrototype();
                case ObjectType.Ammo:
                    return new ItemPrototype();
                case ObjectType.Armor:
                    return new ItemPrototype();
                case ObjectType.Money:
                    return new ItemPrototype();
                case ObjectType.Food:
                    return new ItemPrototype();
                case ObjectType.Scroll:
                    return new ItemPrototype();
                case ObjectType.Key:
                    return new ItemPrototype();
                case ObjectType.Written:
                    return new ItemPrototype();
                case ObjectType.Generic:
                    return new ItemPrototype();
                case ObjectType.PlayerCharacter:
                    return new PlayerCharacterPrototype();
                case ObjectType.NonPlayerCharacter:
                    return new NonPlayerCharacterPrototype();
                case ObjectType.Trap:
                    return new ItemPrototype();
                case ObjectType.Bag:
                    return new ItemPrototype();
                default:
                    throw new ArgumentOutOfRangeException();
            }
        }

        private void WriteProperties(Prototype prototype, XmlWriter writer)
        {
            throw new NotImplementedException();
        }

        private void WriteAdditionalProperties(AdditionalProperties additionalProperties, XmlWriter writer)
        {
            throw new NotImplementedException();
        }

        private void CloseWriters(Dictionary<ObjectType, XmlWriter> xmlWriters)
        {
            foreach (var entry in xmlWriters)
            {
                entry.Value.WriteEndElement();
                entry.Value.Close();
            }
        }

        private XmlWriter GetWriter(IDictionary<ObjectType, XmlWriter> xmlWriters, ObjectType type)
        {
            XmlWriter writer;
            if (!xmlWriters.TryGetValue(type, out writer))
            {
                writer = CreateWriter(type);
                xmlWriters[type] = writer;
            }
            return writer;
        }

        private XmlWriter CreateWriter(ObjectType type)
        {
            var filename = type + ".xml";
            var content = new StringBuilder();

            _files[filename] = content;

            var settings = new XmlWriterSettings
                               {
                                   CloseOutput = false,
                                   Indent = true
                               };

            var writer = XmlWriter.Create(content, settings);

            writer.WriteStartDocument();
            writer.WriteStartElement("Prototypes");

            return writer;
        }
    }
}
