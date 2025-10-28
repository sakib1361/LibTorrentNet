using LibTorrentWrapper;
using System.Diagnostics;

namespace LibTorrentNet
{
    public class LibHandler(string id, LibSessionWrapper session, TorrentConfig config, LibHandlerWrapper handler)
    {
        public string Id => id;
        private TorrentFileCollection fileCol;
        private DateTime _lastUpdate;
        private Action<TorrentData> _callback;
        private bool _stopped;
        private Stream stream;
        private bool _fileLocked;
        private TorrentFileInfo _currentFile;
        private readonly SemaphoreSlim slim = new(1);
        public async Task<TorrentData> GetMetaData()
        {
            await slim.WaitAsync();
            var meta = handler.GetMetadata();
            while (meta == null || meta.HasMetaData == false)
            {
                if (_stopped) break;
                meta = handler.GetMetadata();
                await Task.Delay(500);
            }
            slim.Release();
            return meta;
        }

        public async void SetHighPriority(TorrentFileInfo fileInfos)
        {
            if(_stopped) return;
            await slim.WaitAsync();
            if (_stopped == false)
            {
                handler.SetHighPriority([fileInfos.Index]);
            }
            slim.Release();
        }

        public async Task EnsureDownloads(TorrentFileInfo[] fileInfos)
        {
            if(_stopped) return;
            var indexes = fileInfos.Select(x => x.Index).ToArray();
            await slim.WaitAsync();
            handler.SetHighPriority(indexes);
            slim.Release();

            foreach (var i in fileInfos)
            {
                if (_stopped) break;
                var progress = handler.GetFileProgress(i.Index);
                await Task.Delay(500);
                while (_stopped == false && progress < i.FileSize)
                {
                    progress = handler.GetFileProgress(i.Index);
                    await Task.Delay(2000);
                }
            }
        }

        public void Resume()
        {
            handler.ResumeTorrent();
        }

        public async Task<TorrentFileCollection> GetFileCollectionAsync()
        {
            if (_stopped) return null;
            fileCol = await Task.Run(handler.GetTorrentFiles);
            return fileCol;
        }

        public async Task<Stream> InitializeStream(TorrentFileInfo info)
        {
            if (_stopped) return null;

            if (fileCol == null) await GetFileCollectionAsync();
            if (fileCol == null) return null;


            var torrentFile = Path.Combine(config.SavePath, info.FilePath);
            var progessData = handler.GetFileProgress(info.Index);
            if (progessData == info.FileSize && File.Exists(torrentFile))
            {
                stream = new FileStream(torrentFile, FileMode.Open, FileAccess.Read, FileShare.ReadWrite);
            }
            else
            {
                _fileLocked = true;
                _currentFile = info;
                long startPos = 0;
                foreach (var file in fileCol.FileInfos)
                {
                    if (file.FilePath == info.FilePath) break;
                    startPos+=file.FileSize;
                }
                var libstream = new LibStream(handler, info, torrentFile, startPos, fileCol.PieceLength);
                await libstream.InitStream();
                stream = libstream;
            }

            return stream;
        }

        internal async Task StopSession()
        {
            _stopped = true;
            stream?.Dispose();
            stream = null;
            await slim.WaitAsync();
            try
            {
                handler.PauseTorrent();
                session.SaveSession(handler);
                handler.Dispose();
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex);
            }
            slim.Release();
        }

        public void SubscribeMeta(Action<TorrentData> callback)
        {
            _callback = callback;
        }

        internal async Task UpdateMeta()
        {
            await slim.WaitAsync();
            TorrentData data = null;
            if (_callback != null)
            {
                data = handler.GetMetadata();
                _callback?.Invoke(data);
            }
            if ((DateTime.Now - _lastUpdate).TotalSeconds>30)
            {
                _lastUpdate = DateTime.Now;
                session.SaveSession(handler);
                if (_fileLocked && _currentFile != null)
                {
                    bool reqRelease = false;
                    data??=handler.GetMetadata();
                    if (data.State == TorrentStateInfo.Finished || data.State == TorrentStateInfo.Seeding)
                    {
                        reqRelease = true;
                    }
                    if (reqRelease == false)
                    {
                        var progessData = handler.GetFileProgress(_currentFile.Index);
                        reqRelease =  _currentFile.FileSize == progessData;
                    }

                    if (reqRelease)
                    {
                        _fileLocked = false;
                        handler.ResetPieceFocus();
                    }
                }
            }
            slim.Release();
        }
    }
}
