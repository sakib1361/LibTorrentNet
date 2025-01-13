#include "pch.h"
#include "LibHandlerWrapper.h"
#include <iostream>
#include <fstream>

using namespace LibTorrentWrapper;

void LibHandlerInstance::save_resume_data(lt::session* ss)
{
	_handler.save_resume_data();

	auto i = ss->wait_for_alert(lt::seconds(30));

	// if we don't get an alert within 30 seconds, abort
	if (i == nullptr) return;

	std::vector<lt::alert*> alerts;
	ss->pop_alerts(&alerts);

	for (lt::alert* a : alerts)
	{
		if (auto rd = lt::alert_cast<lt::save_resume_data_alert>(a))
		{
			std::ofstream of(resume_link, std::ios_base::binary);
			of.unsetf(std::ios_base::skipws);
			auto const b = lt::write_resume_data_buf(rd->params);
			of.write(b.data(), int(b.size()));
			break;
		}
	}
}

bool LibHandlerWrapper::HasPiece(int index)
{
	auto handler = instance->Get();
	lt::piece_index_t pc(index);
	return	handler->have_piece(pc);
}

void LibHandlerWrapper::RequestPieces(array<int>^ indexFiles)
{
	auto handler = instance->Get();
	handler->clear_piece_deadlines();
	for (auto i : handler->torrent_file().get()->piece_range())
	{
		if (AppUtilities::Contains(indexFiles, safe_cast<int>(i)))
		{
			handler->piece_priority(i, lt::top_priority);
		}
		else
		{
			handler->piece_priority(i, lt::dont_download);
		}
	}
}

void LibHandlerWrapper::FocusPieces(int start, int end, int fileLength)
{
	auto handler = instance->Get();
	handler->clear_piece_deadlines();
	int index = 0;
	for (auto i : handler->torrent_file().get()->piece_range())
	{
		if (index >= start && index <= end)
		{
			handler->piece_priority(i, lt::top_priority);
			handler->set_piece_deadline(i, _config->TopPieceDeadLine * (1 + index - start));
		}
		else if (index < start || index > fileLength)
		{
			handler->piece_priority(i, lt::dont_download);
		}
		else
		{
			handler->piece_priority(i, lt::low_priority);
			handler->set_piece_deadline(i, _config->LowPieceDeadLine * (1 + index - start));
		}
		index++;
	}
}

void LibHandlerWrapper::ResetPieceFocus()
{
	auto handler = instance->Get();
	handler->clear_piece_deadlines();
	for (auto i : handler->torrent_file().get()->piece_range())
	{
		handler->piece_priority(i, lt::default_priority);
	}
}

array<PieceInfo^>^ LibHandlerWrapper::GetPieceProgress()
{
	auto handler = instance->Get();
	auto pieceDataCol = handler->get_download_queue();
	array<PieceInfo^>^ pieceArray = gcnew array<PieceInfo^>(pieceDataCol.size());
	for (int i = 0; i < pieceDataCol.size(); i++)
	{
		int pIndex = static_cast<int>(pieceDataCol[i].piece_index);
		pieceArray[i] = gcnew PieceInfo(pIndex, pieceDataCol[i].blocks_in_piece, pieceDataCol[i].finished);
	}
	return pieceArray;
}


void LibHandlerWrapper::PauseTorrent()
{
	auto handler = instance->Get();
	handler->pause();
}

void LibHandlerWrapper::ResumeTorrent()
{
	auto handler = instance->Get();
	handler->resume();
	auto torrentInfo = handler->torrent_file().get();
	lt::file_storage files = torrentInfo->files();

	for (auto const i : files.file_range())
	{
		int index = static_cast<int>(i);
		handler->file_priority(i, lt::default_priority);
	}
}


void LibHandlerWrapper::SetHighPriority(array<int>^ indexFiles)
{
	auto handler = instance->Get();
	if (handler->is_valid() == false) return;
	auto torrentInfo = handler->torrent_file().get();
	lt::file_storage files = torrentInfo->files();

	for (auto const i : files.file_range())
	{
		int index = static_cast<int>(i);
		if (AppUtilities::Contains(indexFiles, index))
		{
			handler->file_priority(i, lt::top_priority);
		}
		else
		{
			handler->file_priority(i, lt::low_priority);
		}
	}
}


TorrentFileCollection^ LibHandlerWrapper::GetTorrentFiles()
{
	TorrentFileCollection^ col = gcnew TorrentFileCollection();
	auto handler = instance->Get();
	auto torrentInfo = handler->torrent_file().get();
	lt::file_storage files = torrentInfo->files();
	auto fileCount = files.num_files();
	array<TorrentFileInfo^>^ fileArray = gcnew array<TorrentFileInfo^>(fileCount);

	for (lt::file_index_t i(0); i < lt::file_index_t(fileCount); ++i)
	{
		std::string filePath = files.file_path(i);
		std::int64_t fileSize = files.file_size(i);
		int index = safe_cast<int>(i);
		fileArray[index] = gcnew TorrentFileInfo(gcnew String(filePath.c_str()), fileSize, index);
	}
	col->FileInfos = fileArray;
	col->PieceLength = torrentInfo->piece_length();
	return col;
}

TorrentData^ LibHandlerWrapper::GetMetadata()
{
	TorrentData^ td = gcnew TorrentData();
	auto handler = instance->Get();
	if (handler->is_valid() == false) return td;
	lt::torrent_status stat = handler->status();
	td->HasMetaData = stat.has_metadata;
	td->DownloadSpeed = stat.download_rate;
	td->UploadSpeed = stat.upload_rate;
	td->TotalSize = stat.total;
	td->TotalDownloaded = stat.all_time_download;
	td->TotalUploaded = stat.all_time_upload;
	td->State = (TorrentStateInfo)((int)(stat.state));
	td->Peers = stat.num_peers;
	td->Seeders = stat.num_seeds;
	return td;
}

long long LibHandlerWrapper::GetFileProgress(int fileIndex)
{
	auto handler = instance->Get();
	auto progressData = handler->file_progress(lt::torrent_handle::piece_granularity);
	long long data = progressData[fileIndex];
	return data;
}
