#g++ -I. r.c Rectangle.cpp main.c -o main
###用于用c++源文件编译
#python3 setup.py build_ext --inplace
#cmath命令
#python3 setup.py build
#gcc test.c -fPIC -shared -o libtest.so
#LD_LIBRARY_PATH=. python3 main.py
################C++11##############
CRYPTO_INCLUDE=../../libOTe/cryptoTools
THIRD_INCLUDE=../../libOTe/cryptoTools/thirdparty/linux
BLAKE2_DIR=../../libOTe/cryptoTools/cryptoTools/Crypto/blake2
# g++ -g -std=c++11 -Wall -O2 -msse3 -msse2 -msse4.1 -maes -mpclmul \
g++ -fPIC -shared -std=gnu++11 -Wall -O2 -msse3 -msse2 -msse4.1 -maes -mpclmul \
-DENABLE_MIRACL \
-I${CRYPTO_INCLUDE} -I${THIRD_INCLUDE}/miracl \
-I${THIRD_INCLUDE}/miracl/miracl/include \
-I${BLAKE2_DIR} \
-I../ot \
${THIRD_INCLUDE}/miracl/miracl/source/*.c \
${CRYPTO_INCLUDE}/cryptoTools/Common/*.cpp \
${BLAKE2_DIR}/*.c \
${CRYPTO_INCLUDE}/cryptoTools/Crypto/*.cpp \
../ot/*.cpp -o libpsi.so