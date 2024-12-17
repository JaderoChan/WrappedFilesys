// The "Wrapped FileSys" library written in c++.
//
// Web: https://github.com/JaderoChan/WrappedFilesys
// You can contact me at: c_dl_cn@outlook.com
//
// MIT License
//
// Copyright (c) 2024 頔珞JaderoChan
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

// Usage:
// 1. The wrapped filesys is a header-only library, so you don't need to build it.
// 2. You can use this library by only including the wrapped_filesys.hpp file in your project.
// 3. You can also make a new .cpp file to your project and add the code below.
// (This method's aim is detach the implementation from the header file, can be avoid namespace pollution)
//
// #define WFS_IMPL
// #include <wrapped_filesys.hpp>
//
// Last you need add the code below to your other location which need use this library.
//
// #define WFS_FWD
// #include <wrapped_filesys.hpp>
//

#ifndef WRAPPED_FILESYS_HPP
#define WRAPPED_FILESYS_HPP

#include <cstddef>  // size_t
#include <string>
#include <vector>
#include <iostream>
#include <sstream>  // stringstream
#include <fstream>
#include <stdexcept>

// Compiler version.
#ifdef _MSVC_LANG
#define _WRAPPED_FILESYS_CPPVERS     _MSVC_LANG
#else
#define _WRAPPED_FILESYS_CPPVERS     __cplusplus
#endif // _MSVC_LANG

#if _WRAPPED_FILESYS_CPPVERS < 201103L
#error "The wrapped_filesys library just useable in c++11 and above."
#endif // _WRAPPED_FILESYS_CPPVERS < 201103L

// Check C++17 support.
#if _WRAPPED_FILESYS_CPPVERS >= 201703L
#define _WRAPPED_FILESYS_CPP17
#endif // WRAPPED_FILESYS_CPPVERS >= 201703L

#ifdef WFS_IMPL
#define WFS_API 
#else
#ifdef WFS_FWD
#define WFS_API extern
#else
#define WFS_API inline
#endif // WFS_FWD
#endif // WFS_IMPL

// Wrapped File System namespace.
namespace wfs
{

// Just for the intellisense better show "tip about namespace". :)

}

// Type alias, enum and constants.
namespace wfs
{

using uchar = unsigned char;
using uint = unsigned int;

template <typename T>
using Vec = std::vector<T>;

using String = std::string;
using Strings = Vec<String>;
using IStream = std::istream;
using OStream = std::ostream;
using FStream = std::fstream;
using IFStream = std::ifstream;
using OFStream = std::ofstream;
using Exception = std::runtime_error;

constexpr uint _BUFFER_SIZE = 4096;

// Preferred path separator.
constexpr char WIN_PATH_SEPARATOR = '\\';
constexpr char LINUX_PATH_SEPARATOR = '/';
#ifdef _WIN32
constexpr char PREFERRED_PATH_SEPARATOR = WIN_PATH_SEPARATOR;
#else
constexpr char PREFERRED_PATH_SEPARATOR = LINUX_PATH_SEPARATOR;
#endif // _WIN32

// The invalid characters in filename.
constexpr const char* FILENAME_INVALID_CHARS = "\\/:*?\"<>|";

} // namespace wfs

