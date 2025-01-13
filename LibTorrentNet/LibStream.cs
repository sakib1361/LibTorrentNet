using LibTorrentWrapper;

namespace LibTorrentNet
{
    public class LibStream(LibHandlerWrapper handler,
        TorrentFileInfo info,
        string torrentFile, long start, int pieceSize) : Stream
    {
        private FileStream _fileStream;
        private bool _disposed;
        private int _lastPiece;
        private readonly LibHandlerWrapper _handler = handler;
        private readonly int _piece = pieceSize;
        private readonly long _start = start;
        private readonly List<int> _chunks = [];
        public override void Close()
        {
            base.Close();
            if (_disposed) return;
            _fileStream?.Close();
            _fileStream?.Dispose();
            _disposed = true;
            _fileStream = null;
        }

        public override ValueTask DisposeAsync()
        {
            if (!_disposed)
            {
                _fileStream?.Close();
                _fileStream?.Dispose();
                _disposed = true;
                _fileStream = null;
            }
            return base.DisposeAsync();
        }

        public override bool CanRead => true;

        public override bool CanSeek => true;

        public override bool CanWrite => false;

        public override long Length => _fileStream.Length;

        public override long Position { get => _fileStream?.Position??0; set => throw new Exception(); }

        public override void Flush()
        {
            _fileStream.Flush();
        }

        public async Task InitStream()
        {
            int pieceStart = (int)(_start / _piece);
            _lastPiece = (int)((_start+ info.FileSize)/_piece);
            var streamPieces = new int[] { pieceStart, pieceStart+1, pieceStart+2, _lastPiece };


            _handler.RequestPieces(streamPieces);
            while (_disposed == false && _handler.HasPiece(pieceStart) == false)
            {
                await Task.Delay(1000);
            }
            if (_disposed) return;
            int pieceEnd = (pieceStart + 100*1024*1024)/_piece;
            pieceEnd = Math.Max(pieceStart + 1, pieceEnd);
            _handler.FocusPieces(pieceStart, pieceEnd, _lastPiece);
            while (_disposed == false)
            {
                var progressState = _handler.GetFileProgress(info.Index);
                var percent = progressState*100.0/info.FileSize;
                var downloaded = progressState/(1024*1024);
                if (percent >=5 || downloaded > 30)
                {
                    break;
                }
                await Task.Delay(1000);
            }
            var pieceData = _handler.GetPieceProgress().OrderBy(x => x.Index).ToArray();
            _fileStream = new(torrentFile, FileMode.Open, FileAccess.Read, FileShare.ReadWrite);

        }


        public override async ValueTask<int> ReadAsync(Memory<byte> buffer, CancellationToken cancellationToken = default)
        {
            var startOffset = _start + Position;
            int piece = (int)(startOffset / _piece);

            if (_chunks.Contains(piece))
            {

            }
            else if (_handler.HasPiece(piece))
            {
                _chunks.Add(piece);
            }
            else
            {
                int pieceEnd = (int)((startOffset + Position + 10*1024*1024)/_piece);
                pieceEnd = Math.Max(piece + 1, pieceEnd);
                _handler.FocusPieces(piece, pieceEnd, _lastPiece);
                while (_disposed == false && _handler.HasPiece(piece) == false)
                {
                    await Task.Delay(200, cancellationToken).ConfigureAwait(false);
                }
                _chunks.Add(piece);
                if (_disposed) return 0;
            }
            return await _fileStream.ReadAsync(buffer, cancellationToken);
        }

        public override async Task<int> ReadAsync(byte[] buffer, int offset, int count, CancellationToken cancellationToken)
        {
            var startOffset = _start + offset + Position;
            int piece = (int)(startOffset / _piece);

            if (_chunks.Contains(piece))
            {

            }
            else if (_handler.HasPiece(piece))
            {
                _chunks.Add(piece);
            }
            else
            {
                int pieceEnd = (int)((startOffset + offset + Position + 5*1024*1024)/_piece);
                pieceEnd = Math.Max(piece + 1, pieceEnd);
                _handler.FocusPieces(piece, pieceEnd, _lastPiece);
                while (_disposed == false && _handler.HasPiece(piece) == false)
                {
                    await Task.Delay(200, cancellationToken).ConfigureAwait(false);
                }
                _chunks.Add(piece);
                if (_disposed) return 0;
            }
            return await _fileStream.ReadAsync(buffer, offset, count, cancellationToken);
        }

        public override long Seek(long offset, SeekOrigin origin)
        {
            if (_disposed) return 0;
            return _fileStream.Seek(offset, origin);
        }

        public override void SetLength(long value)
        {
            throw new NotImplementedException();
        }

        public override void Write(byte[] buffer, int offset, int count)
        {
            throw new NotImplementedException();
        }


        public override int Read(byte[] buffer, int offset, int count)
        {
            if (_disposed) return 0;
            //throw new NotImplementedException();
            var startOffset = _start + offset + Position;
            int piece = (int)(startOffset / _piece);

            if (_chunks.Contains(piece))
            {

            }
            else if (_handler.HasPiece(piece))
            {
                _chunks.Add(piece);
            }
            else
            {
                int pieceEnd = (int)((startOffset + offset + Position + 10*1024*1024)/_piece);
                pieceEnd = Math.Max(piece + 1, pieceEnd);
                _handler.FocusPieces(piece, pieceEnd, _lastPiece);
                while (_disposed == false && _handler.HasPiece(piece) == false)
                {
                    Thread.Sleep(500);
                }
                _chunks.Add(piece);
                if (_disposed) return 0;
            }

            return _fileStream.Read(buffer, offset, count);
        }
    }
}
