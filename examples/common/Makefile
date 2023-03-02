# it's pretty hard to get an extended shell prompt:
# - PS1 is always overwritten by .bashrc
# - PROMPT_COMMAND is a way around that - or passing --norc and --noprofile
# - But first we need to know the original PS1 value, which can be obtained
#   via /bin/bash -i -c '...'
# - Make doesn't make this easier ... for some reason the escaping doesn't
#   work the usual way here
shell: PS=$(shell /bin/bash -c -i 'echo $$PS1')
shell:
	@ PS1="${PS} (exploit) " setarch `uname -m` -R /bin/bash --noprofile --norc

zsh: PS=$(shell $$SHELL -c -i 'env | grep PS1 | cut -d = -f2-')
zsh:
	@ PS1="${PS} (exploit) " setarch `uname -m` -R $$SHELL -f

