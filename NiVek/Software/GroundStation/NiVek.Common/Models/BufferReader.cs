using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace NiVek.Common.Models
{
    public class BufferReader
    {
        public enum EndianType
        {
            BigEndian,
            LittleEndian
        }

        private EndianType _endian;

        private int _offset;
        private byte[] _buffer;
        private int _bufferSize;

        public byte[] GetRawBuffer()
        {
            return _buffer;
        }

        public BufferReader(byte[] buffer, int offset, int size, EndianType endian)
        {
            _offset = offset;
            _buffer = buffer;
            _bufferSize = size;
            _endian = endian;
        }

        public BufferReader(byte[] buffer, int offset, int size)
        {
            _offset = offset;
            _buffer = buffer;
            _bufferSize = size;
            _endian = EndianType.BigEndian;
        }

        public EndianType Endian
        {
            get { return _endian;  }
            set { _endian = value; }
        }

        public short ReadShort()
        {
            if (_endian == EndianType.BigEndian)
                return(short)( _buffer[_offset++] << 8 | _buffer[_offset++]);
            else
                return (short)(_buffer[_offset++]  | _buffer[_offset++] << 8);
        }

        public byte ReadByte()
        {
            return _buffer[_offset++];
        }


        public int Offset
        {
            get { return _offset; }
            set { _offset = value;  }
        }
    }
}
