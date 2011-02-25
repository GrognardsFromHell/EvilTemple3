using System;
using System.Dynamic;
using System.Linq;
using Game;
using Rules;
using EvilTemple.Runtime;

namespace Gui
{
    public class CreateCharacter : Menu
    {
        private const float DefaultHeightFactor = 0.5f;

        private readonly IGameView _gameView;

        private readonly Races _races;

        private readonly IModels _models;

        public CreateCharacter(IGameView gameView, IModels models, Races races)
        {
            _gameView = gameView;
            _races = races;
            _models = models;
        }

        public event Action OnCancel;

        private delegate void StatsDistributedAction(int str, int dex, int con, int intl, int wis, int cha);

        private PlayerCharacter currentCharacter = new PlayerCharacter();

        private ModelInstance _modelInstance;

        public void Show()
        {
            CurrentMenu = _gameView.AddInterfaceItem("interface/CreateCharacter.qml");

            CurrentMenu.cancel += (Action)Cancel;

            CurrentMenu.activeStageRequested += (Action<int>)ActiveStageRequested;
            CurrentMenu.statsDistributed += (StatsDistributedAction)StatsDistributed;
            CurrentMenu.raceChosen += (Action<string>)RaceChosen;
            CurrentMenu.genderChosen += (Action<string>)GenderChosen;
            CurrentMenu.heightChosen += (Action<float>)HeightChosen;
            CurrentMenu.classChosen += (Action<string>)ClassChosen;
            CurrentMenu.alignmentChosen += (Action<string>)AlignmentChosen;
            CurrentMenu.deityChosen += (Action<string>)DeityChosen;
            
            // Start with the stats page
            CurrentMenu.overallStage = Stage.Stats;
            CurrentMenu.activeStage = Stage.Stats;

            _modelInstance = new ModelInstance(CurrentMenu.getModelViewer().modelInstance,
                currentCharacter);
        }

        private void LoadRaces()
        {
            CurrentMenu.races = _races.GetSortedByName().Select(r => r.ToPropertyMap());
        }

        private void StatsDistributed(int str, int dex, int con, int intl, int wis, int cha)
        {
            currentCharacter.BaseStrength = str;
            currentCharacter.BaseDexterity = dex;
            currentCharacter.BaseConstitution = con;
            currentCharacter.BaseIntelligence = intl;
            currentCharacter.BaseWisdom = wis;
            currentCharacter.BaseCharisma = cha;

            // Every stat must be > 0 for the entire distribution to be valid.
            var valid = str > 0 && dex > 0 && con > 0 && intl > 0 && wis > 0 && cha > 0;

            CurrentMenu.overallStage = valid ? Stage.Race : Stage.Stats;

            UpdateCharacterSheet();
        }
        
        private void RaceChosen(string raceId)
        {
            currentCharacter.Race = _races[raceId];

            CurrentMenu.overallStage = Stage.Gender;
            
            UpdateModelViewer();
            if (currentCharacter.Gender != Gender.Other)
                HeightChosen(DefaultHeightFactor); // Default height
        }

        private void GenderChosen(string genderId)
        {
            currentCharacter.Gender = Genders.ParseLegacyString(genderId);

            UpdateModelViewer(); // Do it once to get the prototype connected
            HeightChosen(DefaultHeightFactor); // Default height
            UpdateModelViewer();
        }

