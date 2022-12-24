using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using SprinterPublishing;

namespace NekoPackExtract
{
    class UnpackMNGPack : iUnpackObject
    {
        public UnpackMNGPack(string FileName)
        {
            SetFileName(FileName);
        }

        private string m_FileName{ get; set;}
        public bool SetFileName(string FileName)
        {
            if (string.IsNullOrEmpty(FileName))
                throw new Exception("Filename cannot be null!");

            m_FileName = FileName;
            return true;
        }

        public bool Unpack(object Args)
        {
            try
            {
                var Loader = new MNG();
                Loader.Load(m_FileName);
                int Count = Loader.NumEmbeddedPNG;

                string outdir = Path.ChangeExtension(m_FileName, null);

                for (int i = 0; i < Count; i++)
                {
                    var bitmap = Loader.ToBitmap(i);
                    bitmap.Save(Path.Combine(outdir, string.Format("{0:0000}.png", i)), System.Drawing.Imaging.ImageFormat.Png);
                }
            }
            catch(Exception E)
            {
                return false;
            }
            return true;
        }
    }
}
