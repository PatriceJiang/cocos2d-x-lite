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

set -x

echo "Download Android SDK... "
cd $COCOS2DX_ROOT/..
mkdir android
cd android
ANDROID_SDK=$COCOS2DX_ROOT/../android/android_sdk
wget https://dl.google.com/android/repository/commandlinetools-linux-6200805_latest.zip
unzip *.zip
yes | ./tools/bin/sdkmanager  --sdk_root="$ANDROID_SDK" \
        "platforms;android-27" \
        "build-tools;28.0.3" \
        "platform-tools" \
        "tools"  \
        "cmake;3.10.2.4988404"

export ANDROID_HOME=$ANDROID_SDK

export ANDROID_NDK=$HOME/bin/android-ndk
export ANDROID_NDK_HOME=$HOME/bin/android-ndk

#cmake_dir=`find $ANDROID_SDK/cmake -name bin -type d |head -1`
cmake_dir=$ANDROID_SDK/cmake/3.10.2.4988404/bin
export PATH=$cmake_dir:$PATH

echo "Build Android ... "
cd $COCOS2DX_ROOT/templates/js-template-link/frameworks/runtime-src/proj.android-studio
sed -i "s@\${COCOS_X_ROOT}@$COCOS2DX_ROOT@g" app/build.gradle
sed -i "s@\${COCOS_X_ROOT}@$COCOS2DX_ROOT@g" settings.gradle
sed -i "s/^RELEASE_/#RELEASE_/g" gradle.properties
echo "PROP_USE_CMAKE = true" >> gradle.properties
echo "ANDORID_NDK ${ANDROID_NDK} or ${ANDROID_NDK_HOME}" 
./gradlew assembleDebug --quiet

set +x

cd $COCOS2DX_ROOT/tools/travis-scripts
./generate-cocosfiles.sh $TRAVIS_BRANCH
exit 0
