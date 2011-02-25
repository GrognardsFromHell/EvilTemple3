using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Text;

namespace TroikaFormats
{
    public class MessageFileSource
    {
        private const char LeftBracket = '{';
        private const char RightBracket = '}';
        private const char Newline = '\n';
        private const char Slash = '/';

        private readonly IVirtualFileSystem _vfs;

        public MessageFileSource(IVirtualFileSystem vfs)
        {
            _vfs = vfs;
        }

        public IDictionary<uint, string> Load(string path)
        {
            using (var stream = _vfs.OpenFile(path))
            {
                return Load(stream);
            }
        }

        public static IDictionary<uint, string> Load(Stream stream)
        {
            var result = new Dictionary<uint, string>();

            var state = ParserState.WaitingForKey;
            var previous = '\0';
            var key = new StringBuilder();
            var value = new StringBuilder();

            var reader = new StreamReader(stream, Encoding.Default);

            int v;
            while ((v = reader.Read()) != -1)
            {
                var c = (char) v;

                switch (state)
                {
                    case ParserState.WaitingForKey:
                        if (c == LeftBracket)
                        {
                            state = ParserState.Key;
                        }
                        else if (c == Slash && previous == Slash)
                        {
                            state = ParserState.WaitingForKeyComment;
                        }
                        break;
                    case ParserState.WaitingForKeyComment:
                        if (c == Newline)
                        {
                            state = ParserState.WaitingForKey;
                        }
                        break;
                    case ParserState.Key:
                        if (c == RightBracket)
                        {
                            state = ParserState.WaitingForValue;
                        }
                        else
                        {
                            key.Append(c);
                        }
                        break;
                    case ParserState.WaitingForValue:
                        if (c == LeftBracket)
                        {
                            state = ParserState.Value;
                        }
                        else if (c == Slash && previous == Slash)
                        {
                            state = ParserState.WaitingForValueComment;
                        }
                        break;
                    case ParserState.WaitingForValueComment:
                        if (c == Newline)
                        {
                            state = ParserState.WaitingForValue;
                        }

                        break;
                    case ParserState.Value:
                        if (c == RightBracket)
                        {
                            // Try parsing the key
                            uint keyNumber;
                            if (uint.TryParse(key.ToString(), out keyNumber))
                                result[keyNumber] = value.ToString();
                            else
                                Trace.TraceWarning("Invalid key for message: {0}", key);

                            value.Clear();
                            key.Clear();
                            state = ParserState.WaitingForKey;
                        }
                        else
                        {
                            value.Append(c);
                        }
                        break;
                }

                previous = c;
            }

            return result;
        }

        #region Nested type: ParserState

        private enum ParserState
        {
            WaitingForKey, // Everything until {
            WaitingForKeyComment, // Everything after // until newline, while waiting for key
            Key,
            WaitingForValue,
            WaitingForValueComment, // Everything after // until newline, while waiting for value
            Value
        } ;

        #endregion
    }
}