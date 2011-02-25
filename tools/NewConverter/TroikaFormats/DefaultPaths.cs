using System;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Text;

namespace TroikaFormats
{
    public class DefaultPaths
    {
        private const string RegistryKey =
            @"SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{AD80F06B-0F21-4EEE-934D-BEF0D21E6383}";

        private const string RegistryValue = "InstallLocation";


        private const int KEY_WOW64_32KEY = 0x200;
        private const int KEY_READ = 0x20019;
        private const int REG_SZ = 1;
        private static readonly UIntPtr HKEY_LOCAL_MACHINE = new UIntPtr(0x80000002u);
        private static readonly UIntPtr RegistryRoot = HKEY_LOCAL_MACHINE;

        public DefaultPaths()
        {
            UIntPtr key;
            var result = RegOpenKeyEx(RegistryRoot, RegistryKey, 0, KEY_READ | KEY_WOW64_32KEY, out key);

            if (result != 0)
                throw new InvalidOperationException("Unable to open registry key for ToEE installation directory: " +
                                                    result);

            uint dataType;
            var dataLength = 0;

            result = RegQueryValueEx(key, RegistryValue, 0, out dataType, null, ref dataLength);

            if (result != 0)
                throw new InvalidOperationException("Unable to query registry key for ToEE installation directory: " +
                                                    result);

            if (dataType != REG_SZ)
                throw new InvalidOperationException("InstallPath has wrong data type: " + dataType);

            // Allocate a large enough buffer
            var sb = new StringBuilder(dataLength);
            result = RegQueryValueEx(key, RegistryValue, 0, out dataType,
                                     sb, ref dataLength);

            if (result != 0)
                throw new InvalidOperationException("Unable to read registry key for ToEE installation directory: " +
                                                    result);

            result = RegCloseKey(key);

            if (result != 0)
                Trace.TraceWarning("Unable to close registry key for ToEE installation directory: {0}", result);

            InstallationPath = sb.ToString();
        }

        public string InstallationPath { get; private set; }


        [DllImport("advapi32", CharSet = CharSet.Auto)]
        public static extern int RegOpenKeyEx(
            UIntPtr hKey,
            string subKey,
            int ulOptions,
            int samDesired,
            out UIntPtr hkResult);


        // This signature will not get an entire REG_BINARY value. It will stop at the first null byte.
        [DllImport("advapi32", CharSet = CharSet.Auto, SetLastError = true)]
        private static extern int RegQueryValueEx(
            UIntPtr hKey,
            string lpValueName,
            int lpReserved,
            out uint lpType,
            StringBuilder lpData,
            ref int lpcbData);


        [DllImport("advapi32", SetLastError = true)]
        public static extern int RegCloseKey(
            UIntPtr hKey);
    }
}