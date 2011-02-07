using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using NUnit.Framework;
using Rules;

namespace RulesTests
{
    class AlignmentTest
    {
        [Test]
        public void TestFromLegacyString()
        {
            Assert.AreEqual(Alignment.TrueNeutral, Alignments.ParseLegacyString("true_neutral"));
            Assert.AreEqual(Alignment.ChaoticEvil, Alignments.ParseLegacyString("chaotic_evil"));

            Assert.Throws<InvalidOperationException>(() => Alignments.ParseLegacyString("NO_SUCH_ALIGNMENT"));
        }

        [Test]
        public void TestToLegacyString()
        {
            Assert.AreEqual("true_neutral", Alignment.TrueNeutral.ToLegacyString());
            Assert.AreEqual("chaotic_evil", Alignment.ChaoticEvil.ToLegacyString());
        }
    }
}
