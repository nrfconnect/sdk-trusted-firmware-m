#!/bin/sh

TFM_GIT_SHA=TF-Mv1.2.0
TFM_TESTS_GIT_SHA=TF-Mv1.2.0
PSA_ARCH_TESTS_GIT_SHA=v20.03_API1.0

if [ ! -d trusted-firmware-m/.git ]; then
   git clone https://git.trustedfirmware.org/TF-M/trusted-firmware-m.git -b ${TFM_GIT_SHA} --bare trusted-firmware-m/.git
   cd trusted-firmware-m
   git init
   git checkout -f
   cd ..
fi

if [ ! -d tf-m-tests/.git ]; then
   git clone https://git.trustedfirmware.org/TF-M/tf-m-tests.git -b ${TFM_TESTS_GIT_SHA} --bare tf-m-tests/.git
   cd tf-m-tests
   git init
   git checkout -f
   cd ..
fi

if [ ! -d psa-arch-tests/.git ]; then
   git clone https://github.com/ARM-software/psa-arch-tests.git -b ${PSA_ARCH_TESTS_GIT_SHA} --bare psa-arch-tests/.git
   cd psa-arch-tests
   git init
   git checkout -f
   cd ..
fi

