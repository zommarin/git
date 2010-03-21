#include "../git-compat-util.h"
#include "win32.h"
#include <conio.h>
#include <winioctl.h>
#include <wchar.h>
#include "../strbuf.h"
#include "../cache.h"
#include "../run-command.h"

#define IS_REPARSE_POINT(find_file_data) \
	((find_file_data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) && \
	(find_file_data.dwReserved0 == IO_REPARSE_TAG_SYMLINK))

unsigned int _CRT_fmode = _O_BINARY;
static const int delay[] = { 0, 1, 10, 20, 40 };

int err_win_to_posix(DWORD winerr)
{
	int error = ENOSYS;
	switch(winerr) {
	case ERROR_ACCESS_DENIED: error = EACCES; break;
	case ERROR_ACCOUNT_DISABLED: error = EACCES; break;
	case ERROR_ACCOUNT_RESTRICTION: error = EACCES; break;
	case ERROR_ALREADY_ASSIGNED: error = EBUSY; break;
	case ERROR_ALREADY_EXISTS: error = EEXIST; break;
	case ERROR_ARITHMETIC_OVERFLOW: error = ERANGE; break;
	case ERROR_BAD_COMMAND: error = EIO; break;
	case ERROR_BAD_DEVICE: error = ENODEV; break;
	case ERROR_BAD_DRIVER_LEVEL: error = ENXIO; break;
	case ERROR_BAD_EXE_FORMAT: error = ENOEXEC; break;
	case ERROR_BAD_FORMAT: error = ENOEXEC; break;
	case ERROR_BAD_LENGTH: error = EINVAL; break;
	case ERROR_BAD_PATHNAME: error = ENOENT; break;
	case ERROR_BAD_PIPE: error = EPIPE; break;
	case ERROR_BAD_UNIT: error = ENODEV; break;
	case ERROR_BAD_USERNAME: error = EINVAL; break;
	case ERROR_BROKEN_PIPE: error = EPIPE; break;
	case ERROR_BUFFER_OVERFLOW: error = ENAMETOOLONG; break;
	case ERROR_BUSY: error = EBUSY; break;
	case ERROR_BUSY_DRIVE: error = EBUSY; break;
	case ERROR_CALL_NOT_IMPLEMENTED: error = ENOSYS; break;
	case ERROR_CANNOT_MAKE: error = EACCES; break;
	case ERROR_CANTOPEN: error = EIO; break;
	case ERROR_CANTREAD: error = EIO; break;
	case ERROR_CANTWRITE: error = EIO; break;
	case ERROR_CRC: error = EIO; break;
	case ERROR_CURRENT_DIRECTORY: error = EACCES; break;
	case ERROR_DEVICE_IN_USE: error = EBUSY; break;
	case ERROR_DEV_NOT_EXIST: error = ENODEV; break;
	case ERROR_DIRECTORY: error = EINVAL; break;
	case ERROR_DIR_NOT_EMPTY: error = ENOTEMPTY; break;
	case ERROR_DISK_CHANGE: error = EIO; break;
	case ERROR_DISK_FULL: error = ENOSPC; break;
	case ERROR_DRIVE_LOCKED: error = EBUSY; break;
	case ERROR_ENVVAR_NOT_FOUND: error = EINVAL; break;
	case ERROR_EXE_MARKED_INVALID: error = ENOEXEC; break;
	case ERROR_FILENAME_EXCED_RANGE: error = ENAMETOOLONG; break;
	case ERROR_FILE_EXISTS: error = EEXIST; break;
	case ERROR_FILE_INVALID: error = ENODEV; break;
	case ERROR_FILE_NOT_FOUND: error = ENOENT; break;
	case ERROR_GEN_FAILURE: error = EIO; break;
	case ERROR_HANDLE_DISK_FULL: error = ENOSPC; break;
	case ERROR_INSUFFICIENT_BUFFER: error = ENOMEM; break;
	case ERROR_INVALID_ACCESS: error = EACCES; break;
	case ERROR_INVALID_ADDRESS: error = EFAULT; break;
	case ERROR_INVALID_BLOCK: error = EFAULT; break;
	case ERROR_INVALID_DATA: error = EINVAL; break;
	case ERROR_INVALID_DRIVE: error = ENODEV; break;
	case ERROR_INVALID_EXE_SIGNATURE: error = ENOEXEC; break;
	case ERROR_INVALID_FLAGS: error = EINVAL; break;
	case ERROR_INVALID_FUNCTION: error = ENOSYS; break;
	case ERROR_INVALID_HANDLE: error = EBADF; break;
	case ERROR_INVALID_LOGON_HOURS: error = EACCES; break;
	case ERROR_INVALID_NAME: error = EINVAL; break;
	case ERROR_INVALID_OWNER: error = EINVAL; break;
	case ERROR_INVALID_PARAMETER: error = EINVAL; break;
	case ERROR_INVALID_PASSWORD: error = EPERM; break;
	case ERROR_INVALID_PRIMARY_GROUP: error = EINVAL; break;
	case ERROR_INVALID_SIGNAL_NUMBER: error = EINVAL; break;
	case ERROR_INVALID_TARGET_HANDLE: error = EIO; break;
	case ERROR_INVALID_WORKSTATION: error = EACCES; break;
	case ERROR_IO_DEVICE: error = EIO; break;
	case ERROR_IO_INCOMPLETE: error = EINTR; break;
	case ERROR_LOCKED: error = EBUSY; break;
	case ERROR_LOCK_VIOLATION: error = EACCES; break;
	case ERROR_LOGON_FAILURE: error = EACCES; break;
	case ERROR_MAPPED_ALIGNMENT: error = EINVAL; break;
	case ERROR_META_EXPANSION_TOO_LONG: error = E2BIG; break;
	case ERROR_MORE_DATA: error = EPIPE; break;
	case ERROR_NEGATIVE_SEEK: error = ESPIPE; break;
	case ERROR_NOACCESS: error = EFAULT; break;
	case ERROR_NONE_MAPPED: error = EINVAL; break;
	case ERROR_NOT_ENOUGH_MEMORY: error = ENOMEM; break;
	case ERROR_NOT_READY: error = EAGAIN; break;
	case ERROR_NOT_SAME_DEVICE: error = EXDEV; break;
	case ERROR_NO_DATA: error = EPIPE; break;
	case ERROR_NO_MORE_SEARCH_HANDLES: error = EIO; break;
	case ERROR_NO_PROC_SLOTS: error = EAGAIN; break;
	case ERROR_NO_SUCH_PRIVILEGE: error = EACCES; break;
	case ERROR_OPEN_FAILED: error = EIO; break;
	case ERROR_OPEN_FILES: error = EBUSY; break;
	case ERROR_OPERATION_ABORTED: error = EINTR; break;
	case ERROR_OUTOFMEMORY: error = ENOMEM; break;
	case ERROR_PASSWORD_EXPIRED: error = EACCES; break;
	case ERROR_PATH_BUSY: error = EBUSY; break;
	case ERROR_PATH_NOT_FOUND: error = ENOENT; break;
	case ERROR_PIPE_BUSY: error = EBUSY; break;
	case ERROR_PIPE_CONNECTED: error = EPIPE; break;
	case ERROR_PIPE_LISTENING: error = EPIPE; break;
	case ERROR_PIPE_NOT_CONNECTED: error = EPIPE; break;
	case ERROR_PRIVILEGE_NOT_HELD: error = EACCES; break;
	case ERROR_READ_FAULT: error = EIO; break;
	case ERROR_SEEK: error = EIO; break;
	case ERROR_SEEK_ON_DEVICE: error = ESPIPE; break;
	case ERROR_SHARING_BUFFER_EXCEEDED: error = ENFILE; break;
	case ERROR_SHARING_VIOLATION: error = EACCES; break;
	case ERROR_STACK_OVERFLOW: error = ENOMEM; break;
	case ERROR_SWAPERROR: error = ENOENT; break;
	case ERROR_TOO_MANY_MODULES: error = EMFILE; break;
	case ERROR_TOO_MANY_OPEN_FILES: error = EMFILE; break;
	case ERROR_UNRECOGNIZED_MEDIA: error = ENXIO; break;
	case ERROR_UNRECOGNIZED_VOLUME: error = ENODEV; break;
	case ERROR_WAIT_NO_CHILDREN: error = ECHILD; break;
	case ERROR_WRITE_FAULT: error = EIO; break;
	case ERROR_WRITE_PROTECT: error = EROFS; break;
	}
	return error;
}

/*
 * Converts the specified zero-terminated utf8 path to wide and returns a
 * static buffer with room for 4 additional bytes.
 *
 * path must be at most PATH_MAX bytes long.
 *
 * Returns NULL on failure.
 */
