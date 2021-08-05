# A TL;DR client made in C++ 

## Usage

```
Usage: tldr [options] [command]

Options:
  -u, --update:   Update the local tldr cache
  -l, --language: Specify a comma-separated list of language codes, like "fr,it"
                  Pages will be searched for in this order. If nothing is found it will default to "en"
                  When used with --update, this will specify the languages to download
  -p, --platform  Override the default platform
  -v, --verbose:  When used with --update, this will print every file, wich was created or modified
                  When used with --platform, it will show a message when the page was not found in that platform
  -s, --stat      Show the translation status of a command. Results can be narrowed down by --language and --platform
      --raw       Display the raw Markdown page without formatting
  -h, --help:     Display this help
```

## Installation 

```
make build 
sudo make install
```

## Removal

```
sudo rm /usr/bin/local/tldr
```
