using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace NekoPackExtract
{
    public interface iPackObject
    {
        bool SetOriginalFile(string FileName);
        bool SetPackDirectory(string PathName);
        bool SetOutputFile(string FileName);

        bool Pack(object Args);
    }
}
