#include <FileSystem.h>
#include <fstream>
#include <easylogging++.h>
#include <algorithm>

#ifdef _WIN32
	#include <windows.h> // getmodulefilename
    #include <stdio.h>
    #include <psapi.h>
#else

#include <libgen.h> // dirname
#include <sys/resource.h>
#include <cstdio> // for snprintf
#include <string.h> // for strdup(), strlen()
#include <stdlib.h> // free()

#endif

#ifdef _WIN32
    #include <tchar.h>
#else
    #include <sys/types.h>
    #include <unistd.h>
    #include <dirent.h>
#endif


namespace Acidrain {

    DEFINE_string(data_dir, d, "Data dir relative to cwd", "../data")

    FileSystem& FileSystem::getInstance() {
        static FileSystem instance;
        return instance;
    }

    void FileSystem::init(std::string root) {
        LOG(INFO) << "Initializing file system with root dir " << root;
        rootDir = root;
    }

    std::string FileSystem::absolutePath(const char* relativePath) {
        std::string absolutePath = rootDir + "/" + relativePath;
        LOG(TRACE) << "relative path [" << relativePath << "] expanded to [" << absolutePath << "]";
        return absolutePath;
    }

    std::string FileSystem::absolutePath(std::string relativePath) {
        return absolutePath(relativePath.c_str());
    }

#ifdef __APPLE__

	#include <mach-o/dyld.h>
	std::string FileSystem::getExePath() {
		char path[1024];
		uint32_t size = sizeof(path);
		if (_NSGetExecutablePath(path, &size) == 0)
			return std::string(path, size);
		else
			// throw std::exception("_NSGetExecutablePath buffer too small.");
            return std::string("invalid-exe-path");
	}

    std::string FileSystem::getExeDir(std::string fullPath) {
        char* path = strdup(fullPath.c_str());
        char* dname = dirname(path);

        std::string returnValue(dname, strlen(dname));

        free(path);
        return returnValue;
    }

#elif defined _WIN32 || defined _WIN64
	std::string FileSystem::getExePath() {
        char strFileName[1024];
        GetModuleFileName(NULL, strFileName, 1024);

        std::string s = strFileName;
        return strFileName;
	}

	std::string FileSystem::getExeDir(std::string fullPath) {
        size_t n = fullPath.rfind('\\');
        if (n == fullPath.npos) return "";

        fullPath.erase(n, fullPath.length() - n + 1);
        return fullPath;
	}
#elif defined __linux__

    std::string FileSystem::getExePath() {
        static char linkName[100];
        static char fullPath[256];

        pid_t pid = getpid();
        snprintf(linkName, sizeof(linkName), "/proc/%d/exe", pid);

        ssize_t ret = readlink(linkName, fullPath, sizeof(fullPath));
        if (-1 == ret)
            LOG(FATAL) << "readlink() failed while trying to get exe path";

        if (ret >= static_cast<int>(sizeof(fullPath)))
            LOG(FATAL) << "readlink() needs more memory for fullPath";

        return std::string(fullPath, ret);
    }

    std::string FileSystem::getExeDir(std::string fullPath) {
        char* path = strdup(fullPath.c_str());
        char* dname = dirname(path);

        std::string returnValue(dname, strlen(dname));

        free(path);
        return returnValue;
    }

#else
#error "Unknown platform"
#endif

    std::string FileSystem::getFileContent(const std::string& pathRelativeToRoot) {
        return getFileContent(pathRelativeToRoot.c_str());
    }

    std::string FileSystem::getFileContent(const char* pathRelativeToRoot) {
        std::string filename = absolutePath(pathRelativeToRoot);

        std::string contents;
        std::ifstream in(filename, std::ios::in | std::ios::binary);
        if (in) {
            in.seekg(0, std::ios::end);
            contents.resize(in.tellg());
            in.seekg(0, std::ios::beg);
            in.read(&contents[0], contents.size());
            in.close();
        } else {
            LOG(FATAL) << "Failed reading content for file " << filename << ". Errno=" << errno;
        }
        LOG(TRACE) << "Loaded file " << filename << " with size " << contents.size();
        return contents;
    }


    FileSystem::~FileSystem() {
        LOG(INFO) << "Shutting down file system";
    }

    #if defined _WIN32 || defined _WIN64

        vector<FileInfo> FileSystem::filesInDirectory(string directoryURI) {
            vector<FileInfo> result;

            HANDLE dir;
            WIN32_FIND_DATA fileData;
            std::string fileName;
            if ((dir = FindFirstFile(_T((absolutePath(directoryURI) +  "/*").c_str()), &fileData)) == INVALID_HANDLE_VALUE) {
                return result;
            }

            while (FindNextFile(dir, &fileData)) {
                fileName = fileData.cFileName;
                if (fileName !=  "." && fileName != "..") {
                    FileInfo info;
                    info.filename = fileName;
                    info.uri = (directoryURI.empty() ? "" : directoryURI + "/") + info.filename;
                    info.isDirectory = fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
                    result.push_back(info);
                }
            }

            return result;
        }
    #else
        vector<FileInfo> FileSystem::filesInDirectory(string directoryURI) {
            vector<FileInfo> result;

            DIR* dir = opendir(absolutePath(directoryURI).c_str());
            if (dir != nullptr) {
                struct dirent* entry;
                while ((entry = readdir(dir)) != nullptr) {
                    FileInfo info;
                    info.filename = entry->d_name;
                    info.uri = (directoryURI.empty() ? "" : directoryURI + "/") + info.filename;
                    info.isDirectory = entry->d_type == DT_DIR;
                    result.push_back(info);
                }
                closedir(dir);
            }

            return result;
        }
    #endif

    vector<FileInfo> FileSystem::filesInDirectoryRecursive(string directoryURI, vector<FileInfo> filesSoFar) {
        vector<FileInfo> result = filesSoFar;

        vector<FileInfo> filesInThisDir = filesInDirectory(directoryURI);
        for (auto& f : filesInThisDir) {
            if (f.filename == "." || f.filename == "..") {
                continue;
            } else {
                result.push_back(f);
                if (f.isDirectory) {
                    string subdir = directoryURI.empty() ?
                            f.filename :
                            directoryURI + "/" + f.filename;
                    vector<FileInfo> files = filesInDirectoryRecursive(subdir, filesSoFar);
                    copy(files.begin(), files.end(), back_inserter(result));
                }
            }
        }

        return result;

    }
} // namespace Acidrain