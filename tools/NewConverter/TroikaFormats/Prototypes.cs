using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Text;

namespace TroikaFormats
{
    public class Prototypes
    {
        public IDictionary<uint, Prototype> PrototypeMap { get; private set; }

        public Prototypes(IVirtualFileSystem vfs)
        {
            PrototypeMap = new Dictionary<uint, Prototype>();

            using (var stream = vfs.OpenFile("rules/protos.tab"))
            {
                Load(stream);
            }
        }

        private void Load(Stream stream)
        {
            if (stream == null)
                throw new NullReferenceException("The prototypes file is missing.");

            using (var streamReader = new StreamReader(stream, Encoding.ASCII))
            {
                string line;
                while ((line = streamReader.ReadLine()) != null)
                {
                    if (line.Trim().Length == 0)
                        continue;

                    var parts = line.Split(new[] { '\t' }, StringSplitOptions.None);

                    // We ignore lines that have less than 100 entries
                    if (parts.Length < 100)
                    {
                        Trace.TraceWarning("Skipping line in prototype file because it has too few parts: {0}", line);
                        continue;
                    }

                    var id = uint.Parse(parts[0]);

                    var prototype = new Prototype(id);
                    prototype.Parse(parts);
                    PrototypeMap[id] = prototype;
                }
                
            }
        }

        public Prototype this[uint id]
        {
            get { return PrototypeMap[id]; }
        }

    }
}