static inline wchar_t *utf8_to_wbuf(const char *path)
{
	/* Add room for some extra characters, as there are path
	   manipulations done in e.g. opendir(). */
	static wchar_t buffer[4][PATH_MAX+4];
	static int counter = 0;
	int result;

	if (!path)
		return NULL;

	if (++counter >= ARRAY_SIZE(buffer))
		counter = 0;

	result = MultiByteToWideChar(CP_UTF8, 0, path, -1, buffer[counter], PATH_MAX);
	if (result >= 0)
		return buffer[counter];

	error("could not convert utf8 path '%s' to wide string: %s", path,
	      result == ERROR_INSUFFICIENT_BUFFER ? "name too long" :
	      result == ERROR_NO_UNICODE_TRANSLATION ? "unicode lacking" :
	          "flags/parameter error");

	return NULL;
}

/*
 * Converts the specified zero-terminated utf8 string to wide.
 *
 * plength is a pointer to a variable that receives the size of the returned
 * wide string in characters.
 *
 * Returns NULL on failure. Transfers ownership of the string to the caller.
 */
static inline wchar_t *utf8_to_wchar_auto(const char *string, int *plength)
{
	int chars;
	wchar_t *wide_string;

	chars = MultiByteToWideChar(CP_UTF8, 0, string, -1, NULL, 0);
	if (!chars)
		return NULL;

	wide_string = xcalloc(chars, sizeof(wchar_t));
	chars = MultiByteToWideChar(CP_UTF8, 0, string, -1, wide_string, chars);
	if (!chars) {
		free(wide_string);
		return NULL;
	}

	if (plength)
		*plength = chars;

	return wide_string;
}

/* wcstombs-like utf8-to-wide conversion */
size_t wchar_to_utf8(char *dst, const wchar_t *src, size_t length)
{
	int byteCount;

	if (!src)
		return -1;

	/* wcstombs indicates a length-check with !dst,
	   WideCharToMultiByte uses !length */
	if (!dst)
		length = 0;

	byteCount = WideCharToMultiByte(CP_UTF8, 0, src, -1, dst, length, NULL, NULL);
	if (byteCount > 0) {
		/* WideCharToMultiByte returns size with zero-terminator.
		   wcstombs returns size without zero-terminator. */
		if (!length)
			return byteCount - 1;

		if (dst[byteCount - 1] == 0)
			byteCount--;
		else if (byteCount == length)
			dst[byteCount - 1] = 0;
		else
			dst[byteCount] = 0;

		return byteCount;
	}

	error("could not convert wide string '%ls' to utf8: %s", src,
	      byteCount == ERROR_INSUFFICIENT_BUFFER ? "name too long" :
	      byteCount == ERROR_NO_UNICODE_TRANSLATION ? "unicode lacking" :
	          "flags/parameter error");

	return -1;
}

/*
 * Converts the specified zero-terminated wide string to utf8.
 *
 * plength is a pointer to a variable that receives the size of the returned
 * utf8 string in bytes.
 *
 * Returns NULL on failure. Transfers ownership of the string to the caller.
 */
static char *wchar_to_utf8_auto(const wchar_t *string, size_t *plength)
{
	char *utf8_string;
	size_t bytes;

	bytes = wchar_to_utf8(NULL, string, 0);
	if (bytes == -1)
		return NULL;

	utf8_string = xmallocz(bytes);
	if (wchar_to_utf8(utf8_string, string, bytes + 1) == -1) {
		free(utf8_string);
		return NULL;
	}

	if (plength)
		*plength = bytes;

	return utf8_string;
}

static int make_hidden(const wchar_t *path)
{
	DWORD attribs = GetFileAttributesW(path);
	if (SetFileAttributesW(path, FILE_ATTRIBUTE_HIDDEN | attribs))
		return 0;
	errno = err_win_to_posix(GetLastError());
	return -1;
}

void mingw_mark_as_git_dir(const char *dir)
{
	if (hide_dotfiles != HIDE_DOTFILES_FALSE && !is_bare_repository() &&
	    make_hidden(utf8_to_wbuf(dir)))
		warning("Failed to make '%s' hidden", dir);
	git_config_set("core.hideDotFiles",
		hide_dotfiles == HIDE_DOTFILES_FALSE ? "false" :
		(hide_dotfiles == HIDE_DOTFILES_DOTGITONLY ?
		 "dotGitOnly" : "true"));
}

int mingw_access(const char *filename, int mode)
{
	/* Ignore X_OK because it does not apply to Windows, and Vista and later
	   return an error. */
	return _waccess(utf8_to_wbuf(filename), mode & ~X_OK);
}

int mingw_chmod(const char *path, mode_t mode)
{
	return _wchmod(utf8_to_wbuf(path), mode);
}

int mingw_chdir(const char *path)
{
	return _wchdir(utf8_to_wbuf(path));
}

#undef mkdir
int mingw_mkdir(const char *path, int mode)
{
	const wchar_t *wpath = utf8_to_wbuf(path);
	int ret = _wmkdir(wpath);
	if (!ret && hide_dotfiles == HIDE_DOTFILES_TRUE) {
		/*
		 * In Windows a file or dir starting with a dot is not
		 * automatically hidden. So lets mark it as hidden when
		 * such a directory is created.
		 */
		const char *start = basename((char*)path);
		if (*start == '.')
			return make_hidden(wpath);
	}
	return ret;
}

static int ask_user_yes_no(const char *format, ...)
{
	char answer[5];
	char question[4096];
	const char *retry_hook[] = { NULL, NULL, NULL };
	va_list args;

	if ((retry_hook[0] = getenv("GIT_ASK_YESNO"))) {

		va_start(args, format);
		vsnprintf(question, sizeof(question), format, args);
		va_end(args);

		retry_hook[1] = question;
		return !run_command_v_opt(retry_hook, 0);
	}

	if (!isatty(_fileno(stdin)))
		return 0;

	while (1) {
		va_start(args, format);
		vfprintf(stderr, format, args);
		va_end(args);
		fprintf(stderr, " (y/n)? ");

		if (fgets(answer, sizeof(answer), stdin)) {
			/* remove the newline */
			if (answer[strlen(answer)-2] == '\r')
				answer[strlen(answer)-2] = '\0';
			if (answer[strlen(answer)-1] == '\n')
				answer[strlen(answer)-1] = '\0';
		} else
			return 0;

		if (answer[0] == 'y' && strlen(answer) == 1)
			return 1;
		if (!strncasecmp(answer, "yes", sizeof(answer)))
			return 1;
		if (answer[0] == 'n' && strlen(answer) == 1)
			return 0;
		if (!strncasecmp(answer, "no", sizeof(answer)))
			return 0;
		fprintf(stderr, "I did not understand your answer: '%s'\n",
				answer);
	}
}

#undef unlink
int mingw_unlink(const char *pathname)
{
	int ret, tries = 0;
	const wchar_t *wpathname = utf8_to_wbuf(pathname);

	/* read-only files cannot be removed */
	chmod(pathname, 0666);
	while ((ret = _wunlink(wpathname)) == -1 && tries < ARRAY_SIZE(delay)) {
		if (errno != EACCES)
			break;
		/*
		 * We assume that some other process had the source or
		 * destination file open at the wrong moment and retry.
		 * In order to give the other process a higher chance to
		 * complete its operation, we give up our time slice now.
		 * If we have to retry again, we do sleep a bit.
		 */
		Sleep(delay[tries]);
		tries++;
	}
	while (ret == -1 && errno == EACCES &&
	       ask_user_yes_no("Unlink of file '%s' failed. "
	       "Should I try again?", pathname))
		ret = _wunlink(wpathname);
	return ret;
}

#undef rmdir
int mingw_rmdir(const char *pathname)
{
	const wchar_t *wpathname = utf8_to_wbuf(pathname);
	int ret, tries = 0;

	while ((ret = _wmkdir(wpathname)) == -1 && tries < ARRAY_SIZE(delay)) {
		if (errno != EACCES)
			break;
		/*
		 * We assume that some other process had the source or
		 * destination file open at the wrong moment and retry.
		 * In order to give the other process a higher chance to
		 * complete its operation, we give up our time slice now.
		 * If we have to retry again, we do sleep a bit.
		 */
		Sleep(delay[tries]);
		tries++;
	}
	while (ret == -1 && errno == EACCES &&
	       ask_user_yes_no("Deletion of directory '%s' failed. "
	       "Should I try again?", pathname))
		ret = _wmkdir(wpathname);
	return ret;
}

