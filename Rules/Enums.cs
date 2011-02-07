
using System;
using System.Reflection;

namespace Rules
{
    
    class LegacyLiteralAttribute : Attribute
    {
        public string Name { get; private set; }

        public LegacyLiteralAttribute(string name)
        {
            Name = name;
        }

        public static string GetLegacyName(MemberInfo memberInfo)
        {
            var attribute = (LegacyLiteralAttribute)GetCustomAttribute(memberInfo,
                    typeof(LegacyLiteralAttribute));

            return attribute == null ? null : attribute.Name;
        }
    }

    public enum Gender
    {
        [LegacyLiteral("female")]
        Female,
        [LegacyLiteral("male")]
        Male,
        [LegacyLiteral("other")]
        Other
    }

    public enum Alignment
    {
        [LegacyLiteral("lawful_good")]
        LawfulGood,
        [LegacyLiteral("neutral_good")]
        NeutralGood,
        [LegacyLiteral("chaotic_good")]
        ChaoticGood,
        [LegacyLiteral("lawful_neutral")]
        LawfulNeutral,
        [LegacyLiteral("true_neutral")]
        TrueNeutral,
        [LegacyLiteral("chaotic_neutral")]
        ChaoticNeutral,
        [LegacyLiteral("lawful_evil")]
        LawfulEvil,
        [LegacyLiteral("neutral_evil")]
        NeutralEvil,
        [LegacyLiteral("chaotic_evil")]
        ChaoticEvil
    }

    public static class Alignments
    {
        private static readonly string[] LegacyNames = EnumHelper.GetLegacyLiterals<Alignment>();

        public static string ToLegacyString(this Alignment value)
        {
            return LegacyNames[(int) value];
        }

        public static Alignment ParseLegacyString(string alignment)
        {
            for (var i = 0; i < LegacyNames.Length; ++i)
            {
                if (LegacyNames[i] == alignment)
                    return (Alignment)i;
            }

            throw new InvalidOperationException("Given legacy alignment doesn't exist: " + alignment);
        }
    }


    public static class Genders
    {
        private static readonly string[] LegacyNames = EnumHelper.GetLegacyLiterals<Gender>();

        public static string ToLegacyString(this Gender value)
        {
            return LegacyNames[(int)value];
        }

        public static Gender ParseLegacyString(string gender)
        {
            for (var i = 0; i < LegacyNames.Length; ++i)
            {
                if (LegacyNames[i] == gender)
                    return (Gender)i;
            }

            throw new InvalidOperationException("Given legacy gender doesn't exist: " + gender);
        }
    }


    static class EnumHelper
    {
        private static readonly string[] LegacyNames = GetLegacyLiterals<Alignment>();
        
        public static string[] GetLegacyLiterals<T>()
        {
            var enumType = typeof (T);

            if (!enumType.IsEnum)
                throw new InvalidOperationException("This method can only be used for enumeration types.");
            
            var enumNames = enumType.GetEnumNames();
            var legacyNames = new string[enumNames.Length];

            var i = 0;
            foreach (var enumName in enumNames)
            {
                var memberInfos = enumType.GetMember(enumName);

                if (memberInfos.Length != 1)
                    throw new InvalidOperationException("Enumeration literal " + enumName + " is not unique!");

                var memberInfo = memberInfos[0];

                var legacyName = LegacyLiteralAttribute.GetLegacyName(memberInfo);

                if (legacyName == null)
                    throw new InvalidOperationException("Enumeration literal " + enumName +
                                                        " has no legacy type attribute.");

                legacyNames[i++] = legacyName;
            }

            return legacyNames;
        }
    }

}