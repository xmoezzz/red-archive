using System;
using System.Collections.Generic;
using System.Text;
using System.Collections;
using System.Drawing;
using System.IO;

namespace SprinterPublishing
{

    internal class Utils
    {
        /// <summary>
        /// Attempts to read count bytes of data from the supplied stream.
        /// </summary>
        /// <param name="stream">The stream to read from</param>
        /// <param name="count">The number of bytes to read</param>
        /// <returns>A byte[] containing the data or null if an error occurred</returns>
        public static byte[] ReadStream(Stream stream, uint count)
        {
            byte[] bytes = new byte[count];
            try
            {
                stream.Read(bytes, 0, (int)count);
            }
            catch (IOException)
            {
                throw;
            }
            return bytes;
        }

        /// <summary>
        /// Attempts to parse an unsigned integer value from the array of bytes
        /// provided.  The most significant byte of the unsigned integer is
        /// parsed from the first element in the array.
        /// </summary>
        /// <param name="buffer">An array of bytes from which the value is to be extracted</param>
        /// <param name="uintLengthInBytes">The number of bytes to parse (must be <= sizeof(uint))</param>
        /// <returns>The extracted unsigned integer returned in a uint</returns>
        public static uint ParseUint(byte[] buffer, int uintLengthInBytes)
        {
            int offset = 0;
            return ParseUint(buffer, uintLengthInBytes, ref offset);
        }

        /// <summary>
        /// Attempts to parse an unsigned integer value from the array of bytes
        /// provided.  The most significant byte of the unsigned integer is
        /// parsed from the specified offset in the array.
        /// </summary>
        /// <param name="buffer">An array of bytes from which the value is to be extracted</param>
        /// <param name="uintLengthInBytes">The number of bytes to parse (must be <= sizeof(uint))</param>
        /// <param name="offset">The offset in the array of bytes where parsing shall begin</param>
        /// <returns>The extracted unsigned integer returned in a uint</returns>
        public static uint ParseUint(byte[] buffer, int uintLengthInBytes, ref int offset)
        {
            uint value = 0;
            if (uintLengthInBytes > sizeof(uint))
                throw new ArgumentException(
                    String.Format("Function can only be used to parse up to {0} bytes from the buffer",
                    sizeof(uint)));
            if (buffer.Length - offset < uintLengthInBytes)
                throw new ArgumentException(
                    String.Format("buffer is not long enough to extract {0} bytes at offset {1}",
                    sizeof(uint), offset));
            int i, j;
            for (i = offset + uintLengthInBytes - 1, j = 0; i >= offset; i--, j++)
                value |= (uint)(buffer[i] << (8 * j));
            offset += uintLengthInBytes;
            return value;
        }
    }

    internal class MNGHeader
    {
        /// <summary>
        /// The first 8 bytes of an MNG encoding
        /// </summary>
        static byte[] expectedSignature = { 0x8a, 0x4d, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a };
        /// <summary>
        /// The signature parsed from the input stream
        /// </summary>
        private byte[] signature;

        /// <summary>
        /// Default constructor
        /// </summary>
        public MNGHeader()
        {
            signature = null;
        }

        /// <summary>
        /// Attempts to read an MNG Header chunk from the supplied stream.
        /// </summary>
        /// <param name="stream">The stream containing the MNG Header</param>
        public void Read(Stream stream)
        {
            // Stream must be readable
            if (!stream.CanRead)
                throw new ArgumentException("Stream is not readable");

            // Read the signature
            try
            {
                signature = Utils.ReadStream(stream, 8);
            }
            catch (Exception)
            {
                // Re-throw any exceptions
                throw;
            }

            // Test signature for validity
            if (signature.Length == expectedSignature.Length)
            {
                for (int i = 0; i < expectedSignature.Length; i++)
                {
                    // Invalid signature
                    if (expectedSignature[i] != signature[i])
                        throw new ApplicationException("MNG signature not found.");
                }
            }
            else
                // Invalid signature
                throw new ApplicationException("MNG signature not found.");
        }
    }

