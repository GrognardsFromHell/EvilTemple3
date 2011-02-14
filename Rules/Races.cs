using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Xml;
using System.Xml.Serialization;

namespace Rules
{
    public sealed class Races
    {
        private readonly Dictionary<string, Race> _races = new Dictionary<string, Race>();

        public Race this[string raceId]
        {
            get { return _races[raceId]; }
        }

        public IList<Race> GetSortedByName()
        {
            var result = new List<Race>(_races.Values);
            result.Sort(RaceNameComparer.Instance);
            return result;
        }

        /// <summary>
        /// Load additional races from the given input stream.
        /// </summary>
        /// <param name="xmlInputStream">A stream that contains race objects in serialized XML form.</param>
        public void Load(Stream xmlInputStream)
        {
            using (var xmlReader = new XmlTextReader(xmlInputStream))
            {
                var serializer = new XmlSerializer(typeof (Race));

                // Read to document start
                while (xmlReader.Read())
                {
                    if (!serializer.CanDeserialize(xmlReader)) continue;

                    var r = (Race) serializer.Deserialize(xmlReader.ReadSubtree());
                    _races.Add(r.Id, r);
                }
            }
        }
    }
}