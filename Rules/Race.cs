using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Xml.Serialization;

namespace Rules
{

    public class Range<T> where T : struct 
    {
        [XmlAttribute("min")]
        public T Min { get; set; }

        [XmlAttribute("max")]
        public T Max { get; set; }

        internal Range()
        {
        }

        public Range(T min, T max)
        {
            var result = Comparer<T>.Default.Compare(min, max);
            if (result > 0)
            {
                var tmp = max;
                max = min;
                min = tmp;
            }

            Min = min;
            Max = max;
        }

        public override string ToString()
        {
            return string.Format("Min: {0}, Max: {1}", Min, Max);
        }
    }

    public class VisualCharacteristics
    {
        public string Prototype { get; set; }

        public Range<int> Height { get; set; }

        public Range<int> Weight { get; set; }
    }

    [XmlRoot(ElementName = "Race")]
    public class Race
    {
        [XmlAttribute("id")]
        public string Id { get;  set; }

        public bool Playable { get;  set; }

        public string Name { get;  set; }

        public string Description { get;  set; }

        public VisualCharacteristics MaleCharacteristics { get;  set; }

        public VisualCharacteristics FemaleCharacteristics { get;  set; }

        public uint LandSpeed { get;  set; }

        public uint StartingFeats { get;  set; }
    }
    
    

}
