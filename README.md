lua_osx_allocator
=================

lua low mem osx allocator

1) get luajit
git clone http://luajit.org/git/luajit-2.0.git
git checkout v2.1

2) apply patch
git -c diff.mnemonicprefix=false -c core.quotepath=false apply -v --reject -p 1 /YOUR_PATH_TO_PATCH_FOLDER/patch.diff

3) build lua_osx_allocator

4) build lua with
make LIBS="-losx_allocator" all
make install
ln -sf luajit-2.1.0-alpha /usr/local/bin/luajit
