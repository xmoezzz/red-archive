using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace NekoPackExtract
{
    public interface iUnpackObject
    {
        bool SetFileName(string FileName);

        bool Unpack(object Args);
    }
}