    internal class MENDChunk : MNGChunk
    {
        /// <summary>
        /// The ASCII name of the MNG chunk
        /// </summary>
        public const String NAME = "MEND";

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="chunk">The MNG chunk containing the data for this specific chunk</param>
        public MENDChunk(MNGChunk chunk)
            : base(chunk, NAME)
        {
        }
    }

    internal class TERMChunk : MNGChunk
    {
        /// <summary>
        /// The ASCII name of the MNG chunk
        /// </summary>
        public const String NAME = "TERM";

        private uint terminationAction;
        private uint actionAfterTermination;
        private uint delay;
        private uint iterationMax;

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="chunk">The MNG chunk containing the data for this specific chunk</param>
        public TERMChunk(MNGChunk chunk)
            : base(chunk, NAME)
        {
        }

        /// <summary>
        /// Extracts various fields specific to this chunk from the MNG's
        /// data field
        /// </summary>
        /// <param name="chunkData">An array of bytes representing the MNG's data field</param>
        protected override void ParseData(byte[] chunkData)
        {
            int offset = 0;
            terminationAction = Utils.ParseUint(chunkData, 1, ref offset);
            // If the data length is > 1 then read 9 more bytes
            if (chunkData.Length > 1)
            {
                actionAfterTermination = Utils.ParseUint(chunkData, 1, ref offset);
                delay = Utils.ParseUint(chunkData, 4, ref offset);
                iterationMax = Utils.ParseUint(chunkData, 4, ref offset);
            }
        }
    }

    internal class BKGDChunk : MNGChunk
    {
        /// <summary>
        /// The ASCII name of the MNG chunk
        /// </summary>
        public const String NAME = "bKGD";

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="chunk">The MNG chunk containing the data for this specific chunk</param>
        public BKGDChunk(MNGChunk chunk)
            : base(chunk, NAME)
        {
        }
    }

    internal class BACKChunk : MNGChunk
    {
        /// <summary>
        /// The ASCII name of the MNG chunk
        /// </summary>
        public const String NAME = "BACK";

        private uint redBackground;
        private uint greenBackground;
        private uint blueBackground;
        private uint mandatoryBackground;
        private uint backgroundImageId;
        private uint backgroundTiling;

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="chunk">The MNG chunk containing the data for this specific chunk</param>
        public BACKChunk(MNGChunk chunk)
            : base(chunk, NAME)
        {
        }

        /// <summary>
        /// Extracts various fields specific to this chunk from the MNG's
        /// data field
        /// </summary>
        /// <param name="chunkData">An array of bytes representing the MNG's data field</param>
        protected override void ParseData(byte[] chunkData)
        {
            int offset = 0;
            redBackground = Utils.ParseUint(chunkData, 2, ref offset);
            greenBackground = Utils.ParseUint(chunkData, 2, ref offset);
            blueBackground = Utils.ParseUint(chunkData, 2, ref offset);

            // If the data length is > 6 then read 1 more byte
            if (chunkData.Length > 6)
            {
                mandatoryBackground = Utils.ParseUint(chunkData, 1, ref offset);
            }
            // If the data length is > 7 then read 2 more bytes
            if (chunkData.Length > 7)
            {
                backgroundImageId = Utils.ParseUint(chunkData, 2, ref offset);
            }
            // If the data length is > 9 then read 1 more byte
            if (chunkData.Length > 9)
            {
                backgroundTiling = Utils.ParseUint(chunkData, 1, ref offset);
            }
        }
    }

    internal class IHDRChunk : MNGChunk
    {
        /// <summary>
        /// The ASCII name of the MNG chunk
        /// </summary>
        public const String NAME = "IHDR";

        private uint width;
        private uint height;
        private uint bitDepth;
        private uint colorType;
        private uint compressionMethod;
        private uint filterMethod;
        private uint interlaceMethod;

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="chunk">The MNG chunk containing the data for this specific chunk</param>
        public IHDRChunk(MNGChunk chunk)
            : base(chunk, NAME)
        {
        }

