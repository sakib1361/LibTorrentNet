#pragma once
using namespace System;
namespace LibTorrentWrapper
{
	public ref class TorrentConfig
	{
	public:
		bool IsMagnet;
		int MaxConnections;
		int UploadLimit;
		int DownloadLimit;
		int TopPieceDeadLine;
		int LowPieceDeadLine;
		String^ SavePath;
		String^ MagnetLink;
		String^ ResumeFile;
	};

	public ref class TorrentFileInfo
	{
	public:
		String^ FilePath;
		int Index;
		Int64 FileSize;

		TorrentFileInfo(String^ path, Int64 size, int index)
		{
			Index = index;
			FilePath = path;
			FileSize = size;
		}
	};

	public ref class TorrentFileCollection
	{
	public:
		array<TorrentFileInfo^>^ FileInfos;
		int PieceLength;
		int Index;
		String^ Name;
	};

	public ref class PieceInfo
	{
	public:
		int Index;
		double Progress;

		PieceInfo(int index, int totalBlock, int finished)
		{
			Index = index;
			Progress = finished * 100.0 / totalBlock;
		}

	};

	public enum class TorrentStateInfo
	{
		UnUsed,

		// The torrent has not started its download yet, and is
		// currently checking existing files.
		CheckingFiles,

		// The torrent is trying to download metadata from peers.
		// This implies the ut_metadata extension is in use.
		DownloadingMetadata,

		// The torrent is being downloaded. This is the state
		// most torrents will be in most of the time. The progress
		// meter will tell how much of the files that has been
		// downloaded.
		Downloading,

		// In this state the torrent has finished downloading but
		// still doesn't have the entire torrent. i.e. some pieces
		// are filtered and won't get downloaded.
		Finished,

		// In this state the torrent has finished downloading and
		// is a pure seeder.
		Seeding,
	};

	public ref class TorrentData 
	{
	public:
		bool HasMetaData;
		int Peers;
		int Seeders;
		long DownloadSpeed;
		long UploadSpeed;
		Int64 TotalSize;
		Int64 TotalDownloaded;
		Int64 TotalUploaded;
		TorrentStateInfo State = TorrentStateInfo::CheckingFiles;
	};

	
}