#undef open
int mingw_open (const char *filename, int oflags, ...)
{
	const wchar_t *wfilename;
	va_list args;
	unsigned mode;
	int fd;

	va_start(args, oflags);
	mode = va_arg(args, int);
	va_end(args);

	if (!strcmp(filename, "/dev/null"))
		filename = "nul";

	wfilename = utf8_to_wbuf(filename);
	fd = _wopen(wfilename, oflags | O_BINARY, mode);

	if (fd < 0 && (oflags & O_CREAT) && errno == EACCES) {
		DWORD attrs = GetFileAttributesW(wfilename);
		if (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY))
			errno = EISDIR;
	}
	if ((oflags & O_CREAT) && fd >= 0 &&
	    hide_dotfiles == HIDE_DOTFILES_TRUE) {
		/*
		 * In Windows a file or dir starting with a dot is not
		 * automatically hidden. So lets mark it as hidden when
		 * such a file is created.
		 */
		const char *start = basename((char*)filename);
		if (*start == '.' && make_hidden(wfilename))
			warning("Could not mark '%s' as hidden.", filename);
	}
	return fd;
}

#undef fopen
FILE *mingw_fopen (const char *filename, const char *mode)
{
	const wchar_t *wfilename;
	int hide = 0;
	FILE *file;
	if (hide_dotfiles == HIDE_DOTFILES_TRUE &&
	    basename((char*)filename)[0] == '.')
		hide = access(filename, F_OK);

	wfilename = strcmp(filename, "/dev/null") ? utf8_to_wbuf(filename) : L"NUL";
	file = _wfopen(wfilename, utf8_to_wbuf(mode));

	/*
	 * In Windows a file or dir starting with a dot is not
	 * automatically hidden. So lets mark it as hidden when
	 * such a file is created.
	 */
	if (file && hide && make_hidden(wfilename))
		warning("Could not mark '%s' as hidden.", filename);
	return file;
}

#undef freopen
FILE *mingw_freopen(const char *path, const char *mode, FILE *stream)
{
	const wchar_t *wpath;
	wpath = strcmp(path, "/dev/null") ? utf8_to_wbuf(path) : L"NUL";
	return _wfreopen(wpath, utf8_to_wbuf(mode), stream);
}

/*
 * The unit of FILETIME is 100-nanoseconds since January 1, 1601, UTC.
 * Returns the 100-nanoseconds ("hekto nanoseconds") since the epoch.
 */
static inline long long filetime_to_hnsec(const FILETIME *ft)
{
	long long winTime = ((long long)ft->dwHighDateTime << 32) + ft->dwLowDateTime;
	/* Windows to Unix Epoch conversion */
	return winTime - 116444736000000000LL;
}

static inline time_t filetime_to_time_t(const FILETIME *ft)
{
	return (time_t)(filetime_to_hnsec(ft) / 10000000);
}

/*
 * We keep the do_lstat code in a separate function to avoid recursion.
 * When a path ends with a slash, the stat will fail with ENOENT. In
 * this case, we strip the trailing slashes and stat again.
 *
 * If follow is true then act like stat() and report on the link
 * target. Otherwise report on the link itself.
 */
static int do_lstat(int follow, const wchar_t *file_name, struct stat *buf)
{
	WIN32_FIND_DATAW fdata;
	HANDLE handle = FindFirstFileW(file_name, &fdata);

	if (handle == INVALID_HANDLE_VALUE) {
		errno = err_win_to_posix(GetLastError());
		return -1;
	}

	FindClose(handle);

	buf->st_ino = 0;
	buf->st_gid = 0;
	buf->st_uid = 0;
	buf->st_nlink = 1;
	buf->st_mode = file_attr_to_st_mode(fdata.dwFileAttributes);
	buf->st_size = fdata.nFileSizeLow |
		(((off_t)fdata.nFileSizeHigh)<<32);
	buf->st_dev = buf->st_rdev = 0; /* not used by Git */
	buf->st_atime = filetime_to_time_t(&(fdata.ftLastAccessTime));
	buf->st_mtime = filetime_to_time_t(&(fdata.ftLastWriteTime));
	buf->st_ctime = filetime_to_time_t(&(fdata.ftCreationTime));

	if (IS_REPARSE_POINT(fdata)) {
		if (follow)
			buf->st_size = wreadlink(file_name, NULL, 0);
		else
			buf->st_mode = S_IFLNK;
		buf->st_mode |= S_IREAD;
		if (!(fdata.dwFileAttributes & FILE_ATTRIBUTE_READONLY))
			buf->st_mode |= S_IWRITE;
	}

	return 0;
}

/*
 * We provide our own lstat/fstat functions, since the provided
 * lstat/fstat functions are so slow. These stat functions are
 * tailored for Git's usage (read: fast), and are not meant to be
 * complete. Note that Git stat()s are redirected to mingw_lstat()
 * too, since Windows doesn't really handle symlinks that well.
 */
static int do_stat_internal(int follow, const char *file_name, struct stat *buf)
{
	int namelen;
	wchar_t *wfilename = utf8_to_wbuf(file_name);

	if (!do_lstat(follow, wfilename, buf))
		return 0;

	/*
	 * if file_name ended in a '/', Windows returned ENOENT;
	 * try again without trailing slashes
	 */
	if (errno != ENOENT)
		return -1;

	namelen = wcslen(wfilename);
	if (namelen && wfilename[namelen-1] != '/')
		return -1;
	while (namelen && wfilename[namelen-1] == '/')
		--namelen;
	if (!namelen || namelen >= PATH_MAX)
		return -1;

	wfilename[namelen] = 0;
	return do_lstat(follow, wfilename, buf);
}

int mingw_lstat(const char *file_name, struct stat *buf)
{
	return do_stat_internal(0, file_name, buf);
}
int mingw_stat(const char *file_name, struct stat *buf)
{
	return do_stat_internal(1, file_name, buf);
}

#undef fstat
int mingw_fstat(int fd, struct stat *buf)
{
	HANDLE fh = (HANDLE)_get_osfhandle(fd);
	BY_HANDLE_FILE_INFORMATION fdata;

	if (fh == INVALID_HANDLE_VALUE) {
		errno = EBADF;
		return -1;
	}
	/* direct non-file handles to MS's fstat() */
	if (GetFileType(fh) != FILE_TYPE_DISK)
		return _fstati64(fd, buf);

	if (GetFileInformationByHandle(fh, &fdata)) {
		buf->st_ino = 0;
		buf->st_gid = 0;
		buf->st_uid = 0;
		buf->st_nlink = 1;
		buf->st_mode = file_attr_to_st_mode(fdata.dwFileAttributes);
		buf->st_size = fdata.nFileSizeLow |
			(((off_t)fdata.nFileSizeHigh)<<32);
		buf->st_dev = buf->st_rdev = 0; /* not used by Git */
		buf->st_atime = filetime_to_time_t(&(fdata.ftLastAccessTime));
		buf->st_mtime = filetime_to_time_t(&(fdata.ftLastWriteTime));
		buf->st_ctime = filetime_to_time_t(&(fdata.ftCreationTime));
		return 0;
	}
	errno = EBADF;
	return -1;
}

static inline void time_t_to_filetime(time_t t, FILETIME *ft)
{
	long long winTime = t * 10000000LL + 116444736000000000LL;
	ft->dwLowDateTime = winTime;
	ft->dwHighDateTime = winTime >> 32;
}

int mingw_utime (const char *file_name, const struct utimbuf *times)
{
	FILETIME mft, aft;
	int fh, rc;

	/* must have write permission */
	if ((fh = _wopen(utf8_to_wbuf(file_name), O_RDWR | O_BINARY)) < 0)
		return -1;

	time_t_to_filetime(times->modtime, &mft);
	time_t_to_filetime(times->actime, &aft);
	if (!SetFileTime((HANDLE)_get_osfhandle(fh), NULL, &aft, &mft)) {
		errno = EINVAL;
		rc = -1;
	} else
		rc = 0;
	close(fh);
	return rc;
}

unsigned int sleep (unsigned int seconds)
{
	Sleep(seconds*1000);
	return 0;
}

int mkstemp(char *template)
{
	wchar_t *wtemplate = _wmktemp(utf8_to_wbuf(template));
	if (wtemplate == NULL)
		return -1;
	wchar_to_utf8(template, wtemplate, strlen(template) + 1);
	return _wopen(wtemplate, O_RDWR | O_CREAT | O_BINARY, 0600);
}

int gettimeofday(struct timeval *tv, void *tz)
{
	FILETIME ft;
	long long hnsec;

	GetSystemTimeAsFileTime(&ft);
	hnsec = filetime_to_hnsec(&ft);
	tv->tv_sec = hnsec / 10000000;
	tv->tv_usec = (hnsec % 10000000) / 10;
	return 0;
}

int pipe(int filedes[2])
{
	HANDLE h[2];

	/* this creates non-inheritable handles */
	if (!CreatePipe(&h[0], &h[1], NULL, 8192)) {
		errno = err_win_to_posix(GetLastError());
		return -1;
	}
	filedes[0] = _open_osfhandle((int)h[0], O_NOINHERIT);
	if (filedes[0] < 0) {
		CloseHandle(h[0]);
		CloseHandle(h[1]);
		return -1;
	}
	filedes[1] = _open_osfhandle((int)h[1], O_NOINHERIT);
	if (filedes[0] < 0) {
		close(filedes[0]);
		CloseHandle(h[1]);
		return -1;
	}
	return 0;
}