// Utility functions with not filesystem.
namespace wfs
{

// @brief Concatenate two paths.
// @note Based on the string operation, not actual file system.
inline String pathcat(const String& path1, const String& path2)
{
    return path1 + PREFERRED_PATH_SEPARATOR + path2;
}

// @brief Concatenate multiple paths.
// @note Based on the string operation, not actual file system.
template <typename... Args>
String pathcat(const String& path1, const String& path2, Args&&... paths)
{
    if (sizeof...(paths) == 0)
        return pathcat(path1, path2);
    else
        return pathcat(pathcat(path1, path2), std::forward<Args>(paths)...);
}

// @brief Check if the filename is valid.
// (not empty, not ".", "..", and not contain invalid characters)
// @note Invalid characters: \/:*?\"<>|
// @note Based on the string operation, not actual file system.
inline bool isValidFilename(const String& filename)
{
    // Filename can't be empty.
    if (filename.empty())
        return false;

    // Filename can't be "." or "..".
    if (filename == "." || filename == "..")
        return false;

    // Filename can't contain invalid characters.
    if (filename.find_first_of(FILENAME_INVALID_CHARS) != String::npos)
        return false;

    return true;
}

// @brief Quote the path with double quotes.
// @note Based on the string operation, not actual file system.
inline String quotePath(const String& path)
{
    return "\"" + path + "\"";
}

template <typename T>
String _fmt(const String& fmt, const T& arg)
{
    std::stringstream ss;

    if (fmt.size() < 4) {
        size_t pos = fmt.find("{}");
        if (pos == String::npos)
            return fmt;

        ss << fmt.substr(0, pos);
        ss << arg;

        return ss.str() + fmt.substr(pos + 2);
    }

    String window(4, '\0');
    for (size_t i = 0; i < fmt.size();) {
        window[0] = fmt[i];
        window[1] = i < fmt.size() - 1 ? fmt[i + 1] : '\0';
        window[2] = i < fmt.size() - 2 ? fmt[i + 2] : '\0';
        window[3] = i < fmt.size() - 3 ? fmt[i + 3] : '\0';

        if (window == "{{}}") {
            ss << "{}";
            i += 4;
            continue;
        }

        if (window[0] == '{' && window[1] == '}') {
            ss << arg;
            return ss.str() + fmt.substr(i + 2);
        } else {
            ss << window[0];
            i += 1;
            continue;
        }
    }

    return ss.str();
}

template <typename T, typename... Args>
String _fmt(const String& fmt, const T& arg, Args&&... args)
{
    std::stringstream ss;

    if (fmt.size() < 4) {
        size_t pos = fmt.find("{}");
        if (pos == String::npos)
            return fmt;

        ss << fmt.substr(0, pos);
        ss << arg;

        return ss.str() + fmt.substr(pos + 2);
    }

    String window(4, '\0');
    for (size_t i = 0; i < fmt.size();) {
        window[0] = fmt[i];
        window[1] = i < fmt.size() - 1 ? fmt[i + 1] : '\0';
        window[2] = i < fmt.size() - 2 ? fmt[i + 2] : '\0';
        window[3] = i < fmt.size() - 3 ? fmt[i + 3] : '\0';

        if (window == "{{}}") {
            ss << "{}";
            i += 4;
            continue;
        }

        if (window[0] == '{' && window[1] == '}') {
            ss << arg;
            return ss.str() + _fmt(fmt.substr(i + 2), std::forward<Args>(args)...);
        } else {
            ss << window[0];
            i += 1;
            continue;
        }
    }

    return ss.str();
}

} // namespace wfs

#ifdef _WRAPPED_FILESYS_CPP17
#ifndef WFS_FWD
#include <filesystem>
namespace wfs
{

namespace fs = std::filesystem;

}
#endif // !WFS_FWD
#else
#ifndef WFS_FWD
#include <ghc/filesystem.hpp>
namespace wfs
{

namespace fs = ghc::filesystem;

}
#endif // !WFS_FWD
#endif // _WRAPPED_FILESYS_CPP17

