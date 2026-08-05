#include "paths.h"

#include <cerrno>
#include <climits>
#include <cstdlib>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

using namespace std;

// Where the images end up once installed. Only reached when the program cannot
// locate itself, which takes a system without /proc.
#ifndef SUDOKU_ASSETS_DIR
#define SUDOKU_ASSETS_DIR "/usr/share/sudoku/assets"
#endif

namespace {

const char * GRID_FILE = "grille.sdm";

string environment(const char * name){
  const char * value = getenv(name);
  if(value == NULL || *value == '\0')
    return string();
  return string(value);
}

bool isDirectory(const string & path){
  struct stat info;
  if(path.empty())
    return false;
  return stat(path.c_str(), &info) == 0 && S_ISDIR(info.st_mode);
}

bool exists(const string & path){
  struct stat info;
  return stat(path.c_str(), &info) == 0;
}

/* Directory of the running program, so that it finds what was installed or
   unpacked beside it whatever the prefix. */
string executableDir(){
  char path[PATH_MAX];
  ssize_t size = readlink("/proc/self/exe", path, sizeof path - 1);
  if(size <= 0)
    return string();

  path[size] = '\0';
  const string full(path);
  const string::size_type slash = full.rfind('/');
  if(slash == string::npos)
    return string();
  return full.substr(0, slash);
}

// Create every directory of the path, as mkdir -p does.
bool makeDirectories(const string & path){
  for(string::size_type i = 1; i <= path.size(); i++){
    if(i < path.size() && path[i] != '/')
      continue;
    const string part = path.substr(0, i);
    if(mkdir(part.c_str(), 0755) != 0 && errno != EEXIST)
      return false;
  }
  return true;
}

// Directory the desktop conventions reserve for the files of one program.
string dataDir(){
  string base = environment("XDG_DATA_HOME");
  if(base.empty()){
    const string home = environment("HOME");
    if(home.empty())
      return string();
    base = home + "/.local/share";
  }
  return base + "/sudoku";
}

}

string assetsDir(){
  const string forced = environment("SUDOKU_ASSETS_DIR");
  if(isDirectory(forced))
    return forced;

  const string exe = executableDir();
  if(!exe.empty()){
    // Installed under a prefix, or unpacked from the release archive.
    const string installed = exe + "/../share/sudoku/assets";
    if(isDirectory(installed))
      return installed;

    // Built in the source tree, the program sitting in a build directory.
    const string built = exe + "/../assets";
    if(isDirectory(built))
      return built;

    const string nested = exe + "/../../assets";
    if(isDirectory(nested))
      return nested;
  }

  return SUDOKU_ASSETS_DIR;
}

/* A grid saved by an older version, or written by the sample generator, sits in
   the working directory: keep using it rather than leaving it behind. */
string savedGridPath(){
  if(exists(GRID_FILE))
    return GRID_FILE;

  const string dir = dataDir();
  if(dir.empty())
    return GRID_FILE;
  return dir + "/" + GRID_FILE;
}

bool createSavedGridDirectory(){
  const string path = savedGridPath();
  const string::size_type slash = path.rfind('/');
  if(slash == string::npos)
    return true;
  return makeDirectories(path.substr(0, slash));
}
