using LibTorrentWrapper;
using System.Security.Cryptography;
using System.Text;
using System.Timers;

namespace LibTorrentNet
{
    public class LibSession : IDisposable
    {
        private LibSessionWrapper _session;
        private readonly System.Timers.Timer _timer;
        private Dictionary<string, LibHandler> _handlers = [];
        private bool _locked;
        public LibSession()
        {
            _session = new LibSessionWrapper();
            _timer = new System.Timers.Timer(1000);
            _timer.Elapsed+=Timer_Elapsed;
            _timer.Start();
        }

        private async void Timer_Elapsed(object sender, ElapsedEventArgs e)
        {
            if (_locked) return;
            _locked = true;
            foreach (var handler in _handlers.Values)
            {
                await handler.UpdateMeta();
            }
            _locked = false;
        }

        public LibHandler AddTorrent(string url, string folderPath)
        {
            var isMagnet = url.StartsWith("magnet");
            var cacheFolder = Path.Combine(folderPath, "_cache");
            var hashDataSet = GetHashString(url);
            var resumeFile = Path.Combine(cacheFolder, hashDataSet+".resume");
            Directory.CreateDirectory(cacheFolder);


            var config = new TorrentConfig()
            {
                IsMagnet = isMagnet,
                MagnetLink = url,
                ResumeFile = resumeFile,
                SavePath = folderPath,
                TopPieceDeadLine = 2000,
                LowPieceDeadLine = 10000,
            };
            var handler = _session.Create(config);
            var handlerState = new LibHandler(hashDataSet, _session, config, handler);
            _handlers.TryAdd(hashDataSet, handlerState);
            return handlerState;
        }

        private static string GetHashString(string inputString)
        {
            StringBuilder sb = new StringBuilder();
            foreach (byte b in SHA256.HashData(Encoding.UTF8.GetBytes(inputString)))
                sb.Append(b.ToString("X2"));

            return sb.ToString();
        }

        public async void Dispose()
        {
            _timer.Stop();
            _timer.Dispose();
            foreach (var handler in _handlers.Values)
            {
                await handler.StopSession();
            }
            _session.Dispose();
        }

        public async Task StopTorrent(LibHandler manager)
        {
            if (_handlers.Remove(manager.Id))
            {
                await manager.StopSession();
            }
        }
    }
}
