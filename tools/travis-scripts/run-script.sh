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
yes | ./tools/bin/sdkmanager  --verbose --sdk_root="$ANDROID_SDK" \
        "platforms;android-27" \
        "build-tools;28.0.3" \
        "platform-tools" \
        "tools" 
export ANDROID_HOME=$ANDROID_SDK

echo "Build Android ... "
cd $COCOS2DX_ROOT/templates/js-template-link/frameworks/runtime-src/proj.android-studio
sed -i "s@\${COCOS_X_ROOT}@$COCOS2DX_ROOT@g" app/build.gradle
sed -i "s@\${COCOS_X_ROOT}@$COCOS2DX_ROOT@g" settings.gradle
sed -i "s/^RELEASE_/#RELEASE_/g" gradle.properties
echo "PROP_USE_CMAKE = true" >> gradle.properties 
./gradlew assembleDebug

set +x

cd $COCOS2DX_ROOT/tools/travis-scripts
./generate-cocosfiles.sh $TRAVIS_BRANCH
exit 0