int poll(struct pollfd *ufds, unsigned int nfds, int timeout)
{
	int i, pending;

	if (timeout >= 0) {
		if (nfds == 0) {
			Sleep(timeout);
			return 0;
		}
		return errno = EINVAL, error("poll timeout not supported");
	}

	/*
	 * When there is only one fd to wait for, then we pretend that
	 * input is available and let the actual wait happen when the
	 * caller invokes read().
	 */
	if (nfds == 1) {
		if (!(ufds[0].events & POLLIN))
			return errno = EINVAL, error("POLLIN not set");
		ufds[0].revents = POLLIN;
		return 0;
	}

repeat:
	pending = 0;
	for (i = 0; i < nfds; i++) {
		DWORD avail = 0;
		HANDLE h = (HANDLE) _get_osfhandle(ufds[i].fd);
		if (h == INVALID_HANDLE_VALUE)
			return -1;	/* errno was set */

		if (!(ufds[i].events & POLLIN))
			return errno = EINVAL, error("POLLIN not set");

		/* this emulation works only for pipes */
		if (!PeekNamedPipe(h, NULL, 0, NULL, &avail, NULL)) {
			int err = GetLastError();
			if (err == ERROR_BROKEN_PIPE) {
				ufds[i].revents = POLLHUP;
				pending++;
			} else {
				errno = EINVAL;
				return error("PeekNamedPipe failed,"
					" GetLastError: %u", err);
			}
		} else if (avail) {
			ufds[i].revents = POLLIN;
			pending++;
		} else
			ufds[i].revents = 0;
	}
	if (!pending) {
		/*
		 * The only times that we spin here is when the process
		 * that is connected through the pipes is waiting for
		 * its own input data to become available. But since
		 * the process (pack-objects) is itself CPU intensive,
		 * it will happily pick up the time slice that we are
		 * relinquishing here.
		 */
		Sleep(0);
		goto repeat;
	}
	return 0;
}

struct tm *gmtime_r(const time_t *timep, struct tm *result)
{
	/* gmtime() in MSVCRT.DLL is thread-safe, but not reentrant */
	memcpy(result, gmtime(timep), sizeof(struct tm));
	return result;
}

struct tm *localtime_r(const time_t *timep, struct tm *result)
{
	/* localtime() in MSVCRT.DLL is thread-safe, but not reentrant */
	memcpy(result, localtime(timep), sizeof(struct tm));
	return result;
}

#undef getcwd
char *mingw_getcwd(char *pointer, int len)
{
	int i;
	wchar_t buffer[PATH_MAX];
	wchar_t *ret = _wgetcwd(buffer, PATH_MAX);
	/* _wgetcwd sets errno correctly. */
	if (!ret)
		return NULL;
	for (i = 0; buffer[i]; i++)
		if (buffer[i] == L'\\')
			buffer[i] = L'/';

	if (wchar_to_utf8(pointer, buffer, len) >= len) {
		errno = ERANGE;
		return NULL;
	}

	return pointer;
}

struct env_map {
	char *name;
	wchar_t *wide;
	char *utf8;
};

static struct env_map *env_map;
static int env_map_size = 0;
static int env_map_alloc = 0;

void env_unmap(const char *name, size_t length)
{
	int i;
	for (i = 0; i < env_map_size; ++i) {
		size_t len = strlen(env_map[i].name);
		if (len != length)
			continue;

		if (!memcmp(env_map[i].name, name, len)) {
			struct env_map *entry = &env_map[i];
			free(entry->name);
			free(entry->utf8);
			entry->wide = NULL;
		}
	}
}

#undef getenv
char *mingw_getenv(const char *name)
{
	wchar_t *wname = utf8_to_wchar_auto(name, NULL);
	wchar_t *result = _wgetenv(wname);
	struct env_map *entry;
	int i;

	if (!wname) {
		error("could not convert getenv variable name '%s' to wide", name);
		return NULL;
	}

	if (!result && !wcscmp(wname, L"TMPDIR")) {
		/* on Windows it is TMP and TEMP */
		result = _wgetenv(L"TMP");
		if (!result)
			result = _wgetenv(L"TEMP");
	}

	free(wname);

	if (!result)
		return NULL;

	for (i = 0; i < env_map_size; ++i) {
		if (env_map[i].wide == result)
			return env_map[i].utf8;
	}

	/* not found */
	ALLOC_GROW(env_map, env_map_size + 1, env_map_alloc);
	entry = &env_map[env_map_size++];
	entry->name = xstrdup(name);
	entry->wide = result;
	entry->utf8 = wchar_to_utf8_auto(result, NULL);

	return entry->utf8;
}

int mingw_putenv(const char *envstring)
{
	char *eq = strchrnul(envstring, '=');
	wchar_t *wenvstring = utf8_to_wchar_auto(envstring, NULL);

	if (!wenvstring) {
		error("could not convert putenv str '%s' to wide", envstring);
		return -1;
	}

	env_unmap(envstring, eq - envstring);
	free((char*)envstring);
	return _wputenv(wenvstring);
}

void mingw_unsetenv(const char *name)
{
	env_unmap(name, strlen(name));
	gitunsetenv(name);
}

/*
 * See http://msdn2.microsoft.com/en-us/library/17w5ykft(vs.71).aspx
 * (Parsing C++ Command-Line Arguments)
 */
static const char *quote_arg(const char *arg)
{
	/* count chars to quote */
	int len = 0, n = 0;
	int force_quotes = 0;
	char *q, *d;
	const char *p = arg;
	if (!*p) force_quotes = 1;
	while (*p) {
		if (isspace(*p) || *p == '*' || *p == '?' || *p == '{' || *p == '\'')
			force_quotes = 1;
		else if (*p == '"')
			n++;
		else if (*p == '\\') {
			int count = 0;
			while (*p == '\\') {
				count++;
				p++;
				len++;
			}
			if (*p == '"')
				n += count*2 + 1;
			continue;
		}
		len++;
		p++;
	}
	if (!force_quotes && n == 0)
		return arg;

	/* insert \ where necessary */
	d = q = xmalloc(len+n+3);
	*d++ = '"';
	while (*arg) {
		if (*arg == '"')
			*d++ = '\\';
		else if (*arg == '\\') {
			int count = 0;
			while (*arg == '\\') {
				count++;
				*d++ = *arg++;
			}
			if (*arg == '"') {
				while (count-- > 0)
					*d++ = '\\';
				*d++ = '\\';
			}
		}
		*d++ = *arg++;
	}
	*d++ = '"';
	*d++ = 0;
	return q;
}

static const char *parse_interpreter(const char *cmd)
{
	static char buf[100];
	char *p, *opt;
	int n, fd;

	/* don't even try a .exe */
	n = strlen(cmd);
	if (n >= 4 && !strcasecmp(cmd+n-4, ".exe"))
		return NULL;

	fd = open(cmd, O_RDONLY);
	if (fd < 0)
		return NULL;
	n = read(fd, buf, sizeof(buf)-1);
	close(fd);
	if (n < 4)	/* at least '#!/x' and not error */
		return NULL;

	if (buf[0] != '#' || buf[1] != '!')
		return NULL;
	buf[n] = '\0';
	p = buf + strcspn(buf, "\r\n");
	if (!*p)
		return NULL;

	*p = '\0';
	if (!(p = strrchr(buf+2, '/')) && !(p = strrchr(buf+2, '\\')))
		return NULL;
	/* strip options */
	if ((opt = strchr(p+1, ' ')))
		*opt = '\0';
	return p+1;
}

/*
 * Splits the PATH into parts.
 */
static char **get_path_split(void)
{
	char *p, **path, *envpath = getenv("PATH");
	int i, n = 0;

	if (!envpath || !*envpath)
		return NULL;

	envpath = xstrdup(envpath);
	p = envpath;
	while (p) {
		char *dir = p;
		p = strchr(p, ';');
		if (p) *p++ = '\0';
		if (*dir) {	/* not earlier, catches series of ; */
			++n;
		}
	}
	if (!n)
		return NULL;

	path = xmalloc((n+1)*sizeof(char *));
	p = envpath;
	i = 0;
	do {
		if (*p)
			path[i++] = xstrdup(p);
		p = p+strlen(p)+1;
	} while (i < n);
	path[i] = NULL;

	free(envpath);

	return path;
}

static void free_path_split(char **path)
{
	char **p = path;

	if (!path)
		return;

	while (*p)
		free(*p++);
	free(path);
}

/*
 * exe_only means that we only want to detect .exe files, but not scripts
 * (which do not have an extension)
 */