// Declaration of utility functions with filesystem.
namespace wfs
{

#ifndef WFS_IMPL

// @return The normalized path.
// @note Based on the string operation, not actual file system.
// @example "C:\\\\path\/to/////file.ext" -> "C:/path/to/file.ext"
// @example "C:/path/to/subpath/"         -> "C:/path/to/subpath/"
// @example "C:/path/to/subpath"          -> "C:/path/to/subpath"
WFS_API String normalize(const String& path);

// @return The current path.
WFS_API String currentPath();

// @brief Get the path of the parent directory.
// @note Based on the string operation, not actual file system.
// @example "C:/path/to/file.ext" -> "C:/path/to"
// @example "C:/path/to/"         -> "C:/path/to"
// @example "C:/path/to"          -> "C:/path"
// @example "file.ext"            -> ""
WFS_API String parentPath(const String& path);

// @brief Get the parent directory name.
// @note Based on the string operation, not actual file system.
// @example "C:/path/to/file.ext" -> "to"
// @example "C:/path/to/"         -> "to"
// @example "C:/path/to"          -> "path"
// @example "file.ext"            -> ""
WFS_API String parentName(const String& path);

// @brief Get the filename with extension of the path.
// @note Based on the string operation, not actual file system.
// @example "C:/path/to/file.ext" -> "file.ext"
// @example "C:/path/to/"         -> ""
// @example "C:/path/to"          -> "to"
// @example "file.ext"            -> "file.ext"
// @example ".ext"                -> ".ext"
WFS_API String filenameEx(const String& path);

// @brief Get the filename not with extension of the path.
// @note Based on the string operation, not actual file system.
// @example "C:/path/to/file.ext" -> "file"
// @example "C:/path/to/"         -> ""
// @example "C:/path/to"          -> "to"
// @example "file.ext"            -> "file"
// @example ".ext"                -> ".ext"
WFS_API String filename(const String& path);

// @brief Get the extension of the path.
// @note Based on the string operation, not actual file system.
// @example "C:/path/to/file.ext" -> ".ext"
// @example "C:/path/to/"         -> ""
// @example "C:/path/to"          -> "to"
// @example "file.ext"            -> ".ext"
// @example ".ext"                -> ""
WFS_API String extension(const String& path);

// @brief Check the path if is exists.
WFS_API bool isExists(const String& path);

// @brief Check if the path is a file.
// @return If the path is exists and path target is a regular file return true, else return false.
WFS_API bool isFile(const String& path);

// @brief Check if the path is a directory.
// @return If the path is exists and path target is a directory return true, else return false.
WFS_API bool isDirectory(const String& path);

// @brief Check if the path is a symlink.
// @return If the path is exists and path target is a symlink return true, else return false.
WFS_API bool isSymlink(const String& path);

// @brief Check if the path is a empty file or directory.
// @return If the path is a empty file or directory return true, else return false.
// @note If the path is not exists, throw exception.
WFS_API bool isEmpty(const String& path);

// @brief Check if the path is a sub path of the base path.
// @note Based on the string operation, not actual file system.
// @example "C:/path/to/file.ext", "C:/path"           -> true
// @example "C:/path/to/subpath", "C:/path"            -> true
// @example "C:/path/to/subpath", "C:/path/to"         -> true
// @example "C:/path/to/subpath", "C:/path/to/"        -> true
// @example "C:/path/to/subpath", "C:/path/to/subpath" -> false
// @example "C:/path/to", "C:/path/to/subpath"         -> false
WFS_API bool isSubPath(const String& path, const String& base);

// @brief Check if the path is a relative path.
WFS_API bool isRelative(const String& path);

// @brief Check if the path is a absolute path.
WFS_API bool isAbsolute(const String& path);

// @brief Get the relative path from the base path.
WFS_API String relative(const String& path, const String& base = currentPath());

// @brief Get the absolute path.
WFS_API String absolute(const String& path);

// @brief Check if two paths is equal.
// @example "C:/path/to/file.ext", "C:/path/to/file.ext"     -> true
// @example "./path/to/file.ext", "./path/to/../to/file.ext" -> true
WFS_API bool isEqualPath(const String& path1, const String& path2);

// @brief Check if filesystem entity (file, directory, symlink, hardlink) of two paths is same one.
WFS_API bool isSameFileSystemEntity(const String& path1, const String& path2);

// @return The size of the file or directory.
WFS_API size_t sizes(const String& path);

// @brief Create a directory.
// @return If the directory is existed return false.
// @note The parent directory must exists.
WFS_API bool createDirectory(const String& path);

// @brief Create a directory tree.
// @return If the directory is existed return false.
// @note The parent directory can be not exists (create it automatically).
WFS_API bool createDirectorys(const String& path);

// @brief Delete a file.
// @return If the file is deleted return true, else return false.
WFS_API bool deleteFile(const String& path);

// @brief Recursive delete a file or directory.
// @return The count of the file deleted.
WFS_API size_t deletes(const String& path);

// @brief Copy a file.
WFS_API void copyFile(const String& src, const String& dst, bool isOverwrite = false);

// @brief Copy a file or directory.
WFS_API void copys(const String& src, const String& dst, bool isOverwrite = false);

// @brief Copy a symlink.
WFS_API void copySymlink(const String& src, const String& dst);

// @brief Move a file or directory.
WFS_API void moves(const String& src, const String& dst);

WFS_API void reFilename(const String& path, const String& newFilename);

WFS_API void reFilenameEx(const String& path, const String& newFilenameEx);

WFS_API void reExtension(const String& path, const String& newExtension);

// @brief Create a symlink for the file or directory.
WFS_API void createSymlink(const String& src, const String& dst);

WFS_API String symlinkTarget(const String& path);

WFS_API void createHardlink(const String& src, const String& dst);

WFS_API size_t hardlinkCount(const String& path);

WFS_API String tempDirectory();

// @return The pair of the files and drietorys.
WFS_API std::pair<Strings, Strings>
getAlls(const String& path, bool isRecursive = true, bool (*filter)(const String&) = nullptr);

WFS_API Strings getAllFiles(const String& path, bool isRecursive = true, bool (*filter)(const String&) = nullptr);

WFS_API Strings getAllDirectorys(const String& path, bool isRecursive = true, bool (*filter)(const String&) = nullptr);

#endif // !WFS_IMPL

} // namespace wfs

