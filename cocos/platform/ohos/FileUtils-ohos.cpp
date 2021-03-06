#include "cocos/platform/ohos/FileUtils-ohos.h"
#include "cocos/base/Log.h"
#include "cocos/platform/java/jni/JniHelper.h"
#include <cstdio>
#include <hilog/log.h>
#include <regex>
#include <string>
#include <sys/stat.h>

#include <sys/syscall.h>
#include <unistd.h>

#define ASSETS_FOLDER_NAME "@assets/"

#ifndef JCLS_HELPER
    #define JCLS_HELPER "com/cocos/lib/CocosHelper"
#endif

namespace cc {

ResourceManager *FileUtilsOHOS::_ohosResourceMgr = {};
std::string FileUtilsOHOS::_ohosAssetPath = {};

namespace {

const std::string RAWFILE_PREFIX = "entry/resources/rawfile/";

void PrintRawfiles(ResourceManager *mgr, const std::string &path) {

    auto *file = OpenRawFile(mgr, path.c_str());
    if (file) {
        HILOG_DEBUG(LOG_APP, "PrintRawfile %{public}s", path.c_str());
        return;
    }

    RawDir *dir = OpenRawDir(mgr, path.c_str());
    if (dir) {
        auto fileCnt = GetRawFileCount(dir);
        for (auto i = 0; i < fileCnt; i++) {
            std::string subFile = GetRawFileName(dir, i);
            auto newPath = path + "/" + subFile;
            auto debugPtr = newPath.c_str();
            HILOG_ERROR(LOG_APP, " find path %{public}s", newPath.c_str());
            PrintRawfiles(mgr, newPath);
        }
    } else {
        HILOG_ERROR(LOG_APP, "Invalidate path %{public}s", path.c_str());
    }
}
} // namespace

bool FileUtilsOHOS::initResourceManager(ResourceManager *mgr, const std::string &assetPath) {
    CCASSERT(mgr, "ResourceManager should not be empty!");
    _ohosResourceMgr = mgr;
    if (!assetPath.empty() && assetPath[assetPath.length() - 1] != '/') {
        _ohosAssetPath = assetPath + "/";
    } else {
        _ohosAssetPath = assetPath;
    }
    return true;
}

ResourceManager *FileUtilsOHOS::getResourceManager() {
    return _ohosResourceMgr;
}

FileUtils *FileUtils::getInstance() {
    if (s_sharedFileUtils == nullptr) {
        s_sharedFileUtils = new FileUtilsOHOS();
        if (!s_sharedFileUtils->init()) {
            delete s_sharedFileUtils;
            s_sharedFileUtils = nullptr;
            CC_LOG_DEBUG("ERROR: Could not init CCFileUtilsAndroid");
        }
    }
    return s_sharedFileUtils;
}

//    FileUtilsOHOS::FileUtilsOHOS() {
//    }
//
//    FileUtilsOHOS::~FileUtilsOHOS() noexcept {
//    }

bool FileUtilsOHOS::init() {
    _defaultResRootPath = ASSETS_FOLDER_NAME;
    return FileUtils::init();
}

FileUtils::Status FileUtilsOHOS::getContents(const std::string &filename, ResizableBuffer *buffer) {
    if (filename.empty())
        return FileUtils::Status::NotExists;

    std::string fullPath = fullPathForFilename(filename);
    if (fullPath.empty())
        return FileUtils::Status::NotExists;

    if (fullPath[0] == '/')
        return FileUtils::getContents(fullPath, buffer);

    std::string relativePath;
    size_t position = fullPath.find(ASSETS_FOLDER_NAME);
    if (0 == position) {
        // "@assets/" is at the beginning of the path and we don't want it
        relativePath = RAWFILE_PREFIX + fullPath.substr(strlen(ASSETS_FOLDER_NAME));
    } else {
        relativePath = fullPath;
    }

    if (nullptr == _ohosResourceMgr) {
        HILOG_ERROR(LOG_APP, "... FileUtilsAndroid::assetmanager is nullptr");
        return FileUtils::Status::NotInitialized;
    }

    RawFile *asset = OpenRawFile(_ohosResourceMgr, relativePath.c_str());
    if (nullptr == asset) {
        HILOG_DEBUG(LOG_APP, "asset (%{public}s) is nullptr", filename.c_str());
        return FileUtils::Status::OpenFailed;
    }

    auto size = GetRawFileSize(asset);
    buffer->resize(size);

    int readsize = ReadRawFile(asset, buffer->buffer(), size);
    CloseRawFile(asset);
    // TODO: read error

    if (readsize < size) {
        if (readsize >= 0)
            buffer->resize(readsize);
        return FileUtils::Status::ReadFailed;
    }

    return FileUtils::Status::OK;
}

bool FileUtilsOHOS::isAbsolutePath(const std::string &strPath) const {
    return !strPath.empty() && (strPath[0] == '/' || strPath.find(ASSETS_FOLDER_NAME) == 0);
}

std::string FileUtilsOHOS::getWritablePath() const {
    auto tmp = cc::JniHelper::callStaticStringMethod(JCLS_HELPER, "getWritablePath");
    if (tmp.empty())
        return "./";
    return tmp.append("/");
}

bool FileUtilsOHOS::isFileExistInternal(const std::string &strFilePath) const {
    if (strFilePath.empty()) return false;
    auto filePath = strFilePath;
    auto fileFound = false;

    if (strFilePath[0] == '/') { // absolute path
        struct stat info;
        return ::stat(filePath.c_str(), &info) == 0;
    }

    // relative path
    if (strFilePath.find(_defaultResRootPath) == 0) {
        filePath = RAWFILE_PREFIX + filePath.substr(_defaultResRootPath.length());
    }

    auto rawFile = OpenRawFile(_ohosResourceMgr, filePath.c_str());
    if (rawFile != nullptr) {
        CloseRawFile(rawFile);
        return true;
    }
    return false;
}

bool FileUtilsOHOS::isDirectoryExistInternal(const std::string &dirPath) const {
    if (dirPath.empty()) return false;
    std::string dirPathMf = dirPath[dirPath.length() - 1] == '/' ? dirPath.substr(0, dirPath.length() - 1) : dirPath;

    if (dirPathMf[0] == '/') {
        struct stat st;
        return stat(dirPathMf.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
    }

    if (dirPathMf.find(_defaultResRootPath) == 0) {
        dirPathMf = RAWFILE_PREFIX + dirPathMf.substr(_defaultResRootPath.length(), dirPathMf.length());
    }
    assert(_ohosResourceMgr);
    auto dir = OpenRawDir(_ohosResourceMgr, dirPathMf.c_str());
    if (dir != nullptr) {
        CloseRawDir(dir);
        return true;
    }
    return false;
}

std::string FileUtilsOHOS::expandPath(const std::string &input, bool *isRawFile) const {
    if (!input.empty() && input[0] == '/') {
        if (isRawFile) *isRawFile = false;
        return input;
    }
    const auto fullpath = fullPathForFilename(input);

    if (fullpath.find(_defaultResRootPath) == 0) {
        if (isRawFile) *isRawFile = true;
        return RAWFILE_PREFIX + fullpath.substr(_defaultResRootPath.length(), fullpath.length());
    }

    if (isRawFile) *isRawFile = false;

    return fullpath;
}

std::pair<int, std::function<void()>> FileUtilsOHOS::getFd(const std::string &path) const {
    bool isRawFile = false;
    const auto fullpath = expandPath(path, &isRawFile);
    if (isRawFile) {
        RawFile *rf = OpenRawFile(_ohosResourceMgr, fullpath.c_str());
        // FIXME: try reuse file
        const auto bufSize = GetRawFileSize(rf);
        auto fileCache = std::vector<char>(bufSize);
        auto *buf = fileCache.data();
        // Fill buffer
        const auto readBytes = ReadRawFile(rf, buf, bufSize);
        assert(readBytes == bufSize); // read failure ?
        auto fd = syscall(__NR_memfd_create, fullpath.c_str(), 0);
        {
            auto writeBytes = ::write(fd, buf, bufSize); // Write can fail?
            assert(writeBytes == bufSize);
            ::lseek(fd, 0, SEEK_SET);
        }
        if (errno != 0) {
            const auto *err_msg = strerror(errno);
            CC_LOG_ERROR("failed to open buffer fd %s", err_msg);
        }
        return std::make_pair(fd, [fd]() {
            close(fd);
        });
    } else {
        FILE *fp = fopen(fullpath.c_str(), "rb");
        return std::make_pair(fileno(fp), [fp]() {
            fclose(fp);
        });
    }
}

} // namespace cc