static char *lookup_prog(const char *dir, const char *cmd, int isexe, int exe_only)
{
	char path[MAX_PATH];
	snprintf(path, sizeof(path), "%s/%s.exe", dir, cmd);

	if (!isexe && access(path, F_OK) == 0)
		return xstrdup(path);
	path[strlen(path)-4] = '\0';
	if ((!exe_only || isexe) && access(path, F_OK) == 0)
		if (!(GetFileAttributesW(utf8_to_wbuf(path)) & FILE_ATTRIBUTE_DIRECTORY))
			return xstrdup(path);
	return NULL;
}

/*
 * Determines the absolute path of cmd using the the split path in path.
 * If cmd contains a slash or backslash, no lookup is performed.
 */
static char *path_lookup(const char *cmd, char **path, int exe_only)
{
	char *prog = NULL;
	int len = strlen(cmd);
	int isexe = len >= 4 && !strcasecmp(cmd+len-4, ".exe");

	if (strchr(cmd, '/') || strchr(cmd, '\\'))
		prog = xstrdup(cmd);

	while (!prog && *path)
		prog = lookup_prog(*path++, cmd, isexe, exe_only);

	return prog;
}

static int env_compare(const void *a, const void *b)
{
	char *const *ea = a;
	char *const *eb = b;
	return strcasecmp(*ea, *eb);
}

static pid_t mingw_spawnve_fd(const char *cmd, const char **argv, char **env,
			      int prepend_cmd, int fhin, int fhout, int fherr)
{
	STARTUPINFOW si;
	PROCESS_INFORMATION pi;
	struct strbuf envblk, args;
	unsigned flags;
	BOOL ret;
	wchar_t *wcmd;
	wchar_t *wargs;

	/* Determine whether or not we are associated to a console */
	HANDLE cons = CreateFile("CONOUT$", GENERIC_WRITE,
			FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL, NULL);
	if (cons == INVALID_HANDLE_VALUE) {
		/*
		 * There is no console associated with this process.
		 * Since the child is a console process, Windows
		 * would normally create a console window. But
		 * since we'll be redirecting std streams, we do
		 * not need the console.
		 * It is necessary to use DETACHED_PROCESS
		 * instead of CREATE_NO_WINDOW to make ssh
		 * recognize that it has no console.
		 */
		flags = DETACHED_PROCESS;
	} else {
		/*
		 * There is already a console. If we specified
		 * DETACHED_PROCESS here, too, Windows would
		 * disassociate the child from the console.
		 * The same is true for CREATE_NO_WINDOW.
		 * Go figure!
		 */
		flags = 0;
		CloseHandle(cons);
	}
	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESTDHANDLES;
	si.hStdInput = (HANDLE) _get_osfhandle(fhin);
	si.hStdOutput = (HANDLE) _get_osfhandle(fhout);
	si.hStdError = (HANDLE) _get_osfhandle(fherr);

	/* concatenate argv, quoting args as we go */
	strbuf_init(&args, 0);
	if (prepend_cmd) {
		char *quoted = (char *)quote_arg(cmd);
		strbuf_addstr(&args, quoted);
		if (quoted != cmd)
			free(quoted);
	}
	for (; *argv; argv++) {
		char *quoted = (char *)quote_arg(*argv);
		if (*args.buf)
			strbuf_addch(&args, ' ');
		strbuf_addstr(&args, quoted);
		if (quoted != *argv)
			free(quoted);
	}

	if (env) {
		int count = 0;
		char **e, **sorted_env;

		for (e = env; *e; e++)
			count++;

		/* environment must be sorted */
		sorted_env = xmalloc(sizeof(*sorted_env) * (count + 1));
		memcpy(sorted_env, env, sizeof(*sorted_env) * (count + 1));
		qsort(sorted_env, count, sizeof(*sorted_env), env_compare);

		strbuf_init(&envblk, 0);
		for (e = sorted_env; *e; e++) {
			strbuf_addstr(&envblk, *e);
			strbuf_addch(&envblk, '\0');
		}
		free(sorted_env);
	}

	memset(&pi, 0, sizeof(pi));
	wcmd = utf8_to_wchar_auto(cmd, NULL);
	wargs = utf8_to_wchar_auto(args.buf, NULL);
	if (!wcmd) {
		error("could not convert cmd str '%s' to wide", cmd);
		return -1;
	}
	if (!wargs) {
		error("could not convert cmd args str '%s' to wide", args.buf);
		return -1;
	}

	ret = CreateProcessW(wcmd, wargs, NULL, NULL, TRUE, flags,
		env ? envblk.buf : NULL, NULL, &si, &pi);
	free(wcmd);
	free(wargs);

	if (env)
		strbuf_release(&envblk);
	strbuf_release(&args);

	if (!ret) {
		errno = ENOENT;
		return -1;
	}
	CloseHandle(pi.hThread);
	return (pid_t)pi.hProcess;
}

static pid_t mingw_spawnve(const char *cmd, const char **argv, char **env,
			   int prepend_cmd)
{
	return mingw_spawnve_fd(cmd, argv, env, prepend_cmd, 0, 1, 2);
}

pid_t mingw_spawnvpe(const char *cmd, const char **argv, char **env,
		     int fhin, int fhout, int fherr)
{
	pid_t pid;
	char **path = get_path_split();
	char *prog = path_lookup(cmd, path, 0);

	if (!prog) {
		errno = ENOENT;
		pid = -1;
	}
	else {
		const char *interpr = parse_interpreter(prog);

		if (interpr) {
			const char *argv0 = argv[0];
			char *iprog = path_lookup(interpr, path, 1);
			argv[0] = prog;
			if (!iprog) {
				errno = ENOENT;
				pid = -1;
			}
			else {
				pid = mingw_spawnve_fd(iprog, argv, env, 1,
						       fhin, fhout, fherr);
				free(iprog);
			}
			argv[0] = argv0;
		}
		else
			pid = mingw_spawnve_fd(prog, argv, env, 0,
					       fhin, fhout, fherr);
		free(prog);
	}
	free_path_split(path);
	return pid;
}

static int try_shell_exec(const char *cmd, char *const *argv, char **env)
{
	const char *interpr = parse_interpreter(cmd);
	char **path;
	char *prog;
	int pid = 0;

	if (!interpr)
		return 0;
	path = get_path_split();
	prog = path_lookup(interpr, path, 1);
	if (prog) {
		int argc = 0;
		const char **argv2;
		while (argv[argc]) argc++;
		argv2 = xmalloc(sizeof(*argv) * (argc+1));
		argv2[0] = (char *)cmd;	/* full path to the script file */
		memcpy(&argv2[1], &argv[1], sizeof(*argv) * argc);
		pid = mingw_spawnve(prog, argv2, env, 1);
		if (pid >= 0) {
			int status;
			if (waitpid(pid, &status, 0) < 0)
				status = 255;
			exit(status);
		}
		pid = 1;	/* indicate that we tried but failed */
		free(prog);
		free(argv2);
	}
	free_path_split(path);
	return pid;
}

static void mingw_execve(const char *cmd, char *const *argv, char *const *env)
{
	/* check if git_command is a shell script */
	if (!try_shell_exec(cmd, argv, (char **)env)) {
		int pid, status;

		pid = mingw_spawnve(cmd, (const char **)argv, (char **)env, 0);
		if (pid < 0)
			return;
		if (waitpid(pid, &status, 0) < 0)
			status = 255;
		exit(status);
	}
}

void mingw_execvp(const char *cmd, char *const *argv)
{
	char **path = get_path_split();
	char *prog = path_lookup(cmd, path, 0);

	if (prog) {
		mingw_execve(prog, argv, environ);
		free(prog);
	} else
		errno = ENOENT;

	free_path_split(path);
}

int mingw_system(const char *command)
{
	int ret;
	wchar_t *wcommand = utf8_to_wchar_auto(command, NULL);

	if (!wcommand) {
		error("could not convert command '%s' to utf8", command);
		return -1;
	}

	ret = _wsystem(wcommand);
	free(wcommand);

	return ret;
}

static char **copy_environ(void)
{
	char **env;
	int i = 0;
	while (environ[i])
		i++;
	env = xmalloc((i+1)*sizeof(*env));
	for (i = 0; environ[i]; i++)
		env[i] = xstrdup(environ[i]);
	env[i] = NULL;
	return env;
}

void free_environ(char **env)
{
	int i;
	for (i = 0; env[i]; i++)
		free(env[i]);
	free(env);
}

static int lookup_env(char **env, const char *name, size_t nmln)
{
	int i;

	for (i = 0; env[i]; i++) {
		if (0 == strncmp(env[i], name, nmln)
		    && '=' == env[i][nmln])
			/* matches */
			return i;
	}
	return -1;
}

/*
 * If name contains '=', then sets the variable, otherwise it unsets it
 */
