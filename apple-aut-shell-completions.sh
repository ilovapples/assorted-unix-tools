# source: https://mill-build.org/blog/14-bash-zsh-completion.html

_generate_apple_aut_completions() {
  local idx=$1; shift
  local words=( "$@" )
  local current_word=${words[idx]}

  local -a array
  if [ -n "${ZSH_VERSION:-}" ]; then
    array=("${(f)$(apple-aut help 2>/dev/null)}")
  elif [ -n "${BASH_VERSION:-}" ]; then
    apple-aut help 2>/dev/null | readarray -t array
  fi

  for elem in "${array[@]}"; do
    if [[ $elem == "$current_word"* ]]; then echo "$elem"; fi
  done
}

_complete_apple_aut_bash() {
  local raw=($(_generate_apple_aut_completions "$COMP_CWORD" "${COMP_WORDS[@]}"))
  COMPREPLY=( "${raw[@]}" )
}

_complete_apple_aut_zsh() {
  local -a raw
  raw=($(_generate_apple_aut_completions "$CURRENT" "${words[@]}"))
  compadd -- $raw
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
