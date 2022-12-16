#! /bin/bash
#
# Bundle the compiled executables and assistant files into a release ZIP archive.
#
# We ASSUME the executables are 64bit RELEASE MODE builds.
#

pushd "$( dirname $0 )"

rm -rf __gup_release_tmpdir__
mkdir __gup_release_tmpdir__

cd __gup_release_tmpdir__

cp ../win32/msvc2022/bin/Release-Unicode-64bit-x64/*.exe .

mkdir -p demo-sources/atari_st
mkdir -p demo-sources/windows/msvc2022
cp -r ../ni_pack/*.c ../ni_pack/*.prj ../ni_pack/Makefile ../ni_pack/README.md  ./demo-sources/
cp -r ../ni_pack/atari_st/*.s ./demo-sources/atari_st
cp ../ni_pack/windows/msvc2022/*.vcxproj* ../ni_pack/windows/msvc2022/*.sln ./demo-sources/windows/msvc2022/
rm ./demo-sources/windows/msvc2022/*.user

echo "pack the stuff for demo ARJ file production"
cd demo-sources
../gup.exe a -r -m7 ../demo.arj *.c
cd ..

echo "Create distro zip"

7z a -r ../gup-release-$( date '+%Y%m%d.%H' ).zip .

popd