        private void HeightChosen(float heightFactor)
        {
            var race = currentCharacter.Race;

            var visuals = (currentCharacter.Gender == Gender.Female) ? race.FemaleCharacteristics : race.MaleCharacteristics;
            
            currentCharacter.Height = visuals.Height.Interpolate(heightFactor); // This will also affect rendering-scale.
            currentCharacter.Weight = visuals.Weight.Interpolate(heightFactor);

            // TODO: This formula is most likely wrong.
            /*
             Attempt at fixing this:
             Assume that the 0cm is scale 0 and the medium height between min/max (0.5f) is scale 1
             So for a height-range of 100cm-200cm, with a default of 150, the scale-range would be
             0.66 + (1.33 - 0.66) * heightFactor, where 0.66 = 100/150 and 1.33 = 200/150
             */
            var midHeight = visuals.Height.Interpolate(DefaultHeightFactor);
            var minFac = visuals.Height.Min / midHeight;
            var maxFac = visuals.Height.Max / midHeight;

            var adjustedHeightFac = minFac + (maxFac - minFac) * heightFactor;

            var modelScale = currentCharacter.Scale / 100.0f * adjustedHeightFac;
            CurrentMenu.getModelViewer().modelScale = modelScale;
            Console.WriteLine("Set model-viewer scale to " + modelScale);

            // Height changing never changes the state unless it is to advance it
            if (CurrentMenu.overallStage < Stage.Hair)
                CurrentMenu.overallStage = Stage.Class;

            UpdateCharacterSheet();
        }
        
        private void DeityChosen(string obj)
        {
            throw new NotImplementedException();
        }

        private void AlignmentChosen(string obj)
        {
            throw new NotImplementedException();
        }

        private void ClassChosen(string classId)
        {
            throw new NotImplementedException();
        }
        
        private void ActiveStageRequested(int stage)
        {
            switch (stage)
            {
                case Stage.Race:
                    LoadRaces();
                    break;
                case Stage.Height:
                    SetHeightSpan();
                    break;
            }

            CurrentMenu.activeStage = stage;
        }

        private void SetHeightSpan()
        {
            var visuals = currentCharacter.Race.MaleCharacteristics;

            if (currentCharacter.Gender == Gender.Female)
                visuals = currentCharacter.Race.FemaleCharacteristics;

            CurrentMenu.minHeight = visuals.Height.Min;
            CurrentMenu.maxHeight = visuals.Height.Max;
        }

        private void UpdateModelViewer()
        {
            if (currentCharacter.Race == null || currentCharacter.Gender == Gender.Other)
                return;

            var characteristics = currentCharacter.Race.GetCharacteristics(currentCharacter.Gender);
            var prototypeId = characteristics.Prototype;

            Console.WriteLine("Setting prototype of new character to " + prototypeId);

            // currentCharacter.prototype = prototypeId;
            // Prototypes.reconnect(currentCharacter);

            var modelViewer = CurrentMenu.getModelViewer();

            modelViewer.modelRotation = -120;

            currentCharacter.Model = "meshes/PCs/PC_Human_Male/PC_Human_Male.model";

            var modelInstance = modelViewer.modelInstance;
            modelInstance.model = _models.Load(currentCharacter.Model);
            
            Console.WriteLine("Loading model: " + currentCharacter.Model);

            var materials = modelViewer.materials;
            var material = materials.load("meshes/PCs/PC_Human_Male/CHEST.xml");
            modelInstance.overrideMaterial("CHEST", material);
            // Equipment.addRenderEquipment(currentCharacter, model, modelViewer.materials, null);
        }

        private void UpdateCharacterSheet()
        {
            dynamic charSheet = new ExpandoObject();
            charSheet.strength = currentCharacter.Strength;
            charSheet.dexterity = currentCharacter.Dexterity;
            charSheet.constitution = currentCharacter.Constitution;
            charSheet.intelligence = currentCharacter.Intelligence;
            charSheet.wisdom = currentCharacter.Wisdom;
            charSheet.charisma = currentCharacter.Charisma;

            charSheet.height = currentCharacter.Height;
            charSheet.weight = currentCharacter.Weight;
        }

        private void Cancel()
        {
            CurrentMenu = null;
            var handler = OnCancel;
            if (handler != null) handler();
        }

        private static class Stage
        {
            public const int Stats = 0;
            public const int Race = 1;
            public const int Gender = 2;
            public const int Height = 3;
            public const int Hair = 4;
            public const int Class = 5;
            public const int Alignment = 6;
            public const int Deity = 7;
            public const int Features = 8;
            public const int Feats = 9;
            public const int Skills = 10;
            public const int Spells = 11;
            public const int Portrait = 12;
            public const int VoiceAndName = 13;
            public const int Finished = 14;
        }
    }
}