// Implementation of utility functions with filesystem.
namespace wfs
{

#ifndef WFS_FWD

using _pth = fs::path;

WFS_API String normalize(const String& path)
{
    return _pth(path).lexically_normal().generic_string();
}

WFS_API String currentPath()
{
    return fs::current_path().string();
}

WFS_API String parentPath(const String& path)
{
    return _pth(path).parent_path().string();
}

WFS_API String parentName(const String& path)
{
    return _pth(path).parent_path().filename().string();
}

WFS_API String filenameEx(const String& path)
{
    return _pth(path).filename().string();
}

WFS_API String filename(const String& path)
{
    return _pth(path).filename().replace_extension().string();
}

WFS_API String extension(const String& path)
{
    return _pth(path).extension().string();
}

WFS_API bool isExists(const String& path)
{
    return fs::exists(path);
}

WFS_API bool isFile(const String& path)
{
    return fs::exists(path) && fs::is_regular_file(path);
}

WFS_API bool isDirectory(const String& path)
{
    return fs::exists(path) && fs::is_directory(path);
}

WFS_API bool isSymlink(const String& path)
{
    return fs::is_symlink(path);
}

WFS_API bool isEmpty(const String& path)
{
    return fs::is_empty(path);
}

WFS_API bool isSubPath(const String& path, const String& base)
{
    String _path = normalize(absolute(path));
    String _base = normalize(absolute(base));

    if (_path == _base)
        return false;

    return _path.substr(0, _base.size()) == _base;
}

WFS_API bool isRelative(const String& path)
{
    return _pth(path).is_relative();
}

WFS_API bool isAbsolute(const String& path)
{
    return _pth(path).is_absolute();
}

WFS_API String relative(const String& path, const String& base)
{
    return fs::relative(path, base).string();
}

WFS_API String absolute(const String& path)
{
    return fs::absolute(path).string();
}

WFS_API bool isEqualPath(const String& path1, const String& path2)
{
    String _path1 = normalize(absolute(path1));
    String _path2 = normalize(absolute(path2));

    return _path1 == _path2;
}

WFS_API bool isSameFileSystemEntity(const String& path1, const String& path2)
{
    return fs::equivalent(path1, path2);
}

WFS_API size_t sizes(const String& path)
{
    if (isFile(path)) {
        return fs::file_size(path);
    } else if (isDirectory(path)) {
        size_t rslt = 0;

        for (const auto& var : fs::recursive_directory_iterator(path))
            rslt += (var.is_regular_file() ? var.file_size() : 0);

        return rslt;
    } else {
        throw Exception(_fmt("The specified path not exists. \"{}\"", path));
    }
}

WFS_API bool createDirectory(const String& path)
{
    return fs::create_directory(path);
}

WFS_API bool createDirectorys(const String& path)
{
    return fs::create_directories(path);
}

WFS_API bool deleteFile(const String& path)
{
    return fs::remove(path);
}

WFS_API size_t deletes(const String& path)
{
    return fs::remove_all(path);
}

WFS_API void copyFile(const String& src, const String& dst, bool isOverwrite)
{
    auto copyOptions = isOverwrite ?
                       (fs::copy_options::copy_symlinks |
                        fs::copy_options::create_hard_links |
                        fs::copy_options::create_symlinks |
                        fs::copy_options::overwrite_existing |
                        fs::copy_options::update_existing) :
                       (fs::copy_options::skip_existing |
                        fs::copy_options::skip_symlinks);

    fs::copy_file(src, dst, copyOptions);
}

WFS_API void copys(const String& src, const String& dst, bool isOverwrite)
{
    auto copyOptions = isOverwrite ?
                       (fs::copy_options::copy_symlinks |
                        fs::copy_options::create_hard_links |
                        fs::copy_options::create_symlinks |
                        fs::copy_options::overwrite_existing |
                        fs::copy_options::update_existing) :
                       (fs::copy_options::skip_existing |
                        fs::copy_options::skip_symlinks);
    copyOptions |= fs::copy_options::recursive;

    fs::copy(src, dst, copyOptions);
}

WFS_API void copySymlink(const String& src, const String& dst)
{
    fs::copy_symlink(src, dst);
}

WFS_API void moves(const String& src, const String& dst)
{
    fs::rename(src, dst);
}

WFS_API void reFilename(const String& path, const String& newFilename)
{
    auto dst = pathcat(parentPath(path), newFilename + extension(path));
    moves(path, dst);
}

WFS_API void reFilenameEx(const String& path, const String& newFilenameEx)
{
    auto dst = pathcat(parentPath(path), newFilenameEx);
    moves(path, dst);
}

WFS_API void reExtension(const String& path, const String& newExtension)
{
    auto dst = pathcat(parentPath(path), filename(path) + newExtension);
    moves(path, dst);
}

WFS_API void createSymlink(const String& src, const String& dst)
{
    if (isFile(src))
        fs::create_symlink(src, dst);
    else if (isDirectory(src))
        fs::create_directory_symlink(src, dst);
    else
        throw Exception(_fmt("The specified path not exists. \"{}\"", src));
}

WFS_API String symlinkTarget(const String& path)
{
    return fs::read_symlink(path).string();
}

WFS_API void createHardlink(const String& src, const String& dst)
{
    fs::create_hard_link(src, dst);
}

WFS_API size_t hardlinkCount(const String& path)
{
    return fs::hard_link_count(path);
}

WFS_API String tempDirectory()
{
    return fs::temp_directory_path().string();
}

WFS_API std::pair<Strings, Strings>
getAlls(const String& path, bool isRecursive, bool (*filter)(const String&))
{
    if (!isDirectory(path))
        throw Exception(_fmt("The specified path is not directory or not exists. \"{}\"", path));

    Strings files;
    Strings dirs;

    if (isRecursive) {
        for (const auto& var : fs::recursive_directory_iterator(path)) {
            String _path = var.path().string();

            if (var.is_regular_file() && (!filter || filter(_path)))
                files.push_back(_path);

            if (var.is_directory() && (!filter || filter(_path)))
                dirs.push_back(_path);
        }
    } else {
        for (const auto& var : fs::directory_iterator(path)) {
            String _path = var.path().string();

            if (var.is_regular_file() && (!filter || filter(_path)))
                files.push_back(_path);

            if (var.is_directory() && (!filter || filter(_path)))
                dirs.push_back(_path);
        }
    }

    return { files, dirs };
}

WFS_API Strings getAllFiles(const String& path, bool isRecursive, bool (*filter)(const String&))
{
    if (!isDirectory(path))
        throw Exception(_fmt("The specified path is not directory or not exists. \"{}\"", path));

    Strings files;

    if (isRecursive) {
        for (const auto& var : fs::recursive_directory_iterator(path)) {
            String _path = var.path().string();

            if (var.is_regular_file() && (!filter || filter(_path)))
                files.push_back(_path);
        }
    } else {
        for (const auto& var : fs::directory_iterator(path)) {
            String _path = var.path().string();

            if (var.is_regular_file() && (!filter || filter(_path)))
                files.push_back(_path);
        }
    }

    return files;
}

WFS_API Strings getAllDirectorys(const String& path, bool isRecursive, bool (*filter)(const String&))
{
    if (!isDirectory(path))
        throw Exception(_fmt("The specified path is not directory or not exists. \"{}\"", path));

    Strings dirs;

    if (isRecursive) {
        for (const auto& var : fs::recursive_directory_iterator(path)) {
            String _path = var.path().string();

            if (var.is_directory() && (!filter || filter(_path)))
                dirs.push_back(_path);
        }
    } else {
        for (const auto& var : fs::directory_iterator(path)) {
            String _path = var.path().string();

            if (var.is_directory() && (!filter || filter(_path)))
                dirs.push_back(_path);
        }
    }

    return dirs;
}

#endif // !WFS_FWD

} // namespace wfs

