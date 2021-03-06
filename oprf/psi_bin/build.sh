set -e
CRYPTO_INCLUDE=../../libOTe/cryptoTools
THIRD_INCLUDE=../../libOTe/cryptoTools/thirdparty/linux
#BOOST_ROOT=../libOTe/cryptoTools/thirdparty/linux/boost
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
# NOMP 不并行
# OMP_ONLY 仅使用omp并行，匹配不用线程池
# OMP_POOL 使用omp并行且匹配用线程池
################C++11##############
BLAKE2_DIR=../../libOTe/cryptoTools/cryptoTools/Crypto/blake2
# g++ -g -std=c++11 -Wall -O2 -msse3 -msse2 -msse4.1 -maes -mpclmul \
g++ -g -std=gnu++11 -Wall -O2 -msse3 -msse2 -msse4.1 -maes -mpclmul \
-DENABLE_MIRACL \
-DOMP_POOL -DNOSOCKET_TEST \
-I${CRYPTO_INCLUDE} -I${THIRD_INCLUDE}/miracl \
-I${THIRD_INCLUDE}/miracl/miracl/include \
-I${BLAKE2_DIR} \
-I../ot \
${THIRD_INCLUDE}/miracl/miracl/source/*.c \
${CRYPTO_INCLUDE}/cryptoTools/Common/*.cpp \
${BLAKE2_DIR}/*.c \
${CRYPTO_INCLUDE}/cryptoTools/Crypto/*.cpp \
../ot/*.cpp \
-lpthread \
psi_main.cpp -o psi -fopenmp
# -lpthread \
# ################C++14##############3
# BLAKE2_DIR=../../libOTe/cryptoTools/cryptoTools/Crypto/blake2
# g++ -g -Wall -O2 -msse3 -msse2 -msse4.1 -maes -DENABLE_MIRACL=ON \
# -DENABLE_CIRCUITS=OFF -DENABLE_CPP_14=ON -DENABLE_FULL_GSL=ON \
# -I${CRYPTO_INCLUDE} -I${THIRD_INCLUDE}/miracl \
# -I${THIRD_INCLUDE}/miracl/miracl/include \
# -I${BLAKE2_DIR} \
# -I../ot \
# ${THIRD_INCLUDE}/miracl/miracl/source/*.c \
# ${CRYPTO_INCLUDE}/cryptoTools/Common/*.cpp \
# ${BLAKE2_DIR}/*c \
# ${CRYPTO_INCLUDE}/cryptoTools/Crypto/*.cpp \
# ../ot/*cpp \
# -lpthread \
# psi_main.cpp -o psi
# ###################
# g++ -Wall -O2 -msse3 -msse2 -msse4.1 -maes -DENABLE_MIRACL=ON \
# -DENABLE_CIRCUITS=OFF -DENABLE_CPP_14=ON -DENABLE_FULL_GSL=ON \
# -DENABLE_NASM \
# -I${CRYPTO_INCLUDE} -I${THIRD_INCLUDE}/miracl \
# -I${THIRD_INCLUDE}/miracl/miracl/include \
# -I./ot \
# ${THIRD_INCLUDE}/miracl/miracl/source/*.c \
# ${CRYPTO_INCLUDE}/cryptoTools/Common/*.cpp \
# ${CRYPTO_INCLUDE}/cryptoTools/Crypto/*.cpp \
# ./ot/*cpp \
# -lpthread \
# test_np.cpp -o test_np