#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <errno.h>
#include <malloc.h>
#include <sstream>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

using namespace std;

class DirectFileReader
{
private:
  int _fd;
  size_t _size;
  std::string _path;
public:
  DirectFileReader(const std::string& path)
    : _path(path)
  {
    _fd = open(path.c_str(), O_DIRECT);

    if (_fd == -1)
    {
      cerr << "Cannot open file for direct access." << endl;
    }
    else
    {
      _size = lseek(_fd, 0L, SEEK_END);

      cout << "Open: [" << path << "] = " << _size << " bytes" << endl;
    }
  }

  ~DirectFileReader()
  {
    if (_fd != -1)
      close(_fd);

    _fd = -1;
  }

  size_t Read(char* buffer, size_t pos, size_t size)
  {
    size_t seekPos = lseek(_fd, pos, SEEK_SET);
    if (pos != seekPos)
    {
      cerr << "Cannot seek, wanted "<< pos << " got " << seekPos << "." << endl;
      return 0;
    }

    if (_fd == -1)
    {
      cerr << "File not opened." << endl;
      return 0;
    }

    long readNum = read(_fd, buffer, size);
    if (readNum == -1)
    {
      cerr << "Could not read because err: " << errno << "." << endl;
    }
    return static_cast<size_t>(readNum);
  }

  bool IsOpened() const { return _fd != -1; }
};

int main(int argc, char** argv)
{
  if (argc != 3)
  {
    cerr << "Invalid number of parameters." << endl;
    cerr << "Usage: ckeep /path/to/file sleep_ms_per_mbyte" << endl;
    return -1;
  }

  DirectFileReader reader(argv[1]);

  stringstream ss;
  ss << argv[2];
  long millis = 0;
  ss >> millis;

  if (reader.IsOpened())
  {
    const size_t blockSize = 1024 * 1024;
    char* buffer = reinterpret_cast<char*>(memalign(blockSize, blockSize));
    size_t currPos = 0;
    while (true)
    {
      size_t ret = reader.Read(buffer, currPos, blockSize);
      currPos += ret;
      if (ret == 0)
      {
        currPos = 0;
      }
      if ((currPos & 0xFFFFFF) == 0x000000)
        cout << "reading pos: " << currPos << endl;

      if (millis)
        usleep(millis * 1000);
    }
    free(buffer);
  }

  return 0;
}