// Classes.
namespace wfs
{

#ifndef WFS_IMPL

class File
{
public:
    File() = default;

    explicit File(const String& name) { setName(name); }

    File(const File& other)
    {
        name_ = other.name_;

        if (other.data_)
            data_ = new String(*other.data_);
    }

    File(File&& other) noexcept
    {
        name_ = other.name_;

        data_ = other.data_;
        other.data_ = nullptr;
    }

    ~File() { releaseData(); }

    static File fromDiskPath(const String& filename)
    {
        IFStream ifs(filename, std::ios_base::binary);

        if (!ifs.is_open())
            throw Exception(_fmt("Failed to open the file: \"{}\"", filename));

        File file(filenameEx(filename));
        file << ifs;

        ifs.close();

        return file;
    }

    File copy() const { return File(*this); }

    String name() const { return name_; }

    String data() const { return data_ ? *data_ : ""; }

    size_t size() const { return data_ ? data_->size() : 0; }

    bool empty() const { return size() == 0; }

    void setName(const String& name) { name_ = name; }

    void releaseData()
    {
        if (data_) {
            delete data_;
            data_ = nullptr;
        }
    }

    void write(OStream& os) const
    {
        if (data_)
            os << *data_;
    }

    void write(const String& path, bool isOverwrite = false,
               std::ios_base::openmode openmode = std::ios_base::binary) const
    {
        String _path = path + PREFERRED_PATH_SEPARATOR + name_;

        if (!isOverwrite && isFile(_path))
            return;

        std::ofstream ofs(_path.data(), openmode);

        if (!ofs.is_open())
            throw Exception(_fmt("Failed to open the file: \"{}\"", _path));

        write(ofs);

        ofs.close();
    }

