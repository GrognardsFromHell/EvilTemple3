using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace TroikaFormats
{
    internal static class Util
    {

        
        /**
        Converts radians to degree.
        */
        public static float rad2deg(float rad)
        {
            return (float) (rad*180.0f/Math.PI);
        }


    }
}
