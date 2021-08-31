# g++ -fPIC -shared

set -e
CRYPTO_INCLUDE=../../libOTe/cryptoTools
THIRD_INCLUDE=../../libOTe/cryptoTools/thirdparty/linux

#g++ -I./include -m64 -O2 -Wall -fPIC -shared -o mir-nom64 source/*.c
if [ -f "psi" ];then
    rm -rf psi
fi
if [ $? -ne 0 ]; then
    echo "===failed"
else
    echo "===succeed"
fi
#-DENABLE_CIRCUITS=OFF 
#-DENABLE_MIRACL=ON \
################C++11##############
BLAKE2_DIR=../../libOTe/cryptoTools/cryptoTools/Crypto/blake2
# g++ -g -std=c++11 -Wall -O2 -msse3 -msse2 -msse4.1 -maes -mpclmul \
g++ -fPIC -shared -std=c++11 -Wall -O2 -msse3 -msse2 -msse4.1 -maes -mpclmul \
-DENABLE_MIRACL \
-I${CRYPTO_INCLUDE} -I${THIRD_INCLUDE}/miracl \
-I${THIRD_INCLUDE}/miracl/miracl/include \
-I${BLAKE2_DIR} \
-I../ot \
-I./include \
${THIRD_INCLUDE}/miracl/miracl/source/*.c \
${CRYPTO_INCLUDE}/cryptoTools/Common/*.cpp \
${BLAKE2_DIR}/*.c \
${CRYPTO_INCLUDE}/cryptoTools/Crypto/*.cpp \
../ot/*.cpp \
./include/psi_c.cpp \
-o libpsi.so -fopenmp
# -lpthread \