    File& operator=(const File& other)
    {
        name_ = other.name_;

        releaseData();

        if (other.data_)
            data_ = new String(*other.data_);

        return *this;
    }

    File& operator=(const String& data)
    {
        releaseData();

        data_ = new String(data);

        return *this;
    }

    template <typename T>
    File& operator=(const Vec<T>& data)
    {
        releaseData();

        data_ = new String;
        data_->reserve(data_->size() + data.size());

        for (const auto& var : data)
            data_->push_back(var);

        return *this;
    }

    File& operator<<(const File& other)
    {
        if (!data_)
            data_ = new String;
        data_->append(other.data());

        return *this;
    }

    File& operator<<(IStream& is)
    {
        is.seekg(0, std::ios_base::end);
        size_t size = is.tellg();
        is.seekg(0, std::ios_base::beg);

        if (!data_)
            data_ = new String();
        data_->reserve(data_->size() + size);

        char buffer[_BUFFER_SIZE] = {};
        while (is.read(buffer, _BUFFER_SIZE))
            data_->append(String(buffer, is.gcount()));

        data_->append(String(buffer, is.gcount()));

        return *this;
    }

    File& operator<<(const String& data)
    {
        if (!data_)
            data_ = new String();
        data_->append(data);

        return *this;
    }

    template <typename T>
    File& operator<<(const Vec<T>& data)
    {
        size_t size = data.size();

        if (!data_)
            data_ = new String;
        data_->reserve(data_->size() + size);

        for (const auto& var : data)
            data_->push_back(var);

        return *this;
    }