static char **env_setenv(char **env, const char *name)
{
	char *eq = strchrnul(name, '=');
	int i = lookup_env(env, name, eq-name);

	if (i < 0) {
		if (*eq) {
			for (i = 0; env[i]; i++)
				;
			env = xrealloc(env, (i+2)*sizeof(*env));
			env[i] = xstrdup(name);
			env[i+1] = NULL;
		}
	}
	else {
		free(env[i]);
		if (*eq)
			env[i] = xstrdup(name);
		else
			for (; env[i]; i++)
				env[i] = env[i+1];
	}
	return env;
}

/*
 * Copies global environ and adjusts variables as specified by vars.
 */
char **make_augmented_environ(const char *const *vars)
{
	char **env = copy_environ();

	while (*vars)
		env = env_setenv(env, *vars++);
	return env;
}

/*
 * Note, this isn't a complete replacement for getaddrinfo. It assumes
 * that service contains a numerical port, or that it it is null. It
 * does a simple search using gethostbyname, and returns one IPv4 host
 * if one was found.
 */
static int WSAAPI getaddrinfo_stub(const char *node, const char *service,
				   const struct addrinfo *hints,
				   struct addrinfo **res)
{
	struct hostent *h = gethostbyname(node);
	struct addrinfo *ai;
	struct sockaddr_in *sin;

	if (!h)
		return WSAGetLastError();

	ai = xmalloc(sizeof(struct addrinfo));
	*res = ai;
	ai->ai_flags = 0;
	ai->ai_family = AF_INET;
	ai->ai_socktype = hints->ai_socktype;
	switch (hints->ai_socktype) {
	case SOCK_STREAM:
		ai->ai_protocol = IPPROTO_TCP;
		break;
	case SOCK_DGRAM:
		ai->ai_protocol = IPPROTO_UDP;
		break;
	default:
		ai->ai_protocol = 0;
		break;
	}
	ai->ai_addrlen = sizeof(struct sockaddr_in);
	ai->ai_canonname = strdup(h->h_name);

	sin = xmalloc(ai->ai_addrlen);
	memset(sin, 0, ai->ai_addrlen);
	sin->sin_family = AF_INET;
	if (service)
		sin->sin_port = htons(atoi(service));
	sin->sin_addr = *(struct in_addr *)h->h_addr;
	ai->ai_addr = (struct sockaddr *)sin;
	ai->ai_next = 0;
	return 0;
}

static void WSAAPI freeaddrinfo_stub(struct addrinfo *res)
{
	free(res->ai_canonname);
	free(res->ai_addr);
	free(res);
}

static int WSAAPI getnameinfo_stub(const struct sockaddr *sa, socklen_t salen,
				   char *host, DWORD hostlen,
				   char *serv, DWORD servlen, int flags)
{
	const struct sockaddr_in *sin = (const struct sockaddr_in *)sa;
	if (sa->sa_family != AF_INET)
		return EAI_FAMILY;
	if (!host && !serv)
		return EAI_NONAME;

	if (host && hostlen > 0) {
		struct hostent *ent = NULL;
		if (!(flags & NI_NUMERICHOST))
			ent = gethostbyaddr((const char *)&sin->sin_addr,
					    sizeof(sin->sin_addr), AF_INET);

		if (ent)
			snprintf(host, hostlen, "%s", ent->h_name);
		else if (flags & NI_NAMEREQD)
			return EAI_NONAME;
		else
			snprintf(host, hostlen, "%s", inet_ntoa(sin->sin_addr));
	}

	if (serv && servlen > 0) {
		struct servent *ent = NULL;
		if (!(flags & NI_NUMERICSERV))
			ent = getservbyport(sin->sin_port,
					    flags & NI_DGRAM ? "udp" : "tcp");

		if (ent)
			snprintf(serv, servlen, "%s", ent->s_name);
		else
			snprintf(serv, servlen, "%d", ntohs(sin->sin_port));
	}

	return 0;
}

static HMODULE ipv6_dll = NULL;
static void (WSAAPI *ipv6_freeaddrinfo)(struct addrinfo *res);
static int (WSAAPI *ipv6_getaddrinfo)(const char *node, const char *service,
				      const struct addrinfo *hints,
				      struct addrinfo **res);
static int (WSAAPI *ipv6_getnameinfo)(const struct sockaddr *sa, socklen_t salen,
				      char *host, DWORD hostlen,
				      char *serv, DWORD servlen, int flags);
/*
 * gai_strerror is an inline function in the ws2tcpip.h header, so we
 * don't need to try to load that one dynamically.
 */

static void socket_cleanup(void)
{
	WSACleanup();
	if (ipv6_dll)
		FreeLibrary(ipv6_dll);
	ipv6_dll = NULL;
	ipv6_freeaddrinfo = freeaddrinfo_stub;
	ipv6_getaddrinfo = getaddrinfo_stub;
	ipv6_getnameinfo = getnameinfo_stub;
}

static void ensure_socket_initialization(void)
{
	WSADATA wsa;
	static int initialized = 0;
	const char *libraries[] = { "ws2_32.dll", "wship6.dll", NULL };
	const char **name;

	if (initialized)
		return;

	if (WSAStartup(MAKEWORD(2,2), &wsa))
		die("unable to initialize winsock subsystem, error %d",
			WSAGetLastError());

	for (name = libraries; *name; name++) {
		ipv6_dll = LoadLibrary(*name);
		if (!ipv6_dll)
			continue;

		ipv6_freeaddrinfo = (void (WSAAPI *)(struct addrinfo *))
			GetProcAddress(ipv6_dll, "freeaddrinfo");
		ipv6_getaddrinfo = (int (WSAAPI *)(const char *, const char *,
						   const struct addrinfo *,
						   struct addrinfo **))
			GetProcAddress(ipv6_dll, "getaddrinfo");
		ipv6_getnameinfo = (int (WSAAPI *)(const struct sockaddr *,
						   socklen_t, char *, DWORD,
						   char *, DWORD, int))
			GetProcAddress(ipv6_dll, "getnameinfo");
		if (!ipv6_freeaddrinfo || !ipv6_getaddrinfo || !ipv6_getnameinfo) {
			FreeLibrary(ipv6_dll);
			ipv6_dll = NULL;
		} else
			break;
	}
	if (!ipv6_freeaddrinfo || !ipv6_getaddrinfo || !ipv6_getnameinfo) {
		ipv6_freeaddrinfo = freeaddrinfo_stub;
		ipv6_getaddrinfo = getaddrinfo_stub;
		ipv6_getnameinfo = getnameinfo_stub;
	}

	atexit(socket_cleanup);
	initialized = 1;
}

#undef gethostbyname
struct hostent *mingw_gethostbyname(const char *host)
{
	ensure_socket_initialization();
	return gethostbyname(host);
}

void mingw_freeaddrinfo(struct addrinfo *res)
{
	ipv6_freeaddrinfo(res);
}

int mingw_getaddrinfo(const char *node, const char *service,
		      const struct addrinfo *hints, struct addrinfo **res)
{
	ensure_socket_initialization();
	return ipv6_getaddrinfo(node, service, hints, res);
}

int mingw_getnameinfo(const struct sockaddr *sa, socklen_t salen,
		      char *host, DWORD hostlen, char *serv, DWORD servlen,
		      int flags)
{
	ensure_socket_initialization();
	return ipv6_getnameinfo(sa, salen, host, hostlen, serv, servlen, flags);
}

int mingw_socket(int domain, int type, int protocol)
{
	int sockfd;
	SOCKET s = WSASocket(domain, type, protocol, NULL, 0, 0);
	if (s == INVALID_SOCKET) {
		/*
		 * WSAGetLastError() values are regular BSD error codes
		 * biased by WSABASEERR.
		 * However, strerror() does not know about networking
		 * specific errors, which are values beginning at 38 or so.
		 * Therefore, we choose to leave the biased error code
		 * in errno so that _if_ someone looks up the code somewhere,
		 * then it is at least the number that are usually listed.
		 */
		errno = WSAGetLastError();
		return -1;
	}
	/* convert into a file descriptor */
	if ((sockfd = _open_osfhandle(s, O_RDWR|O_BINARY)) < 0) {
		closesocket(s);
		return error("unable to make a socket file descriptor: %s",
			strerror(errno));
	}
	return sockfd;
}

#undef connect
int mingw_connect(int sockfd, struct sockaddr *sa, size_t sz)
{
	SOCKET s = (SOCKET)_get_osfhandle(sockfd);
	return connect(s, sa, sz);
}

