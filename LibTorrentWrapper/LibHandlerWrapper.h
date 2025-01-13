#pragma once
#include "Imports.h"
namespace LibTorrentWrapper
{
	using namespace System;
	class LibHandlerInstance
	{
	private:
		lt::torrent_handle _handler;
		std::string resume_link;
		int currentIndex = -1;
	public:
		LibHandlerInstance(lt::torrent_handle handler, std::string resume_link_new)
		{
			_handler = handler;
			resume_link = resume_link_new;
		}

		~LibHandlerInstance()
		{

		}
		lt::torrent_handle* Get()
		{
			return &_handler;
		}
		//bool RequestPiece(int pieceIndex, bool urgent);
		void save_resume_data(lt::session* ss);
	};

	public	ref class LibHandlerWrapper
	{
	private:
		LibHandlerInstance* instance;
		TorrentConfig^ _config;
	internal:
		LibHandlerWrapper(LibHandlerInstance* instanceData, TorrentConfig^ config)
		{
			_config = config;
			instance = instanceData;
		}

		~LibHandlerWrapper()
		{
			delete instance;
		}
		void SaveSession(lt::session* ss)
		{
			instance->save_resume_data(ss);
		}

	public:
		void PauseTorrent();
		void ResumeTorrent();
		void RequestPieces(array<int>^ pieceIndex);
		bool HasPiece(int index);
		array<PieceInfo^>^ GetPieceProgress();
		void FocusPieces(int start, int end, int fileLength);
		void ResetPieceFocus();
		TorrentFileCollection^ GetTorrentFiles();
		TorrentData^ GetMetadata();
		void SetHighPriority(array<int>^ indexFiles);
		long long GetFileProgress(int fileIndex);
	};
}

