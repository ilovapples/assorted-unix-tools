# source: https://mill-build.org/blog/14-bash-zsh-completion.html

_generate_apple_aut_completions() {
  local idx=$1; shift
  local words=( "$@" )
  local current_word=${words[idx]}

  local -a array
  if [ -n "${ZSH_VERSION:-}" ]; then
    array=("${(f)$(apple-aut help 2>/dev/null)}")
  elif [ -n "${BASH_VERSION:-}" ]; then
    readarray -t array <<<$(apple-aut help 2>/dev/null)
  fi

  for elem in "${array[@]}"; do
    if [[ $elem == "$current_word"* ]]; then echo "$elem"; fi
  done
}

_complete_apple_aut_bash() {
  if [ "$COMP_CWORD" -ne 1 ]; then
    # fallback to filename completion
    if type _filedir_x >/dev/null 2>&1; then
      _filedir_x
    elif type _filedir >/dev/null 2>&1; then
      _filedir
    else
      COMPREPLY=( $(compgen -f -- "${COMP_WORDS[COMP_CWORD]}") )
    fi
  else
    # actual completion
    local raw=($(_generate_apple_aut_completions "$COMP_CWORD" "${COMP_WORDS[@]}"))
    COMPREPLY=( "${raw[@]}" )
  fi
}

_complete_apple_aut_zsh() {
  local -a raw
  if (( CURRENT != 2 )); then
    # fallback to filename completion
    _files
  else
    # actual completion
    raw=($(_generate_apple_aut_completions "$CURRENT" "${words[@]}"))
    compadd -- $raw
  fi
}

if [ -n "${ZSH_VERSION:-}" ]; then
  # using zsh
  autoload -Uz compinit
  compinit
  compdef _complete_apple_aut_zsh apple-aut
elif [ -n "${BASH_VERSION:-}" ]; then
  # using bash
  complete -F _complete_apple_aut_bash apple-aut
fi
