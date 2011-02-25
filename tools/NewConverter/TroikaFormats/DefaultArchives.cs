using System;
using System.Diagnostics;
using System.IO;

namespace TroikaFormats
{
    public static class DefaultArchives
    {
        /// <summary>
        ///   The archives to load in order of precedence.
        /// </summary>
        private static readonly string[] Archives = new[]
                                                        {
                                                            @"Modules\ToEE.dat",
                                                            "ToEE4.dat",
                                                            "ToEE3.dat",
                                                            "ToEE2.dat",
                                                            "ToEE1.dat"
                                                        };

        public static IVirtualFileSystem Create(string basePath)
        {
            var vfs = new DelegatingVirtualFileSystem();

            foreach (var archive in Archives)
            {
                var fullPath = Path.Combine(basePath, archive);
                Trace.TraceInformation("Adding archive {0}.", fullPath);
                vfs.Add(new TroikaArchive(fullPath));
            }

            return vfs;
        }

        public static IVirtualFileSystem Create()
        {
            var defaultPaths = new DefaultPaths();

            return Create(defaultPaths.InstallationPath);
        }
    }
}