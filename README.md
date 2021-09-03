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
  -f, --find      Perform a full-text search through all pages. Modifiers are --name, --description and --example
    , --stem      In case you don't need fancy English. Enjoy pure information. (This version of the page will be searched in with --find)
  -h, --help:     Display help about tldr-cpp
```

## Installation 

### Arch Linux

On Arch Linux and Arch based systems, `tldr-cpp-git` can be installed from the AUR:
```
yay -S tldr-cpp-git
```

### Debian/Ubuntu

On Ubuntu, `tldr-cpp` and it's dependencies have to be installed manually:
```
sudo apt install libcurl4-openssl-dev libzip-dev
git clone https://github.com/marchersimon/tldr-cpp
cd tldr-cpp
sudo make install
```