        /// <summary>
        /// Extracts various fields specific to this chunk from the MNG's
        /// data field
        /// </summary>
        /// <param name="chunkData">An array of bytes representing the MNG's data field</param>
        protected override void ParseData(byte[] chunkData)
        {
            int offset = 0;
            width = Utils.ParseUint(chunkData, 4, ref offset);
            height = Utils.ParseUint(chunkData, 4, ref offset);
            bitDepth = Utils.ParseUint(chunkData, 1, ref offset);
            colorType = Utils.ParseUint(chunkData, 1, ref offset);
            compressionMethod = Utils.ParseUint(chunkData, 1, ref offset);
            filterMethod = Utils.ParseUint(chunkData, 1, ref offset);
            interlaceMethod = Utils.ParseUint(chunkData, 1, ref offset);
        }
    }

    internal class PLTEChunk : MNGChunk
    {
        /// <summary>
        /// The ASCII name of the MNG chunk
        /// </summary>
        public const String NAME = "PLTE";

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="chunk">The MNG chunk containing the data for this specific chunk</param>
        public PLTEChunk(MNGChunk chunk)
            : base(chunk, NAME)
        {
        }

        public bool IsEmpty()
        {
            return ChunkLength == 0;
        }

    }

    internal class IENDChunk : MNGChunk
    {
        /// <summary>
        /// The ASCII name of the MNG chunk
        /// </summary>
        public const String NAME = "IEND";

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="chunk">The MNG chunk containing the data for this specific chunk</param>
        public IENDChunk(MNGChunk chunk)
            : base(chunk, NAME)
        {
        }
    }

    internal class IDATChunk : MNGChunk
    {
        /// <summary>
        /// The ASCII name of the MNG chunk
        /// </summary>
        public const String NAME = "IDAT";

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="chunk">The MNG chunk containing the data for this specific chunk</param>
        public IDATChunk(MNGChunk chunk)
            : base(chunk, NAME)
        {
        }
    }

    internal class MHDRChunk : MNGChunk
    {
        /// <summary>
        /// The ASCII name of the MNG chunk
        /// </summary>
        public const String NAME = "MHDR";

        /// <summary>
        /// The MNG frame width in pixels
        /// </summary>
        private uint frameWidth;
        /// <summary>
        /// The MNG frame height in pixels
        /// </summary>
        private uint frameHeight;
        /// <summary>
        /// The MNG frame rate
        /// </summary>
        private uint ticksPerSecond;
        /// <summary>
        /// The MNG layer count
        /// </summary>
        private uint nominalLayerCount;
        /// <summary>
        /// The MNG frame count
        /// </summary>
        private uint nominalFrameCount;
        /// <summary>
        /// The MNG play time
        /// </summary>
        private uint nominalPlayTime;
        /// <summary>
        /// The MNG simplicity profile
        /// </summary>
        private uint simplicityProfile;

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="chunk">The MNG chunk containing the data for this specific chunk</param>
        public MHDRChunk(MNGChunk chunk)
            : base(chunk, NAME)
        {
        }

        /// <summary>
        /// The MNG width in pixels
        /// </summary>
        public uint FrameWidth
        {
            get { return frameWidth; }
        }

        /// <summary>
        /// The MNG height in pixels
        /// </summary>
        public uint FrameHeight
        {
            get { return frameHeight; }
        }

        /// <summary>
        /// Extracts various fields specific to this chunk from the MNG's
        /// data field
        /// </summary>
        /// <param name="chunkData">An array of bytes representing the MNG's data field</param>
        protected override void ParseData(byte[] chunkData)
        {
            int offset = 0;
            frameWidth = Utils.ParseUint(chunkData, 4, ref offset);
            frameHeight = Utils.ParseUint(chunkData, 4, ref offset);
            ticksPerSecond = Utils.ParseUint(chunkData, 4, ref offset);
            nominalLayerCount = Utils.ParseUint(chunkData, 4, ref offset);
            nominalFrameCount = Utils.ParseUint(chunkData, 4, ref offset);
            nominalPlayTime = Utils.ParseUint(chunkData, 4, ref offset);
            simplicityProfile = Utils.ParseUint(chunkData, 4, ref offset);
        }
    }

    internal class MNGChunk
    {
        protected String error;
        protected byte[] chunkLength;
        protected byte[] chunkType;
        protected byte[] chunkData;
        protected byte[] chunkCRC;
        protected uint calculatedCRC;

