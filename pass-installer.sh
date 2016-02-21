#!/bin/bash
# (C) 2016 Tobias Girstmair, released under the GNU GPL

git clone https://git.zx2c4.com/password-store
cd password-store/src
tee pass-patch.diff <<-EOF
--- password-store.sh.orig	2016-02-13 21:27:16.133642142 +0100
+++ password-store.sh	2016-02-13 14:02:17.412565405 +0100
@@ -324,7 +324,8 @@
 		else
 			echo "\${path%\/}"
 		fi
-		tree -C -l --noreport "\$PREFIX/\$path" | tail -n +2 | sed -E 's/\.gpg(\x1B\[[0-9]+m)?( ->|\$)/\1\2/g' # remove .gpg at end of line, but keep colors
+		#tree -C -l --noreport "\$PREFIX/\$path" | tail -n +2 | sed -E 's/\.gpg(\x1B\[[0-9]+m)?( ->|\$)/\1\2/g' # remove .gpg at end of line, but keep colors
+		tree -f -i -l --noreport "\$PREFIX/\$path" | tail -n +2 |grep "\.gpg"| sed 's/\.gpg\(\x1B\[[0-9]\+m\)\{0,1\}\( ->\|\$\)/\1\2/g' | sed "s|\$PREFIX/||g"
 	elif [[ -z \$path ]]; then
 		die "Error: password store is empty. Try \"pass init\"."
 	else
EOF
patch <pass-patch.diff
cd ..
sudo make install

echo "finished."
echo "you can now use PASSWORD_STORE_GPG_OPTS=\"your_passphrase\" pass show foo/bar |head -n 1"
