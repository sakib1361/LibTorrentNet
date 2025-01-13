#LibHandler Library

##Overview

LibHandler is a robust library for managing torrent downloads in a .NET environment. It provides an interface to control torrent sessions, prioritize files, and handle streaming of incomplete files seamlessly. The library is built with multithreading safety and includes mechanisms for metadata subscription, file prioritization, and session management.

##Features

Add torrents: Add new torrents using magnet links or file URLs.

Metadata management: Subscribe to updates and fetch metadata asynchronously.

File prioritization: Set high priority for specific files within a torrent.

Streaming support: Stream torrent files as they download.

Session management: Save and restore torrent sessions to persist state.

Periodic updates: Automatically updates metadata and session state every second.

##Installation

Include the library in your .NET project. You can download the source code from this repository and compile it, or use the precompiled DLL (if available).

###Getting Started

Here is a quick example to demonstrate how to use LibHandler in your project.

Example Usage

```C#
using System;
using System.IO;
using System.Threading.Tasks;

class Program
{
    static async Task Main(string[] args)
    {
        // Initialize the session
        using var libSession = new LibSession();

        // Add a torrent
        string magnetLink = "magnet:?xt=urn:btih:examplehash&dn=example";
        string downloadFolder = Path.Combine(Environment.CurrentDirectory, "Downloads");
        var handler = libSession.AddTorrent(magnetLink, downloadFolder);

        // Subscribe to metadata updates
        handler.SubscribeMeta(data => Console.WriteLine($"Torrent Name: {data.Name}, Progress: {data.Progress}%"));

        // Fetch file collection
        var fileCollection = await handler.GetFileCollectionAsync();
        Console.WriteLine("Files in Torrent:");
        foreach (var file in fileCollection.FileInfos)
        {
            Console.WriteLine($"{file.FilePath} - {file.FileSize} bytes");
        }

        // Ensure download of specific files
        await handler.EnsureDownloads(new[] { fileCollection.FileInfos[0] });

        // Stream a file
        var stream = await handler.InitializeStream(fileCollection.FileInfos[0]);
        using (var reader = new StreamReader(stream))
        {
            Console.WriteLine(await reader.ReadLineAsync());
        }

        // Stop the session
        libSession.StopTorrent(handler);
    }
}```

##API Documentation

###Classes and Methods

####LibHandler

Constructor: LibHandler(string id, LibSessionWrapper session, TorrentConfig config, LibHandlerWrapper handler)

Methods:

 - Task<TorrentData> GetMetaData() - Retrieves torrent metadata.

- void SetHighPriority(TorrentFileInfo fileInfo) - Sets a file as high priority.

- Task EnsureDownloads(TorrentFileInfo[] fileInfos) - Ensures specific files are downloaded.

- Task<TorrentFileCollection> GetFileCollectionAsync() - Retrieves the collection of files in the torrent.

- Task<Stream> InitializeStream(TorrentFileInfo info) - Prepares a stream for a specific file in the torrent.

- void SubscribeMeta(Action<TorrentData> callback) - Subscribes to metadata updates.

- Task StopSession() - Stops the torrent session.

####LibSession

Constructor: LibSession()

Methods:

- LibHandler AddTorrent(string url, string folderPath) - Adds a new torrent to the session.
- void StopTorrent(LibHandler manager) - Stops a specific torrent.
- void Dispose() - Cleans up the session and saves the state.

###Contributing

Contributions are welcome! Feel free to submit issues and pull requests to improve the library.
For developing, developer needs to setup vcpkg with visual studio and install libtorrent. This package is aimed to 
support only the windows versions of the application. 

###License

This project is licensed under the MIT License. See the LICENSE file for details