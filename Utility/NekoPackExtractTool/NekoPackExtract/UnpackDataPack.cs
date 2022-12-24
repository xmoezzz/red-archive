using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;

namespace NekoPackExtract
{

    public struct NekoHeader
    {
        public byte[]  Signature; //8
        public uint    Hash;
        public uint    Padding;
    };

    public struct ChunkHeader 
    {
        public uint Seed;
        public uint Length;
    };

    public struct EntryHeader
    {
        public uint NameHash;
        public uint Count;
    };

    public struct FileEntry
    {
        public uint NameHash;
        public uint Length;
    };

    class UnpackDataPack : iUnpackObject
    {
        public UnpackDataPack(string FileName)
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


        public uint ReadUInt32(Stream stream)
        {
            byte[] buff = new byte[4];
            stream.Read(buff, 0, buff.Count());
            unsafe 
            {
                fixed (byte* BuffAddr = &buff[0])
                {
                    return *(uint*)(BuffAddr); 
                }
            }
        }


        public unsafe uint ReadUInt32(byte* buff)
        {
            return *(uint*)(buff); 
        }

        private unsafe void Decode(byte[] buffer, uint length, uint seed)
        {
            fixed(byte* buff = &buffer[0])
            {
                uint t1 = seed ^ (seed + 0x5D588B65);
                uint t2 = t1   ^ (seed - 0x359D3E2A);
                uint t3 = t2   ^ (t1   - 0x70E44324);
                uint t4 = t3   ^ (t2   + 0x6C078965);

                ulong key = ((ulong)t4 << 32) | t3;
                ulong* p   = (ulong*) buff;
                ulong* end = p + (length / 8);

                while (p < end)
                {
                    *p ^= key;

                    ushort* ka = (ushort*) &key;
                    ushort* pa = (ushort*) p;
                    ka[0] += pa[0];
                    ka[1] += pa[1];
                    ka[2] += pa[2];
                    ka[3] += pa[3];

                    p++;
                }
            }
        }

        private void ReadBuffer(FileStream stream, ref byte[] buff, ref uint len) 
        {
            ChunkHeader Header = new ChunkHeader();
            Header.Seed   = ReadUInt32(stream);
            Header.Length = ReadUInt32(stream);

            len                      = Header.Length;
            uint PaddingSize         = (len + (uint)7) & ~(uint)7;
            buff                     = new byte[PaddingSize];

            stream.Read(buff, 0, (int)len);

            if (Header.Seed != 0)
            {
                Decode(buff,PaddingSize, Header.Seed);
            }
        }


        public bool CompareArray(byte[] src, byte[] dest, uint size)
        {
            bool Result = true;
            for(uint i = 0; i < size; i++)
            {
                if(src[i] != dest[i])
                {
                    Result = false;
                    break;
                }
            }
            return Result;
        }


        private byte[] OggsSign = new byte[4] { 0x4f, 0x67, 0x67, 0x53 };
        private byte[] PngSign = new byte[4] { 0x89, 0x50, 0x4e, 0x47 };
        private byte[] MngSign = new byte[4] { 0x8a, 0x4d, 0x4e, 0x47 };
        private byte[] PsdSign = new byte[4] { 0x38, 0x42, 0x50, 0x53 };


        private bool IsTextFile(byte[] Buffer, uint Length)
        {
            for(var i = 0; i < Length; i++)
            {
                if (Buffer[i] == 0)
                    return false;
            }
            return true;
        }

        public bool Unpack(object Args)
        {
            var CurrentPath = Path.ChangeExtension(m_FileName, null);
            Directory.CreateDirectory(CurrentPath);

            using(var NtFile = new FileStream(m_FileName, FileMode.Open))
            {
                NekoHeader Header = new NekoHeader();
                Header.Signature = new byte[8];
                NtFile.Read(Header.Signature, 0, Header.Signature.Count());
                Header.Hash = ReadUInt32(NtFile);
                Header.Padding = ReadUInt32(NtFile);

                unsafe
                {
                    uint ChunkSize = 0;
                    byte[] ChunkBuffer = null;

                    ReadBuffer(NtFile, ref ChunkBuffer, ref ChunkSize);

                    fixed(byte* buff = &ChunkBuffer[0])
                    {

                        byte* p = buff;
                        byte* end = buff + ChunkSize;

                        for (ulong i = 0; p < end; i++) 
                        {
                            EntryHeader Dir = new EntryHeader();
                            Dir.NameHash = ReadUInt32(p);
                            p += 4;
                            Dir.Count = ReadUInt32(p);
                            p += 4;

                            for (ulong j = 0; j < Dir.Count; j++) 
                            {
                                FileEntry Item = new FileEntry();
                                Item.NameHash = ReadUInt32(p);
                                p += 4;
                                Item.Length = ReadUInt32(p);
                                p += 4;

                                uint   FileSize  = 0;
                                byte[] FileBuffer = null;
                                ReadBuffer(NtFile, ref FileBuffer, ref FileSize);

                                bool IsMng = false;
                                //var FileName = string.Format("{0:000000}_{1:000000}", i, j);
                                var FileName = string.Format("{0:d}_{1:X00000000}", i, Item.NameHash);

                                if (CompareArray(OggsSign, FileBuffer, (uint)OggsSign.Count()))
                                    FileName += ".ogg";
                                else if (CompareArray(PngSign, FileBuffer, (uint)PngSign.Count()))
                                    FileName += ".png";
                                else if (CompareArray(PsdSign, FileBuffer, (uint)PsdSign.Count()))
                                    FileName += ".psd";
                                else if (CompareArray(MngSign, FileBuffer, (uint)MngSign.Count()))
                                {
                                    FileName += ".mng";
                                    IsMng = true;
                                }
                                else if (IsTextFile(FileBuffer, FileSize))
                                    FileName += ".txt";

                                var DirectoryName = Path.ChangeExtension(m_FileName, null);
                                if (!Directory.Exists(DirectoryName))
                                    Directory.CreateDirectory(DirectoryName);


                                var FullPathName = Path.Combine(DirectoryName, FileName);
                            
                                using(var Writer = new FileStream(FullPathName, FileMode.Create))
                                {
                                    Writer.Write(FileBuffer, 0, (int)FileSize);
                                    Writer.Close();
                                }

                                if (IsMng)
                                {
                                    var Loader = new SprinterPublishing.MNG();
                                    Loader.Load(FullPathName);
                                    int Count = Loader.NumEmbeddedPNG;

                                    string outdir = Path.ChangeExtension(FullPathName, null);
                                    if (!Directory.Exists(outdir))
                                        Directory.CreateDirectory(outdir);

                                    for (int index = 0; index < Count; index++)
                                    {
                                        var bitmap = Loader.ToBitmap(index);
                                        bitmap.Save(Path.Combine(outdir, string.Format("{0:0000}.png", index)), System.Drawing.Imaging.ImageFormat.Png);
                                    }
                                }

                                FileBuffer = null;
                                GC.Collect();
                            }
                        }
                    }
                }
            }
            return true;
        }
    }
}
