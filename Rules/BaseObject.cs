using System;
using System.ComponentModel;
using Rules.Prototypes;

namespace Rules
{
    public class BaseObject
    {
        private bool? _drawBehindWalls;
        private uint? _hitPoints;
        private string _individualName;
        private bool? _interactive;
        private uint? _rotation;
        private uint? _scale;
        private uint? _subdualDamageTaken;
        private uint? _damageTaken;
        private uint? _temporaryHitPoints;

        public event EventHandler<PropertyChangedEventArgs> OnPropertyChanged;

        public virtual BaseObjectPrototype Prototype { get; set; }

        public BaseObject()
        {
            Prototype = DefaultPrototype;
        }

        public uint Scale
        {
            get { return _scale ?? Prototype.Scale; }
            set { _scale = (value == Prototype.Scale) ? (uint?) null : value; }
        }


        public uint Rotation
        {
            get { return _rotation ?? Prototype.Rotation; }
            set { _rotation = (value == Prototype.Rotation) ? (uint?) null : value; }
        }

        public bool Interactive
        {
            get { return _interactive ?? Prototype.Interactive; }
            set { _interactive = (value == Prototype.Interactive) ? (bool?) null : value; }
        }

        public bool DrawBehindWalls
        {
            get { return _drawBehindWalls ?? Prototype.DrawBehindWalls; }
            set { _drawBehindWalls = (value == Prototype.DrawBehindWalls) ? (bool?) null : value; }
        }

        public uint HitPoints
        {
            get { return _hitPoints ?? Prototype.HitPoints; }
            set { _hitPoints = (value == Prototype.HitPoints) ? (uint?) null : value; }
        }

        public uint TemporaryHitPoints
        {
            get { return _temporaryHitPoints ?? Prototype.TemporaryHitPoints; }
            set { _temporaryHitPoints = (value == Prototype.TemporaryHitPoints) ? (uint?) null : value; }
        }

        public uint DamageTaken
        {
            get { return _damageTaken ?? Prototype.DamageTaken; }
            set { _damageTaken = (value == Prototype.DamageTaken) ? (uint?) null : value; }
        }

        public uint SubdualDamageTaken
        {
            get { return _subdualDamageTaken ?? Prototype.SubdualDamageTaken; }
            set { _subdualDamageTaken = (value == Prototype.SubdualDamageTaken) ? (uint?) null : value; }
        }

        public string IndividualName
        {
            get { return _individualName ?? Prototype.IndividualName; }
            set { _individualName = (value == Prototype.IndividualName) ? null : value; }
        }

        public string Name
        {
            get { return IndividualName; }
        }

        public virtual BaseObjectPrototype DefaultPrototype
        {
            get { return BaseObjectPrototype.Default; }
        }

        protected void InvokeOnPropertyChanged(string propertyName)
        {
            var e = new PropertyChangedEventArgs(propertyName);
            var handler = OnPropertyChanged;
            if (handler != null) handler(this, e);
        }
    }
}