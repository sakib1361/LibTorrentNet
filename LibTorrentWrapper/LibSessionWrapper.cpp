#include "pch.h"
#include "LibSessionWrapper.h"
#include <iostream>
#include <fstream>
using namespace LibTorrentWrapper;

LibHandlerWrapper^ LibSessionWrapper::Create(TorrentConfig^ config)
{
	auto mg_link = AppUtilities::ToStdString(config->MagnetLink);
	auto downPath = AppUtilities::ToStdString(config->SavePath);
	auto resume_link = AppUtilities::ToStdString(config->ResumeFile);

	lt::add_torrent_params atp = lt::parse_magnet_uri(mg_link);

	std::ifstream ifs(resume_link, std::ios_base::binary);
	ifs.unsetf(std::ios_base::skipws);
	std::vector<char> buf{ std::istream_iterator<char>(ifs), std::istream_iterator<char>() };

	if (buf.size())
	{
		lt::add_torrent_params resume_param = lt::read_resume_data(buf);
		if (atp.info_hashes == resume_param.info_hashes) atp = std::move(resume_param);
	}

	atp.save_path = downPath;
	auto handler = ss->add_torrent(atp);
	auto instance = new LibHandlerInstance(handler, resume_link);
	auto manager = gcnew LibHandlerWrapper(instance, config);
	return manager;
}

void LibSessionWrapper::SaveSession(LibHandlerWrapper^ handler)
{
	handler->SaveSession(ss);
}
