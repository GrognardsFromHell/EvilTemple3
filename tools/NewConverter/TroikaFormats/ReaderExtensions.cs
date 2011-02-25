using System.IO;
using System.Text;

namespace TroikaFormats
{
    static class ReaderExtensions
    {

        public static string ReadTroikaString(this BinaryReader reader)
        {
            var length = reader.ReadInt32();
            var bytes = reader.ReadBytes(length);
            var realLength = bytes.Length;
            // Ignore trailing null bytes
            while (realLength > 0 && bytes[realLength - 1] == 0)
                realLength--;
            var chars = new ASCIIEncoding().GetChars(bytes, 0, realLength);
            return new string(chars);
        }

    }
}
