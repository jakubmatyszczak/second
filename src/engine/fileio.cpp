#pragma once
#include <fcntl.h>
#include "commons.cpp"

namespace fileio
{
	// return number of bytes read
	inline s32 loadRawFile(const char* filepath, u32 maxSize, void* data)
	{
		int fd = open(filepath, O_RDONLY);
		if (fd < 0)
			return -1;
		lseek(fd, 0, SEEK_SET);
		int dataRead = read(fd, data, maxSize);
		close(fd);
		return dataRead;
	}
	inline bool saveRawFile(const char* filepath, const void* data, u32 size)
	{
		int	 fd		 = open(filepath, O_CREAT | O_WRONLY, 0644);
		bool success = false;
		if (write(fd, data, size) == size)
			success = true;
		close(fd);
		return success;
	}
}  // namespace fileio