#undef rename
int mingw_rename(const char *pold, const char *pnew)
{
	DWORD attrs, gle;
	int tries = 0;
	const wchar_t *pold_w = utf8_to_wbuf(pold);
	const wchar_t *pnew_w = utf8_to_wbuf(pnew);

	/*
	 * Try native rename() first to get errno right.
	 * It is based on MoveFile(), which cannot overwrite existing files.
	 */
	if (!_wrename(pold_w, pnew_w))
		return 0;
	if (errno != EEXIST)
		return -1;
repeat:
	if (MoveFileExW(pold_w, pnew_w, MOVEFILE_REPLACE_EXISTING))
		return 0;
	/* TODO: translate more errors */
	gle = GetLastError();
	if (gle == ERROR_ACCESS_DENIED &&
	    (attrs = GetFileAttributesW(pnew_w)) != INVALID_FILE_ATTRIBUTES) {
		if (attrs & FILE_ATTRIBUTE_DIRECTORY) {
			errno = EISDIR;
			return -1;
		}
		if ((attrs & FILE_ATTRIBUTE_READONLY) &&
		    SetFileAttributesW(pnew_w, attrs & ~FILE_ATTRIBUTE_READONLY)) {
			if (MoveFileExW(pold_w, pnew_w, MOVEFILE_REPLACE_EXISTING))
				return 0;
			gle = GetLastError();
			/* revert file attributes on failure */
			SetFileAttributesW(pnew_w, attrs);
		}
	}
	if (tries < ARRAY_SIZE(delay) && gle == ERROR_ACCESS_DENIED) {
		/*
		 * We assume that some other process had the source or
		 * destination file open at the wrong moment and retry.
		 * In order to give the other process a higher chance to
		 * complete its operation, we give up our time slice now.
		 * If we have to retry again, we do sleep a bit.
		 */
		Sleep(delay[tries]);
		tries++;
		goto repeat;
	}
	if (gle == ERROR_ACCESS_DENIED &&
	       ask_user_yes_no("Rename from '%s' to '%s' failed. "
		       "Should I try again?", pold, pnew))
		goto repeat;

	errno = EACCES;
	return -1;
}

/*
 * Note that this doesn't return the actual pagesize, but
 * the allocation granularity. If future Windows specific git code
 * needs the real getpagesize function, we need to find another solution.
 */
int mingw_getpagesize(void)
{
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	return si.dwAllocationGranularity;
}

struct passwd *getpwuid(int uid)
{
	static char user_name[100];
	static struct passwd p;

	DWORD len = sizeof(user_name);
	if (!GetUserName(user_name, &len))
		return NULL;
	p.pw_name = user_name;
	p.pw_gecos = "unknown";
	p.pw_dir = NULL;
	return &p;
}

static HANDLE timer_event;
static HANDLE timer_thread;
static int timer_interval;
static int one_shot;
static sig_handler_t timer_fn = SIG_DFL;

/*
 * The timer works like this:
 * The thread, ticktack(), is a trivial routine that most of the time
 * only waits to receive the signal to terminate. The main thread tells
 * the thread to terminate by setting the timer_event to the signalled
 * state.
 * But ticktack() interrupts the wait state after the timer's interval
 * length to call the signal handler.
 */

static unsigned __stdcall ticktack(void *dummy)
{
	while (WaitForSingleObject(timer_event, timer_interval) == WAIT_TIMEOUT) {
		if (timer_fn == SIG_DFL)
			die("Alarm");
		if (timer_fn != SIG_IGN)
			timer_fn(SIGALRM);
		if (one_shot)
			break;
	}
	return 0;
}

static int start_timer_thread(void)
{
	timer_event = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (timer_event) {
		timer_thread = (HANDLE) _beginthreadex(NULL, 0, ticktack, NULL, 0, NULL);
		if (!timer_thread )
			return errno = ENOMEM,
				error("cannot start timer thread");
	} else
		return errno = ENOMEM,
			error("cannot allocate resources for timer");
	return 0;
}

static void stop_timer_thread(void)
{
	if (timer_event)
		SetEvent(timer_event);	/* tell thread to terminate */
	if (timer_thread) {
		int rc = WaitForSingleObject(timer_thread, 1000);
		if (rc == WAIT_TIMEOUT)
			error("timer thread did not terminate timely");
		else if (rc != WAIT_OBJECT_0)
			error("waiting for timer thread failed: %lu",
			      GetLastError());
		CloseHandle(timer_thread);
	}
	if (timer_event)
		CloseHandle(timer_event);
	timer_event = NULL;
	timer_thread = NULL;
}

static inline int is_timeval_eq(const struct timeval *i1, const struct timeval *i2)
{
	return i1->tv_sec == i2->tv_sec && i1->tv_usec == i2->tv_usec;
}

int setitimer(int type, struct itimerval *in, struct itimerval *out)
{
	static const struct timeval zero;
	static int atexit_done;

	if (out != NULL)
		return errno = EINVAL,
			error("setitimer param 3 != NULL not implemented");
	if (!is_timeval_eq(&in->it_interval, &zero) &&
	    !is_timeval_eq(&in->it_interval, &in->it_value))
		return errno = EINVAL,
			error("setitimer: it_interval must be zero or eq it_value");

	if (timer_thread)
		stop_timer_thread();

	if (is_timeval_eq(&in->it_value, &zero) &&
	    is_timeval_eq(&in->it_interval, &zero))
		return 0;

	timer_interval = in->it_value.tv_sec * 1000 + in->it_value.tv_usec / 1000;
	one_shot = is_timeval_eq(&in->it_interval, &zero);
	if (!atexit_done) {
		atexit(stop_timer_thread);
		atexit_done = 1;
	}
	return start_timer_thread();
}

int sigaction(int sig, struct sigaction *in, struct sigaction *out)
{
	if (sig != SIGALRM)
		return errno = EINVAL,
			error("sigaction only implemented for SIGALRM");
	if (out != NULL)
		return errno = EINVAL,
			error("sigaction: param 3 != NULL not implemented");

	timer_fn = in->sa_handler;
	return 0;
}

#undef signal
sig_handler_t mingw_signal(int sig, sig_handler_t handler)
{
	sig_handler_t old = timer_fn;
	if (sig != SIGALRM)
		return signal(sig, handler);
	timer_fn = handler;
	return old;
}

static const char *make_backslash_path(const char *path, char *buf)
{
	char *c;

	if (strlcpy(buf, path, PATH_MAX) >= PATH_MAX)
		die("Too long path: %.*s", 60, path);

	for (c = buf; *c; c++) {
		if (*c == '/')
			*c = '\\';
	}
	return buf;
}

void mingw_open_html(const char *unixpath)
{
	char buf[PATH_MAX + 1];
	const char *htmlpath = make_backslash_path(unixpath, buf);
	typedef HINSTANCE (WINAPI *T)(HWND, const wchar_t *,
			const wchar_t *, const wchar_t *, const wchar_t *, INT);
	T ShellExecute;
	HMODULE shell32;
	int r;

	shell32 = LoadLibrary("shell32.dll");
	if (!shell32)
		die("cannot load shell32.dll");
	ShellExecute = (T)GetProcAddress(shell32, "ShellExecuteW");
	if (!ShellExecute)
		die("cannot run browser");

	printf("Launching default browser to display HTML ...\n");
	r = (int)ShellExecute(NULL, L"open", utf8_to_wbuf(htmlpath), NULL, L"\\", SW_SHOWNORMAL);
	FreeLibrary(shell32);
	/* see the MSDN documentation referring to the result codes here */
	if (r <= 32) {
		die("failed to launch browser for %.*s", MAX_PATH, unixpath);
	}
}

int link(const char *oldpath, const char *newpath)
{
	typedef BOOL (WINAPI *T)(const wchar_t*, const wchar_t*, LPSECURITY_ATTRIBUTES);
	static T create_hard_link = NULL;
	if (!create_hard_link) {
		create_hard_link = (T) GetProcAddress(
			GetModuleHandle("kernel32.dll"), "CreateHardLinkW");
		if (!create_hard_link)
			create_hard_link = (T)-1;
	}
	if (create_hard_link == (T)-1) {
		errno = ENOSYS;
		return -1;
	}
	if (!create_hard_link(utf8_to_wbuf(newpath), utf8_to_wbuf(oldpath), NULL)) {
		errno = err_win_to_posix(GetLastError());
		return -1;
	}
	return 0;
}

int symlink(const char *oldpath, const char *newpath)
{
	typedef BOOL WINAPI (*symlink_fn)(const wchar_t*, const wchar_t*, DWORD);
	static symlink_fn create_symbolic_link = NULL;
	char buf[PATH_MAX + 1];

	if (!create_symbolic_link) {
		create_symbolic_link = (symlink_fn) GetProcAddress(
				GetModuleHandle("kernel32.dll"), "CreateSymbolicLinkW");
		if (!create_symbolic_link)
			create_symbolic_link = (symlink_fn)-1;
	}
	if (create_symbolic_link == (symlink_fn)-1) {
		errno = ENOSYS;
		return -1;
	}

	if (!create_symbolic_link(utf8_to_wbuf(newpath), utf8_to_wbuf(make_backslash_path(oldpath, buf)), 0)) {
		errno = err_win_to_posix(GetLastError());
		return -1;
	}
	return 0;
}

