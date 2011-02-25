using System;
using System.Collections.Generic;
using System.IO;
using System.IO.Compression;
using System.Linq;
using System.Text;

namespace TroikaFormats
{
    /// <summary>
    ///   This interface exposes methods to access the virtual Troika file system.
    /// </summary>
    public interface IVirtualFileSystem
    {
        Stream OpenFile(string path);
        bool Exists(string path);
        IEnumerable<string> ListFiles(string path, string filter = "*");
        IEnumerable<string> ListAllFiles(string filenameFilter);
    }

    /// <summary>
    ///   This virtual file system delegates to an ordered list of other virtual
    ///   file systems.
    /// </summary>
    public class DelegatingVirtualFileSystem : IVirtualFileSystem
    {
        private readonly IList<IVirtualFileSystem> _virtualFileSystems = new List<IVirtualFileSystem>();

        #region IVirtualFileSystem Members

        public Stream OpenFile(string path)
        {
            return _virtualFileSystems.Select(vfs => vfs.OpenFile(path)).FirstOrDefault(result => result != null);
        }

        public bool Exists(string path)
        {
            return _virtualFileSystems.Any(vfs => vfs.Exists(path));
        }

        public IEnumerable<string> ListFiles(string path, string filter)
        {
            var resultSet = new HashSet<string>();

            foreach (var vfs in _virtualFileSystems)
            {
                foreach (var result in vfs.ListFiles(path, filter))
                    resultSet.Add(result);
            }

            return resultSet;
        }

        public IEnumerable<string> ListAllFiles(string filenameFilter)
        {
            var resultSet = new HashSet<string>();

            foreach (var vfs in _virtualFileSystems)
            {
                foreach (var result in vfs.ListAllFiles(filenameFilter))
                    resultSet.Add(result);
            }

            return resultSet;
        }

        #endregion

        public void Add(IVirtualFileSystem vfs)
        {
            _virtualFileSystems.Add(vfs);
        }

        public void Remove(IVirtualFileSystem vfs)
        {
            _virtualFileSystems.Remove(vfs);
        }
    }

    internal class TroikaArchiveEntry
    {
        #region Type enum

        public enum EntryType
        {
            File = 1,
            CompressedFile = 2,
            Directory = 1024
        } ;

        #endregion

        public const char Separator = '/';
        public int CompressedSize;
        public uint DataStart;
        public string Filename;
        public TroikaArchiveEntry FirstChild;
        public TroikaArchiveEntry NextSibling;
        public TroikaArchiveEntry Parent;
        public EntryType Type;
        public uint UncompressedSize;

        public string Path
        {
            get
            {
                var result = new StringBuilder(Filename);

                var entry = Parent;

                while (entry != null)
                {
                    result.Insert(0, Separator);
                    result.Insert(0, entry.Filename);
                    entry = entry.Parent;
                }

                if (Directory)
                    result.Append(Separator);

                return result.ToString();
            }
        }

        public IList<TroikaArchiveEntry> Children
        {
            get
            {
                IList<TroikaArchiveEntry> result = new List<TroikaArchiveEntry>();

                var entry = FirstChild;

                while (entry != null)
                {
                    result.Add(entry);
                    entry = entry.NextSibling;
                }
                return result;
            }
        }

        public bool Directory
        {
            get { return Type == EntryType.Directory; }
        }

        public bool Compressed
        {
            get { return Type == EntryType.CompressedFile; }
        }

        public bool File
        {
            get { return (Type == EntryType.File || Type == EntryType.CompressedFile); }
        }
    }

    public class TroikaArchive : IVirtualFileSystem, IDisposable
    {
        private readonly TroikaArchiveEntry[] _entries;
        private readonly IList<TroikaArchiveEntry> _rootEntries;

        private readonly FileStream _stream;

        public TroikaArchive(string filename)
        {
            Filename = filename;

            _stream = new FileStream(filename, FileMode.Open, FileAccess.Read, FileShare.Read);

            _rootEntries = new List<TroikaArchiveEntry>();
            _entries = ReadEntries(_stream, _rootEntries);
        }

        public string Filename { get; private set; }

        #region IDisposable Members

        public void Dispose()
        {
            _stream.Dispose();
        }

        #endregion

        #region IVirtualFileSystem Members

        public Stream OpenFile(string path)
        {
            var entry = FindEntry(path);

            if (entry == null || !entry.File)
                return null;

            lock (_stream)
            {
                _stream.Seek(entry.DataStart, SeekOrigin.Begin);

                if (entry.Compressed)
                {
                    var buffer = new byte[entry.CompressedSize];
                    _stream.Read(buffer, 0, entry.CompressedSize);

                    // Uncompress in memory ahead of time, since we know the uncompressed size here

                    // Skip the 2 byte Zlib header
                    var inputStream = new MemoryStream(buffer, 2, buffer.Length - 2, false);
                    var deflateStream = new DeflateStream(inputStream, CompressionMode.Decompress);

                    buffer = new byte[entry.UncompressedSize];
                    deflateStream.Read(buffer, 0, buffer.Length);

                    return new MemoryStream(buffer, false);
                }
                else
                {
                    var buffer = new byte[entry.CompressedSize];
                    _stream.Read(buffer, 0, entry.CompressedSize);
                    return new MemoryStream(buffer, false);
                }
            }
        }