        /// <summary>
        /// Default constructor
        /// </summary>
        public MNGChunk()
        {
            chunkLength = chunkType = chunkData = chunkCRC = null;
            error = "No Error";
        }

        /// <summary>
        /// Constructor which takes an existing MNGChunk object and
        /// verifies that its type matches that which is expected
        /// </summary>
        /// <param name="chunk">The MNGChunk to copy</param>
        /// <param name="expectedType">The input MNGChunk expected type</param>
        public MNGChunk(MNGChunk chunk, String expectedType)
        {
            // Copy the existing chunks members
            chunkLength = chunk.chunkLength;
            chunkType = chunk.chunkType;
            chunkData = chunk.chunkData;
            chunkCRC = chunk.chunkCRC;

            // Verify the chunk type is as expected
            if (ChunkType != expectedType)
                throw new ArgumentException(
                    String.Format("Specified chunk type is not {0} as expected", expectedType));

            // Parse the chunk's data
            ParseData(chunkData);
        }

        /// <summary>
        /// Extracts various fields specific to this chunk from the MNG's
        /// data field
        /// </summary>
        /// <param name="chunkData">An array of bytes representing the MNG's data field</param>
        protected virtual void ParseData(byte[] chunkData)
        {
            // Nothing specific to do here.  Derived classes can override this
            // to do specific field parsing.
        }

        /// <summary>
        /// Gets the array of bytes which make up the MNG chunk.  This includes:
        /// o 4 bytes of the chunk's length
        /// o 4 bytes of the chunk's type
        /// o N bytes of the chunk's data
        /// o 4 bytes of the chunk's CRC
        /// </summary>
        public byte[] Chunk
        {
            get
            {
                byte[] ba = new byte[chunkLength.Length +
                    chunkType.Length + chunkData.Length +
                    chunkCRC.Length];
                chunkLength.CopyTo(ba, 0);
                chunkType.CopyTo(ba, chunkLength.Length);
                chunkData.CopyTo(ba, chunkLength.Length + chunkType.Length);
                chunkCRC.CopyTo(ba, chunkLength.Length + chunkType.Length + chunkData.Length);
                return ba;
            }
        }

        /// <summary>
        /// Gets the array of bytes which make up the chunk's data field
        /// </summary>
        public byte[] ChunkData
        {
            get
            {
                return chunkData;
            }
        }

        /// <summary>
        /// Gets chunk's type field as an string
        /// </summary>
        public String ChunkType
        {
            get
            {
                return new String(new char[] { (char)chunkType[0], (char)chunkType[1], (char)chunkType[2], (char)chunkType[3] });
            }
        }

        /// <summary>
        /// Gets the length field of the chunk
        /// </summary>
        public uint ChunkLength
        {
            get
            {
                return Utils.ParseUint(chunkLength, chunkLength.Length);
            }
        }

        /// <summary>
        /// Gets the CRC field of the chunk
        /// </summary>
        public uint ChunkCRC
        {
            get
            {
                return Utils.ParseUint(chunkCRC, chunkCRC.Length);
            }
        }

        /// <summary>
        /// Attempts to parse an MNGChunk for the specified stream
        /// </summary>
        /// <param name="stream">The stream containing the MNG Chunk</param>
        public void Read(Stream stream)
        {
            if (!stream.CanRead)
                throw new ArgumentException("Stream is not readable");

            calculatedCRC = CRC.INITIAL_CRC;

            long chunkStart = stream.Position;

            // Read the data Length
            chunkLength = Utils.ReadStream(stream, 4);

            // Read the chunk type
            chunkType = Utils.ReadStream(stream, 4);
            calculatedCRC = CRC.UpdateCRC(calculatedCRC, chunkType);

            // Read the data
            chunkData = Utils.ReadStream(stream, ChunkLength);
            calculatedCRC = CRC.UpdateCRC(calculatedCRC, chunkData);

            // Read the CRC
            chunkCRC = Utils.ReadStream(stream, 4);

            // CRC is inverted
            calculatedCRC = ~calculatedCRC;

            // Verify the CRC
            if (ChunkCRC != calculatedCRC)
            {
                StringBuilder sb = new StringBuilder();
                sb.AppendLine(String.Format("MNG Chunk CRC Mismatch.  Chunk CRC = {0}, Calculated CRC = {1}.",
                    ChunkCRC, calculatedCRC));
                sb.AppendLine(String.Format("This occurred while parsing the chunk at position {0} (0x{1:X8}) in the stream.",
                    chunkStart, chunkStart));
                throw new ApplicationException(sb.ToString());
            }
        }
    }