int wreadlink(const wchar_t *path, wchar_t *buf, size_t bufsiz)
{
	HANDLE handle = CreateFileW(path, GENERIC_READ,
			FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
			NULL, OPEN_EXISTING,
			FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT,
			NULL);

	if (handle != INVALID_HANDLE_VALUE) {
		unsigned char buffer[MAXIMUM_REPARSE_DATA_BUFFER_SIZE];
		DWORD dummy = 0;
		if (DeviceIoControl(handle, FSCTL_GET_REPARSE_POINT, NULL, 0, buffer,
			MAXIMUM_REPARSE_DATA_BUFFER_SIZE, &dummy, NULL)) {
			REPARSE_DATA_BUFFER *b = (REPARSE_DATA_BUFFER *) buffer;
			if (b->ReparseTag == IO_REPARSE_TAG_SYMLINK) {
				int len = b->SymbolicLinkReparseBuffer.SubstituteNameLength / sizeof(wchar_t);
				int offset = b->SymbolicLinkReparseBuffer.SubstituteNameOffset / sizeof(wchar_t);
				int i;
				if (buf) {
					len = (bufsiz < len) ? bufsiz : len;
					wcsncpy(buf, & b->SymbolicLinkReparseBuffer.PathBuffer[offset], len);
					for (i = 0; i < len; i++)
						if (buf[i] == L'\\')
							buf[i] = L'/';
				}
				CloseHandle(handle);
				return len;
			}
		}

		CloseHandle(handle);
	}

	errno = EINVAL;
	return -1;
}

int readlink(const char *path, char *buf, size_t bufsiz)
{
	wchar_t buffer[MAX_PATH];
	int result = wreadlink(utf8_to_wbuf(path), buffer, MAX_PATH);
	if (result >= 0)
		return wchar_to_utf8(buf, buffer, bufsiz);

	return -1;
}

char *getpass(const char *prompt)
{
	struct strbuf buf = STRBUF_INIT;

	fputs(prompt, stderr);
	for (;;) {
		char c = _getch();
		if (c == '\r' || c == '\n')
			break;
		strbuf_addch(&buf, c);
	}
	fputs("\n", stderr);
	return strbuf_detach(&buf, NULL);
}

/* readdir implementation to avoid extra lstats for Git. */
struct win_DIR
{
	HANDLE           search_handle;
	WIN32_FIND_DATAW find_file_data;
	struct           dirent dir_entry;

	/* Contains 1 if `find_file_data' contains the first found entry (filled
	   by opendir). This is only the case for the first call to readdir. */
	int has_first_entry;
};

DIR *win_opendir(const char *name)
{
	wchar_t *wname;
	int len;
	struct win_DIR *dir = xmalloc(sizeof(struct win_DIR));
	memset(dir, 0, sizeof(struct win_DIR));

	wname = utf8_to_wbuf(name);
	len = wcslen(wname);

	/* Append the search pattern "*" to the path. */
	if (len > 0) {
		if (wname[len-1] != L'/')
			wname[len++] = L'/';
		wname[len++] = L'*';
		wname[len] = 0;
	}

	dir->has_first_entry = 1;
	dir->search_handle = FindFirstFileW(wname, & dir->find_file_data);

	if (dir->search_handle == INVALID_HANDLE_VALUE) {
		errno = err_win_to_posix(GetLastError());
		free(dir);
		return NULL;
	}

	return (DIR*)dir;
}

int win_closedir(DIR *dir)
{
	struct win_DIR *wdir;
	HANDLE search_handle;

	wdir = (struct win_DIR*)dir;
	search_handle = wdir->search_handle;

	free(wdir);

	if (!FindClose(search_handle)) {
		errno = err_win_to_posix(GetLastError());
		return -1;
	}

	return 0;
}

struct dirent *win_readdir(DIR *dir)
{
	struct win_DIR *wdir = (struct win_DIR*)dir;

	if (!wdir->search_handle) {
		errno = EBADF; /* No set_errno for mingw */
		return NULL;
	}

	if (wdir->has_first_entry)
		wdir->has_first_entry = 0;
	else if (!FindNextFileW(wdir->search_handle, & wdir->find_file_data)) {
		int lasterr = GetLastError();
		/*
		 * POSIX says you shouldn't set errno when readdir can't
		 * find any more files; so, if another error we leave it set.
		 */
		if (lasterr != ERROR_NO_MORE_FILES)
			errno = err_win_to_posix(lasterr);
		return NULL;
	}

	/* We get here if `buf' contains valid data.  */
	wchar_to_utf8(wdir->dir_entry.d_name, wdir->find_file_data.cFileName, FILENAME_MAX);

	/* Set file type, based on WIN32_FIND_DATA */
	wdir->dir_entry.d_type = 0;
	if (IS_REPARSE_POINT(wdir->find_file_data))
		wdir->dir_entry.d_type |= DT_LNK;
	else if (wdir->find_file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		wdir->dir_entry.d_type |= DT_DIR;
	else
		wdir->dir_entry.d_type |= DT_REG;

	return & wdir->dir_entry;
}

/*
 * Returns the path of the executable file as utf8 string.
 *
 * plength is a pointer to a variable that receives the size of the returned
 * utf8 string in bytes.
 *
 * Returns NULL on failure.
 */
char *get_program_name(size_t *plength)
{
	wchar_t *buffer;
	size_t bufsize;
	DWORD length;
	char* filename;

	bufsize = MAX_PATH;
	buffer = xcalloc(bufsize, sizeof(wchar_t));

	length = GetModuleFileNameW(NULL, buffer, bufsize);
	if (!length) {
		free(buffer);
		errno = err_win_to_posix(GetLastError());
		return NULL;
	}

	/* Check if buffer was big enough. This should only happen with extended
	   \\?\ paths. */
	if (length == bufsize && (
	      GetLastError() == ERROR_INSUFFICIENT_BUFFER || /* Vista and later */
	      buffer[bufsize - 1] != 0) /* XP does not zero-terminate */
	    ) {

		bufsize = 1024 + 32767; /* \\?\ expanded + 32,767 path + NUL */
		buffer = xrealloc(buffer, bufsize);
		length = GetModuleFileNameW(NULL, buffer, bufsize);
		if (!length) {
			free(buffer);
			errno = err_win_to_posix(GetLastError());
			return NULL;
		}

		if (length == bufsize) {
			free(buffer);
			errno = ENAMETOOLONG;
			return NULL;
		}
	}

	filename = wchar_to_utf8_auto(buffer, plength);
	if (!filename) {
		free(buffer);
		return NULL;
	}

	free(buffer);

	return filename;
}

/*
 * Converts the specified wide command_line string to an utf8 argv array.
 *
 * pargc is a pointer to a variable that receives the number of arguments
 * in the returned argv.
 *
 * Note: Makes sure that argv[0] contains a full path.
 */
const char** convert_command_line(const wchar_t *command_line, int *pargc)
{
	int argc;
	char** argv;
	wchar_t** wide_argv;
	int total_bytes;
	int bytes;
	int* utf8_sizes;
	char* buffer;
	int i;

	wide_argv = CommandLineToArgvW(command_line, &argc);
	if (!wide_argv)
		return NULL;

	argv = xcalloc(argc + 1, sizeof(char*));
	argv[argc] = NULL;
	utf8_sizes = xcalloc(argc, sizeof(int));

	argv[0] = get_program_name((size_t*)&utf8_sizes[0]);
	if (!argv[0]) {
		LocalFree(wide_argv);
		free(argv);
		free(utf8_sizes);
		error("unable to get program name");
		return NULL;
	}

	/* sum the size of each resulting utf8 string */
	total_bytes = utf8_sizes[0];
	for (i = 1; i < argc; ++i) {
		bytes = WideCharToMultiByte(CP_UTF8, 0, wide_argv[i], -1,
		                            NULL, 0, NULL, NULL);
		if (!bytes) {
			LocalFree(wide_argv);
			free(argv);
			free(utf8_sizes);
			error("unable to get utf8 byte size for argv %d", i);
			return NULL;
		}

		utf8_sizes[i] = bytes;
		total_bytes += bytes;
	}

	buffer = xcalloc(total_bytes, sizeof(char));

	/* fill the buffer with the converted strings */
	total_bytes = utf8_sizes[0];
	for (i = 1; i < argc; ++i) {
		argv[i] = buffer + total_bytes;

		bytes = WideCharToMultiByte(CP_UTF8, 0, wide_argv[i], -1,
		                            argv[i], utf8_sizes[i], NULL, NULL);

		if (!bytes) {
			LocalFree(wide_argv);
			free(argv);
			free(utf8_sizes);
			free(buffer);
			error("unable to convert argv %d to utf8", i);
			return NULL;
		}

		total_bytes += bytes;
	}

	LocalFree(wide_argv);
	free(utf8_sizes);

	if (pargc)
		*pargc = argc;

	return (const char**)argv;
}
