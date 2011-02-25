using System.Collections.Generic;
using System.Xml;
using System.Xml.Serialization;

namespace Rules.Prototypes
{
    public class Prototypes
    {
        private readonly IDictionary<string, BaseObjectPrototype> _prototypes =
            new Dictionary<string, BaseObjectPrototype>();

        private readonly XmlSerializer _prototypeSerializer = new XmlSerializer(typeof(BaseObject));

        public BaseObjectPrototype this[string i]
        {
            get { return _prototypes[i]; }
        }

        public void LoadFrom(XmlReader xmlReader)
        {
            var prototype = (BaseObjectPrototype)_prototypeSerializer.Deserialize(xmlReader);
            _prototypes[prototype.Id] = prototype;
        }

    }

    /************************************************************************/
    /* TODO                                                                 */
    /************************************************************************/

    public class ItemPrototype : BaseObjectPrototype
    {
    }

    public class ContainerPrototype : BaseObjectPrototype
    {
    }

    public class PortalPrototype : BaseObjectPrototype
    {
    }

    public class MapChangerPrototype : BaseObjectPrototype
    {
    }

    public class NonPlayerCharacterPrototype : CritterPrototype
    {
    }

    public class PlayerCharacterPrototype : CritterPrototype
    {
    }

    public class SceneryPrototype : ItemPrototype
    {
    }

}