        public bool Exists(string path)
        {
            return FindEntry(path) != null;
        }

        public IEnumerable<string> ListFiles(string path, string filter)
        {
            IList<string> result = new List<string>();
            var entry = FindEntry(path);

            if (entry == null)
                return result;

            var basepath = entry.Path;

            var filterParts = filter.Split(new[] {'*'}, StringSplitOptions.None);

            if (filterParts.Length > 2)
                throw new ArgumentException("listFiles may only have one wildcard per filter: " + filter);

            foreach (var child in entry.Children)
            {
                if (filterParts.Length == 1 &&
                    !child.Filename.Equals(filter, StringComparison.InvariantCultureIgnoreCase))
                {
                    result.Add(basepath + child.Filename);
                }
                else if (child.Filename.StartsWith(filterParts[0], StringComparison.InvariantCultureIgnoreCase) &&
                         child.Filename.EndsWith(filterParts[1], StringComparison.InvariantCultureIgnoreCase))
                {
                    result.Add(basepath + child.Filename);
                }
            }

            return result;
        }

        public IEnumerable<string> ListAllFiles(string filenameFilter)
        {
            var filterParts = filenameFilter.Split(new[] {'*'}, StringSplitOptions.None);

            if (filterParts.Length > 2)
                throw new ArgumentException("listFiles may only have one wildcard per filter: " + filenameFilter);

            IList<string> result = new List<string>();

            // We're looking for an exact match in case the filter didn't have a wildcard
            if (filterParts.Length == 1)
            {
                var entry =_entries.FirstOrDefault(
                        e => e.File 
                            && e.Filename.Equals(filenameFilter, StringComparison.InvariantCultureIgnoreCase));
                if (entry != null)
                    result.Add(entry.Filename);
                return result;
            }

            return from entry in _entries
                   where entry.File
                         && entry.Filename.StartsWith(filterParts[0], StringComparison.InvariantCultureIgnoreCase) 
                         && entry.Filename.EndsWith(filterParts[1], StringComparison.InvariantCultureIgnoreCase)
                   select entry.Path;
        }

        #endregion

        private static TroikaArchiveEntry[] ReadEntries(Stream stream, IList<TroikaArchiveEntry> rootEntries)
        {
            var reader = new BinaryReader(stream, Encoding.ASCII);

            stream.Seek(-4, SeekOrigin.End);

            // Length of the archive virtual file-system in byte
            var fsLength = reader.ReadUInt32();

            stream.Seek(stream.Length - fsLength, SeekOrigin.Begin);

            // Number of entries in archive file-system
            var entryCount = reader.ReadUInt32();

            var entries = new TroikaArchiveEntry[entryCount];

            for (var i = 0; i < entryCount; ++i)
                entries[i] = new TroikaArchiveEntry();

            for (var i = 0; i < entryCount; ++i)
            {
                var entry = ReadEntry(i, entries, reader);

                entries[i] = entry;

                // Record the entries in the root directory as a starting point
                if (entry.Parent == null)
                    rootEntries.Add(entry);
            }

            return entries;
        }

        private static TroikaArchiveEntry ReadEntry(int index, TroikaArchiveEntry[] entries, BinaryReader reader)
        {
            var entry = entries[index];

            entry.Filename = reader.ReadTroikaString();
            reader.ReadInt32(); // Unknown value
            entry.Type = (TroikaArchiveEntry.EntryType) reader.ReadInt32();
            entry.UncompressedSize = reader.ReadUInt32();
            entry.CompressedSize = reader.ReadInt32();
            entry.DataStart = reader.ReadUInt32();
            var parentDirId = reader.ReadInt32();
            var firstChildId = reader.ReadInt32();
            var nextSiblingId = reader.ReadInt32();

            if (parentDirId >= index)
                throw new InvalidDataException("Referencing an object after the current one as parent.");

            if (parentDirId != -1)
                entry.Parent = entries[parentDirId];
            if (firstChildId != -1)
                entry.FirstChild = entries[firstChildId];
            if (nextSiblingId != -1)
                entry.NextSibling = entries[nextSiblingId];

            return entry;
        }

        private TroikaArchiveEntry FindEntry(string filename)
        {
            var parts = filename.Split(new[] {'/', '\\'}, StringSplitOptions.RemoveEmptyEntries);

            TroikaArchiveEntry result = null;
            var entries = _rootEntries;

            foreach (var part in parts)
            {
                var localPart = part; // To make ReSharper happy
                result = entries.FirstOrDefault(e => e.Filename.Equals(localPart, StringComparison.InvariantCultureIgnoreCase));

                if (result == null)
                    return null;

                entries = result.Children;
            }

            return result;
        }
    }
}