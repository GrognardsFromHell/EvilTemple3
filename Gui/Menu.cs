using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Gui
{
    public class Menu
    {

        private dynamic _currentMenu;

        protected dynamic CurrentMenu
        {
            get { return _currentMenu; }
            set
            {
                if (_currentMenu != null)
                    _currentMenu.deleteLater();
                _currentMenu = value;
            }
        }

    }
}
