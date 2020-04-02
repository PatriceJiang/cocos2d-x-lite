#!/bin/bash
set -e

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
COCOS2DX_ROOT="$DIR"/../..

if [ -z "$NDK_ROOT" ]; then
    export NDK_ROOT=$HOME/bin/android-ndk
fi

# to fix git error: shallow update not allowed
# https://stackoverflow.com/questions/28983842/remote-rejected-shallow-update-not-allowed-after-changing-git-remote-url
#git remote add old https://github.com/cocos-creator/cocos2d-x-lite
#git fetch --unshallow old

cd $COCOS2DX_ROOT/tools/travis-scripts
./generate-bindings.sh $TRAVIS_BRANCH

cd $COCOS2DX_ROOT/external
set -x

external_tag=`node -e "let c = require(\"./config.json\");console.log(c.version)"`
repo_name=`node -e "let c = require(\"./config.json\");console.log(c.repo_name)"`
repo_parent=`node -e "let c = require(\"./config.json\");console.log(c.repo_parent)"`

echo "Clone external ..."
git clone --branch $external_tag $repo_parent$repo_parent --depth 1

echo "Build Android ... "
cd $COCOS2DX_ROOT/templates/js-template-link/frameworks/runtime-src/proj.android-studio
sed -i s/\$\{COCOS_X_ROOT\}/$COCOS2DX_ROOT/g app/build.gradle
echo "PROP_USE_CMAKE = true" >> gradle.properties 
./gradlew assembleDebug

set +x

cd $COCOS2DX_ROOT/tools/travis-scripts
./generate-cocosfiles.sh $TRAVIS_BRANCH
exit 0
