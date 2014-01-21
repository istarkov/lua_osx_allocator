lua_osx_allocator
=================

lua low mem osx allocator

1) get luajit <br>
   git clone http://luajit.org/git/luajit-2.0.git<br>
   git checkout v2.1<br>

2) apply patch<br>
git -c diff.mnemonicprefix=false -c core.quotepath=false apply -v --reject -p 1 /YOUR_PATH_TO_PATCH_FOLDER/patch.diff


3) build lua_osx_allocator

4) build lua with<br>
make LIBS="-losx_allocator" all<br>
make install<br>
ln -sf luajit-2.1.0-alpha /usr/local/bin/luajit<br>
