# The all-in-one TL;DR client for users and contributors

## Usage

```
Usage: tldr [options] [command]

Options:
  -u, --update:   Update all currently installed languages in the local tldr cache
  -l, --language: Specify a comma-separated list of language codes, like "fr,it"
                  Pages will be searched for in this order. If nothing is found it will default to "en"
                  When used with --update, this will specify the languages to download
  -p, --platform  Override the default platform
  -v, --verbose:  When used with --update, this will print every file, wich was created or modified
                  When used with --platform, it will show a message when the page was not found in that platform
  -s, --stat      Show the translation status of a command. Results can be narrowed down by --language and --platform
      --raw       Display the raw Markdown page without formatting
  -r, --render    Specify the path to a custom Markdown page to render
  -a, --all       When used with --update, this updates all languages, instead of just the installed ones
  -f, --find      Perform a full-text search through all pages. Modifiers are --name, --description and --examples
      --stem      In case you don't need fancy English. Enjoy pure information. (This version of the page will be searched in with --find)
      --sub       Get a list of all documented sub-commands
      --destroy   Remove the tldr cache and the tldr executable. This option is only available if tldr-cpp was installed manually
      --pr        Print a page being modified in an open PR, specifying the PR number (e.g. --pr 1234)
  -h, --help:     Display help about tldr-cpp
```

## Installation 

### Arch Linux

On Arch Linux and Arch based systems, `tldr-cpp-git` can be installed from the AUR:
```
yay -S tldr-cpp-git
```

### Debian/Ubuntu

On Debian-based systems, just download and install the latest deb package from the [Releases](https://github.com/marchersimon/tldr-cpp/releases) page.

In case the error `tldr: error while loading shared libraries: libzip.so.5: cannot open shared object file: No such file or directory` appears, run

```
sudo ln -s /lib/x86_64-linux-gnu/libzip.so.4 /lib/x86_64-linux-gnu/libzip.so.5
```

### Manual installation

To compile and install `tldr-cpp` manually, make sure to have `libcurl` and `libzip` installed. The actual package name depends on your distribution.

```
git clone https://github.com/marchersimon/tldr-cpp
cd tldr-cpp
sudo make install
```
### Removal

To remove the executable and the tldr cache, just run
```
sudo tldr --destroy
```
This option will only be available if `tldr-cpp` was installed manually.
