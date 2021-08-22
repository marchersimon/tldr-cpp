#compdef _tldr tldr

function _tldr {

    _arguments \
        '--help[Show help]' \
        '--update[Update]' \
        '--platform[Platform]:noideawhattoputinhere:_get_all_platforms' \
        '--language[Language]:somestuff:_get_installed_languages' \
        '*::arg:->args'

    case $state in
        args)
            _get_all_pages
        ;;
    esac
}

function _get_all_pages {
    compadd $(find $HOME/.tldr/cache/pages/ -name "*.md" | grep --only-matching "\/[^/]*.md" | cut -c 2- | grep --only-matching ".*[^.md]")
}

function _get_all_platforms {
    compadd $(ls $HOME/.tldr/cache/pages)
}

function _get_installed_languages {
    compadd en $(ls -d $HOME/.tldr/cache/*/ | grep --only-matching "pages.*/$" | cut -c 7- | grep --only-matching ".*[^/]")
}
