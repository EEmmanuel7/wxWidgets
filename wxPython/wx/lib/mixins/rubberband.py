
"""Renamer stub: provides a way to drop the wx prefix from wxPython objects."""

from wx import _rename
from wxPython.lib.mixins import rubberband
_rename(globals(), rubberband.__dict__, modulename='lib.mixins.rubberband')
del rubberband
del _rename