    internal class PNG
    {
        /// <summary>
        /// The PNG file signature
        /// </summary>
        private static byte[] header = new byte[] { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };
        /// <summary>
        /// The PNG file's IHDR chunk
        /// </summary>
        private IHDRChunk ihdr;
        /// <summary>
        /// The PNG file's PLTE chunk (optional)
        /// </summary>
        private PLTEChunk plte;
        /// <summary>
        /// The PNG file's IDAT chunks
        /// </summary>
        private List<IDATChunk> idats;
        /// <summary>
        /// The PNG file's IEND chunk
        /// </summary>
        private IENDChunk iend;

        /// <summary>
        /// Default constructor
        /// </summary>
        public PNG()
        {
            idats = new List<IDATChunk>();
        }

        /// <summary>
        /// Converts the chunks making up the PNG into a single MemoryStream which
        /// is suitable for writing to a PNG file or creating a Image object using
        /// Bitmap.FromStream
        /// </summary>
        /// <returns>MemoryStream</returns>
        public MemoryStream ToStream()
        {
            MemoryStream ms = new MemoryStream();
            ms.Write(header, 0, header.Length);
            ms.Write(ihdr.Chunk, 0, ihdr.Chunk.Length);
            if (plte != null)
                ms.Write(plte.Chunk, 0, plte.Chunk.Length);
            foreach (IDATChunk chunk in idats)
                ms.Write(chunk.Chunk, 0, chunk.Chunk.Length);
            ms.Write(iend.Chunk, 0, iend.Chunk.Length);
            return ms;
        }

        /// <summary>
        /// Gets or Sets the PNG's IHDR chunk
        /// </summary>
        public IHDRChunk IHDR
        {
            get { return ihdr; }
            set { ihdr = value; }
        }

        /// <summary>
        /// Gets or Sets the PNG's PLTE chunk
        /// </summary>
        public PLTEChunk PLTE
        {
            get { return plte; }
            set { plte = value; }
        }

        /// <summary>
        /// Gets or Sets the PNG's IEND chunk
        /// </summary>
        public IENDChunk IEND
        {
            get { return iend; }
            set { iend = value; }
        }

        /// <summary>
        /// Gets the list of IDAT chunk's making up the PNG
        /// </summary>
        public List<IDATChunk> IDATS
        {
            get { return idats; }
        }

        /// <summary>
        /// Adds the assigned IDAT chunk to the end of the PNG's list of IDAT chunks
        /// </summary>
        public IDATChunk IDAT
        {
            set { idats.Add(value); }
        }
    }

    public class MNG
    {
        /// <summary>
        /// List of chunks in the MNG
        /// </summary>
        List<MNGChunk> chunks;
        /// <summary>
        /// List of PNGs embedded in the MNG
        /// </summary>
        List<PNG> pngs;
        /// <summary>
        /// The MNG's MHDRChunk
        /// </summary>
        MHDRChunk headerChunk;

        /// <summary>
        /// Gets the number of embedded PNGs within the MNG
        /// </summary>
        public int NumEmbeddedPNG
        {
            get { return pngs.Count; }
        }

        /// <summary>
        /// Creates a Bitmap object containing the embedded PNG at the specified
        /// index in the MNG's list of embedded PNGs
        /// </summary>
        /// <param name="index">The embedded PNG index</param>
        /// <returns>Bitmap</returns>
        public Bitmap ToBitmap(int index)
        {
            // Verify the index
            if (index > NumEmbeddedPNG)
                throw new ArgumentException(String.Format(
                    "Embedded PNG index must be between 0 and {0}", NumEmbeddedPNG - 1));
            // Create the bitmap
            Bitmap b = (Bitmap)Bitmap.FromStream(pngs[index].ToStream());
            return b;
        }

