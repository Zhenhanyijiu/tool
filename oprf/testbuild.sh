set -e
CRYPTO_INCLUDE=../libOTe/cryptoTools
THIRD_INCLUDE=../libOTe/cryptoTools/thirdparty/linux
BOOST_ROOT=../libOTe/cryptoTools/thirdparty/linux/boost
#${CRYPTO_INCLUDE}/cryptoTools/Common/*.cpp
#-mfma
#-DENABLE_NASM=OFF
#crytoTools 源码编译，链接静态库-lmiracl
####################################
# g++ -msse3 -msse2 -msse4.1 -maes -DENABLE_MIRACL=ON \
# -DENABLE_CIRCUITS=OFF -DENABLE_CPP_14=ON -DENABLE_FULL_GSL=ON \
# -Wall -I${CRYPTO_INCLUDE} -I${THIRD_INCLUDE}/boost -I${THIRD_INCLUDE}/miracl \
# -L${THIRD_INCLUDE}/miracl/miracl/source -L${BOOST_ROOT}/stage/lib test.cpp \
# ${CRYPTO_INCLUDE}/cryptoTools/Common/*.cpp \
# ${CRYPTO_INCLUDE}/cryptoTools/Crypto/*.cpp -lpthread \
# -lmiracl -lboost_system -lboost_thread -o test
####################################
#-lboost_system 

#crytoTools 源码编译，将miracl源码一起编译，不链接静态库
#g++ -I./include -m64 -O2 -Wall -fPIC -shared -o mir-nom64 source/*.c
if [ -f "test" ];then
    rm -rf test
fi
if [ $? -ne 0 ]; then
    echo "failed"
else
    echo "succeed"
fi
BLAKE2_DIR=../libOTe/cryptoTools/cryptoTools/Crypto/blake2
g++ -Wall -O2 -fPIC -shared -msse3 -msse2 -msse4.1 -maes -DENABLE_MIRACL=ON \
-DENABLE_CIRCUITS=OFF -DENABLE_CPP_14=ON -DENABLE_FULL_GSL=ON \
-I${CRYPTO_INCLUDE} -I${THIRD_INCLUDE}/miracl \
-I${THIRD_INCLUDE}/miracl/miracl/include \
-I${BLAKE2_DIR} \
-I./ot \
${THIRD_INCLUDE}/miracl/miracl/source/*.c \
${CRYPTO_INCLUDE}/cryptoTools/Common/*.cpp \
${BLAKE2_DIR}/*c \
${CRYPTO_INCLUDE}/cryptoTools/Crypto/*.cpp \
-lpthread \
./test-c-go/test_c.c -o ./test-c-go/libtest_c.so
