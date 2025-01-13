#pragma once
#include "Imports.h"
#include "LibHandlerWrapper.h"

namespace LibTorrentWrapper
{
	public ref class LibSessionWrapper
	{
	private:
		lt::session* ss;
	public:
		LibSessionWrapper()
		{
			lt::settings_pack settings;
			settings.set_int(lt::settings_pack::close_file_interval, 30);
			settings.set_bool(lt::settings_pack::close_redundant_connections, true);
			//settings.set_int(lt::settings_pack::deprecated_flush_write_cache, 30); // Flush every 30 seconds
		
			//settings.set_bool(lt::settings_pack::use_read_cache, true); // Enable read cache
			//settings.set_int(lt::settings_pack::write_cache_line_size, 32); // 512 KiB write chunks
			
			//settings.set_int(lt::settings_pack::torrent_connect_boost, 200);
			//settings.set_int(lt::settings_pack::disk_io_write_mode, lt::settings_pack::write_through); // Use OS cache
			ss = new lt::session(settings);
		}
		~LibSessionWrapper()
		{
			delete ss;
		}
		LibHandlerWrapper^ Create(TorrentConfig^ config);
		void SaveSession(LibHandlerWrapper^ handler);
	};
}