        /// <summary>
        /// Creates a string containing the names of all the chunks in the MNG
        /// </summary>
        /// <returns>String</returns>
        public override string ToString()
        {
            StringBuilder sb = new StringBuilder();
            foreach (MNGChunk chunk in chunks)
                sb.AppendLine(chunk.ChunkType);
            return sb.ToString();
        }

        /// <summary>
        /// Attempts to load an MNG from the specified file name
        /// </summary>
        /// <param name="filename">Name of the MNG file to load</param>
        public void Load(string filename)
        {
            chunks = new List<MNGChunk>();
            pngs = new List<PNG>();

            // Open the file for reading
            Stream stream = File.OpenRead(filename);

            // Create a new header (should be 1 per file) and
            // read it from the stream
            MNGHeader header = new MNGHeader();
            try
            {
                header.Read(stream);
            }
            catch (Exception)
            {
                stream.Close();
                throw;
            }

            MNGChunk chunk;
            PNG png = null;
            PLTEChunk globalPLTE = null;

            // Read chunks from the stream until we reach the MEND chunk
            do
            {
                // Read a generic Chunk
                chunk = new MNGChunk();
                try
                {
                    chunk.Read(stream);
                }
                catch (Exception)
                {
                    stream.Close();
                    throw;
                }

                // Take a look at the chunk type and decide what derived class to
                // use to create a specific chunk
                switch (chunk.ChunkType)
                {
                    case MHDRChunk.NAME:
                        if (headerChunk != null)
                            throw new ApplicationException(String.Format(
                                "Only one chunk of type {0} is allowed", chunk.ChunkType));
                        chunk = headerChunk = new MHDRChunk(chunk);
                        break;
                    case MENDChunk.NAME:
                        chunk = new MENDChunk(chunk);
                        break;
                    case TERMChunk.NAME:
                        chunk = new TERMChunk(chunk);
                        break;
                    case BACKChunk.NAME:
                        chunk = new BACKChunk(chunk);
                        break;
                    case BKGDChunk.NAME:
                        chunk = new BKGDChunk(chunk);
                        break;
                    case PLTEChunk.NAME:
                        chunk = new PLTEChunk(chunk);
                        // We can get an PLTE chunk w/o having gotten
                        // an IHDR
                        if (png != null)
                        {
                            png.PLTE = chunk as PLTEChunk;
                            if (png.PLTE.IsEmpty())
                            {
                                if (globalPLTE == null)
                                    throw new ApplicationException(String.Format(
                                        "An empty PLTE chunk was found inside a IHDR/IEND pair but no 'global' PLTE chunk was found"));
                                png.PLTE = globalPLTE;
                            }
                        }
                        else
                            globalPLTE = chunk as PLTEChunk;
                        break;
                    case IHDRChunk.NAME:
                        chunk = new IHDRChunk(chunk);
                        // This is the beginning of a new embedded PNG
                        png = new PNG();
                        png.IHDR = chunk as IHDRChunk;
                        break;
                    case IDATChunk.NAME:
                        chunk = new IDATChunk(chunk);
                        // We shouldn't get an IDAT chunk if we haven't yet
                        // gotten an IHDR chunk
                        if (png == null)
                            throw new ArgumentNullException("png");
                        png.IDAT = chunk as IDATChunk;
                        break;
                    case IENDChunk.NAME:
                        chunk = new IENDChunk(chunk);
                        // We can get an IEND chunk w/o having gotten
                        // an IHDR
                        if (png != null)
                        {
                            // However, if we've gotten an IHDR chunk then
                            // this is the end of the embedded PNG
                            png.IEND = chunk as IENDChunk;
                            pngs.Add(png);
                            png = null;
                        }
                        break;
                    default:
                        break;
                }
                // Add the chunk to our list of chunks
                chunks.Add(chunk);
            }
            while (chunk.ChunkType != MENDChunk.NAME);
        }
    }
}