    const File& operator>>(OStream& os) const
    {
        write(os);
        return *this;
    }

private:
    String name_;
    String* data_ = nullptr;
};

class Dir
{
public:
    Dir() = default;

    explicit Dir(const String& name) { setName(name); }

    Dir(const Dir& other)
    {
        name_ = other.name_;

        if (other.subFiles_)
            subFiles_ = new Vec<File>(*other.subFiles_);

        if (other.subDirs_)
            subDirs_ = new Vec<Dir>(*other.subDirs_);
    }

    Dir(Dir&& other) noexcept
    {
        name_ = other.name_;

        subFiles_ = other.subFiles_;
        other.subFiles_ = nullptr;

        subDirs_ = other.subDirs_;
        other.subDirs_ = nullptr;
    }

    ~Dir() { clear(); }

    static Dir fromDiskPath(const String& dirpath)
    {
        Dir root(filenameEx(dirpath));

        auto dirs = getAllDirectorys(dirpath, false);
        for (const auto& var : dirs)
            root << Dir::fromDiskPath(var);

        auto files = getAllFiles(dirpath, false);
        for (const auto& var : files)
            root << File::fromDiskPath(var);

        return root;
    }

    String name() const { return name_; }

    size_t size() const
    {
        size_t size = 0;

        if (subFiles_)
            for (const auto& var : *subFiles_)
                size += var.size();

        if (subDirs_)
            for (const auto& var : *subDirs_)
                size += var.size();

        return size;
    }

    size_t fileCount(bool isRecursive = true) const
    {
        size_t cnt = 0;

        if (subFiles_)
            cnt += subFiles_->size();

        if (isRecursive && subDirs_)
            for (const auto& var : *subDirs_)
                cnt += var.fileCount();

        return cnt;
    }

    size_t dirCount(bool isRecursive = true) const
    {
        size_t cnt = 0;

        if (subDirs_)
            cnt += subDirs_->size();

        if (isRecursive && subDirs_)
            for (const auto& var : *subDirs_)
                cnt += var.dirCount();

        return cnt;
    }

    size_t count(bool isRecursive = true) const { return fileCount(isRecursive) + dirCount(isRecursive); }

    bool empty() const { return size() == 0; }

    bool hasFile(const String& name, bool isRecursive = false) const
    {
        if (hasFile_(name) != NOF_)
            return true;

        if (isRecursive && subDirs_) {
            for (const auto& var : *subDirs_) {
                if (var.hasFile(name, true))
                    return true;
            }
        }

        return false;
    }

    bool hasDir(const String& name, bool isRecursive = false) const
    {
        if (hasDir_(name) != NOF_)
            return true;

        if (isRecursive && subDirs_) {
            for (const auto& var : *subDirs_) {
                if (var.hasDir(name, true))
                    return true;
            }
        }

        return false;
    }

    void setName(const String& name)
    {
        if (!isValidFilename(name))
            throw Exception(_fmt("Invalid file name: \"{}\"", name));

        name_ = name;
    }

    const Vec<File>& files() const { return *subFiles_; }

    const Vec<Dir>& dirs() const { return *subDirs_; }

    Vec<File>& files() { return *subFiles_; }

    Vec<Dir>& dirs() { return *subDirs_; }

    File& file(const String& name)
    {
        size_t pos = hasFile_(name);

        if (pos == NOF_) {
            add(File(name));
            return subFiles_->back();
        }

        return (*subFiles_)[pos];
    }

    Dir& dir(const String& name)
    {
        size_t pos = hasDir_(name);

        if (pos == NOF_) {
            add(Dir(name));
            return subDirs_->back();
        }

        return (*subDirs_)[pos];
    }

