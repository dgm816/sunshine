# sunshine
A command line usenet search tool.

# Build

Here's the basic outline to build the project:

    ~$ git clone https://github.com/dgm816/sunshine
    ~$ cd sunshine
    ~/sunshine$ mkdir build
    ~/sunshine$ cd build
    ~/sunshine/build$ cmake ..
    ~/sunshine/build$ make

You can then execute the binary out of the build directory:

    ~/sunshine/build$ ./sunshine --help

# Developer Notes

Just a quick list of the plugins that are currently being used.

Plugins:

  - .ignore
  - EditorConfig
  - File Watchers
  - GitLab Projects
  - IdeaVim
  - Makefile support
  - Markdown support
