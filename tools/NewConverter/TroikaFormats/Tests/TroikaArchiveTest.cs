using System;
using System.Diagnostics;
using System.IO;
using System.Security.Cryptography;
using System.Text;
using NUnit.Framework;

namespace TroikaFormats.Tests
{
    public class TroikaArchiveTest
    {
        [Test]
        public void SmokeTest()
        {
            using (var archive = new TroikaArchive(@"C:\Games\Temple of Elemental Evil\toee1.dat")) {
                var stream = archive.OpenFile("art/BLACK.tga");

                Assert.NotNull(stream);

                var result = new byte[stream.Length];
                stream.Read(result, 0, result.Length);

                Assert.AreEqual(196626, result.Length);
                Assert.AreEqual("bf92d45fcfe71458fe5b3d7258d1ee03", GetMd5(result));
            }
        }

        private static string GetMd5(byte[] data)
        {
            var digest = MD5.Create();
            var hash = digest.ComputeHash(data);

            var result = new StringBuilder();

            for (var i = 0; i < hash.Length; i++)
                result.Append(hash[i].ToString("x2"));

            return result.ToString();
        }

        [Test]
        public void TestDefaultArchives()
        {
            var sw = new Stopwatch();
            sw.Start();
            var vfs = DefaultArchives.Create(@"D:\Temple of Elemental Evil");
            Console.WriteLine("Loaded all archives in {0}ms", sw.ElapsedMilliseconds);
            sw.Restart();

            var allFiles = vfs.ListAllFiles("*");

            Console.WriteLine("Listed all files in {0}ms", sw.ElapsedMilliseconds);
        }
    }
}