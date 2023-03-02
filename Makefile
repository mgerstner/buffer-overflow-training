IMAGES = $(wildcard images/*.png)

all: bo_training.html

# for the suse theme I'm currently using a custom theme
# custom themes need to reside in the home directory an we can't pass an
# alternative location to asciidoc it seems
# therefore we "make" the theme via symlinks
$(HOME)/.asciidoc/themes/suse/suse.css: ./themes/suse/suse.css ./themes/install.sh
	./themes/install.sh

theme: $(HOME)/.asciidoc/themes/suse/suse.css

# slidy backend is part of the standard asciidoc
bo_training.html: bo_training.adoc $(IMAGES) $(HOME)/.asciidoc/themes/suse/suse.css
	/usr/bin/asciidoc --backend slidy -a theme=suse bo_training.adoc

clean:
	rm bo_training.html

show: bo_training.html
	xdg-open bo_training.html