    void removeFile(const String& name)
    {
        size_t pos = hasFile_(name);

        if (pos == NOF_)
            return;

        subFiles_->erase(subFiles_->begin() + pos);
    }

    void removeDir(const String& name)
    {
        size_t pos = hasDir_(name);

        if (pos == NOF_)
            return;

        subDirs_->erase(subDirs_->begin() + pos);
    }

    void releaseAllFilesData()
    {
        if (subFiles_)
            for (auto& var : *subFiles_)
                var.releaseData();

        if (subDirs_)
            for (auto& var : *subDirs_)
                var.releaseAllFilesData();
    }

    void clearFiles()
    {
        if (subFiles_) {
            delete subFiles_;
            subFiles_ = nullptr;
        }
    }

    void clearDirs()
    {
        if (subDirs_) {
            delete subDirs_;
            subDirs_ = nullptr;
        }
    }

    void clear()
    {
        clearFiles();
        clearDirs();
    }

    void add(File& file, bool isOverwrite = false)
    {
        if (!subFiles_)
            subFiles_ = new Vec<File>();

        size_t pos = hasFile_(file.name());

        if (pos != NOF_) {
            if (isOverwrite)
                (*subFiles_)[pos] = std::move(file);

            return;
        }

        subFiles_->emplace_back(std::move(file));
    }

    void add(Dir& dir, bool isOverwrite = false)
    {
        if (!subDirs_)
            subDirs_ = new Vec<Dir>();

        size_t pos = hasDir_(dir.name());

        if (pos != NOF_) {
            if (isOverwrite)
                (*subDirs_)[pos] = std::move(dir);

            return;
        }

        subDirs_->emplace_back(std::move(dir));
    }

    void add(File&& file, bool isOverwrite = false) { add(file, isOverwrite); }

    void add(Dir&& dir, bool isOverwrite = false) { add(dir, isOverwrite); }

    void write(const String& path, bool isOverwrite = false,
               std::ios_base::openmode openmode = std::ios_base::binary) const
    {
        String root = String(path) + PREFERRED_PATH_SEPARATOR + name_;

        createDirectory(root);

        if (subFiles_)
            for (const auto& var : *subFiles_)
                var.write(root, isOverwrite, openmode);

        if (subDirs_)
            for (const auto& var : *subDirs_)
                var.write(root, isOverwrite);
    }

    Dir copy() const { return Dir(*this); }

    Dir& operator=(const Dir& other)
    {
        name_ = other.name_;

        clear();

        if (other.subFiles_)
            subFiles_ = new Vec<File>(*other.subFiles_);

        if (other.subDirs_)
            subDirs_ = new Vec<Dir>(*other.subDirs_);

        return *this;
    }

    Dir& operator[](const String& name) { return dir(name); }

    File& operator()(const String& name) { return file(name); }

    Dir& operator<<(File& file)
    {
        add(file);
        return *this;
    }

    Dir& operator<<(Dir& dir)
    {
        add(dir);
        return *this;
    }

    Dir& operator<<(File&& file)
    {
        add(file);
        return *this;
    }

    Dir& operator<<(Dir&& dir)
    {
        add(dir);
        return *this;
    }

private:
    static constexpr size_t NOF_ = size_t(-1);

    size_t hasFile_(const String& name) const
    {
        if (subFiles_) {
            for (size_t i = 0; i < subFiles_->size(); ++i) {
                if ((*subFiles_)[i].name() == name)
                    return i;
            }
        }

        return NOF_;
    }

    size_t hasDir_(const String& name) const
    {
        if (subDirs_) {
            for (size_t i = 0; i < subDirs_->size(); ++i) {
                if ((*subDirs_)[i].name() == name)
                    return i;
            }
        }

        return NOF_;
    }

    String name_;
    Vec<File>* subFiles_ = nullptr;
    Vec<Dir>* subDirs_ = nullptr;
};

#endif // !WFS_IMPL

} // namespace wfs

#endif // !WRAPPED_FILESYS_HPP
