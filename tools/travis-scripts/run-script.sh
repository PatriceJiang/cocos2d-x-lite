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

set -x

cd $COCOS2DX_ROOT/tools/travis-scripts
./generate-bindings.sh $TRAVIS_BRANCH


ANDROID_SDK=$COCOS2DX_ROOT/../android/android_sdk
export ANDROID_HOME=$ANDROID_SDK
export ANDROID_NDK=$NDK_ROOT       #installed in generate-bindings.sh
export ANDROID_NDK_HOME=$NDK_ROOT


function setup_andorid_ndk()
{
    echo "Download Android SDK... "
    cd $COCOS2DX_ROOT/..
    mkdir android
    cd android
    wget -t 5 -q https://dl.google.com/android/repository/commandlinetools-linux-6200805_latest.zip
    unzip *.zip
    yes | ./tools/bin/sdkmanager  --sdk_root="$ANDROID_SDK" \
            "platforms;android-27" \
            "build-tools;28.0.3" \
            "platform-tools" \
            "tools"  \
            "cmake;3.10.2.4988404"
    cmake_dir=$ANDROID_SDK/cmake/3.10.2.4988404/bin
    export PATH=$cmake_dir:$PATH
}

function build_android()
{
    echo "Compiling Android ... "
    cd $COCOS2DX_ROOT/templates/js-template-link/frameworks/runtime-src/proj.android-studio
    sed -i "s@\${COCOS_X_ROOT}@$COCOS2DX_ROOT@g" app/build.gradle
    sed -i "s@\${COCOS_X_ROOT}@$COCOS2DX_ROOT@g" settings.gradle
    sed -i "s/^RELEASE_/#RELEASE_/g" gradle.properties
    echo "PROP_USE_CMAKE = true" >> gradle.properties
    echo "ANDORID_NDK ${ANDROID_NDK} or ${ANDROID_NDK_HOME}" 
    ./gradlew assembleDebug --quiet
    echo "Compile Android Done!"
}

function build_macosx()
{   
    echo "Compiling CMake ... "
    NUM_OF_CORES=`getconf _NPROCESSORS_ONLN`
    cd $HOME/bin
    cmake_source=https://github.com/Kitware/CMake/releases/download/v3.17.0/cmake-3.17.0.tar.gz
    wget -t 5 --no-check-certificate $cmake_source -O cmake-mac.tar.gz -q
    tar xf cmake-mac.tar.gz 2>/dev/null
    cd cmake-3.17.0
    ./configure --prefix=$HOME/bin/cmake
    make -j $NUM_OF_CORES >/dev/null
    make install >/dev/null
    ls $HOME/bin/cmake
    export PATH=$HOME/bin/cmake/bin:$PATH
}

function mac_install_cmake()
{
    
    echo "Compiling MacOSX ... "
    cd  $COCOS2DX_ROOT/templates/js-template-link/frameworks/runtime-src
    mkdir build-mac 
    cd build-mac
    cmake .. -GXcode -DCOCOS_X_ROOT=$COCOS2DX_ROOT
    cmake --build . --config Release -- -quiet
    echo "Compile MacOSX Done!"
}

function run_compile()
{
    if [ "$BUILD_TARGET" == "android_cmake" ]; then
        setup_andorid_ndk
        build_android
    fi

    if [ "$BUILD_TARGET" == "macosx_cmake" ]; then
        mac_install_cmake
        build_macosx
    fi
}

run_compile

set +x

cd $COCOS2DX_ROOT/tools/travis-scripts
./generate-cocosfiles.sh $TRAVIS_BRANCH
exit 0
