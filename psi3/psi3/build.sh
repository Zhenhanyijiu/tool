#该脚本编译出frontend.exe可执行文件
#当前目录有lib依赖库libcryptoTools.a、liblibOPRF.a、liblibOTe.a、liblibPaXoS.a
#否则先编译出上述依赖库
#-lboost_system
# -fPIC -shared
#g++ -O3 -DNDEBUG -O2 -g -O0 -ffunction-sections -Wall -Wfatal-errors \
#-maes -msse2 -msse4.1 -mpclmul -std=gnu++14 -pthread \
#-I ./cryptoTools \
#-I ./thirdparty/linux/boost/includes \
#-I ./libOPRF \
#-I ./libOTe \
#-I ./libPaXoS \
#-I ./thirdparty/linux/miracl/ \
#-I ./thirdparty/linux/ntl/include/ \
#-I ./frontend \
#-L ./thirdparty/linux/boost/stage/lib \
#-L lib \
#-L thirdparty/linux/miracl/miracl/source \
#-L thirdparty/linux/ntl/src/ \
#frontend/CLP.cpp \
#frontend/OtBinMain.cpp \
#frontend/OtBinMain.v2.cpp \
#frontend/bitPosition.cpp \
#frontend/miraclTest.cpp \
#frontend/util.cpp \
#./lib/liblibPaXoS.a \
#./lib/liblibOTe.a \
#./lib/liblibOPRF.a \
#-lcryptoTools -lmiracl -lntl -lboost_system \
#./frontend/main.cpp -o frontend.exe \

# -lcryptoTools -llibOPRF -llibOTe -lmiracl -lntl \
#nasm -f elf64 ../cryptoTools/Crypto/asm/sha_lnx.S -o ../cryptoTools/Crypto/asm/sha_lnx.S.o
#../cryptoTools/Crypto/asm/sha_lnx.S.o \
# -DNO_INTEL_ASM_SHA1=1 -DTEST_PSI3 \
#-fPIC -shared
#./b2 stage --with-system --with-thread link=static -mt -fPIC -shared
#-lboost_system 

g++ -O3 -DNDEBUG -O2 -g -O0 -ffunction-sections -Wall -Wfatal-errors \
-maes -msse2 -msse3 -msse4.1 -mpclmul -std=c++11 -pthread \
-DNO_INTEL_ASM_SHA1=1 -DTEST_PSI3 \
-I ../psi3 \
-I ../cryptoTools \
-I ../thirdparty/linux/boost/includes \
-I /tmp/include \
-I ../libOPRF \
-I ../libOTe \
-I ../libPaXoS \
-I ../thirdparty/linux/miracl/ \
-I ../thirdparty/linux/ntl/include/ \
-L ../thirdparty/linux/boost/stage/lib \
-L ../thirdparty/linux/miracl/miracl/source \
-L ../thirdparty/linux/ntl/src/ \
-L /tmp/lib \
../cryptoTools/Network/*.cpp \
../cryptoTools/Crypto/*.cpp \
../cryptoTools/Common/*.cpp \
../libOTe/TwoChooseOne/*.cpp \
../libOTe/NChooseOne/*.cpp \
../libOTe/Base/*.cpp \
../libOTe/Tools/*.cpp \
../libOPRF/OPPRF/*.cpp \
../libOPRF/Hashing/*.cpp \
../libPaXoS/ObliviousDictionary.cpp \
../libPaXoS/gf2e_mat_solve.cpp \
../psi3/*.cpp \
../libPaXoS/xxHash/libxxhash.a \
/tmp/lib/liblinbox.a \
/tmp/lib/libgivaro.a \
/tmp/lib/libgmpxx.a \
/tmp/lib/libopenblas.a \
/tmp/lib/libgmp.a \
-lmiracl -lntl -lboost_system \
-o psi3
# -o libpsi3.so
#-llinbox
# -lboost_system 
# -lopenblas  -lgmp -lgomp \
#-I ../thirdparty/linux/linbox/build/fflas-ffpack \
# -I ../thirdparty/linux/linbox \
# -I ../thirdparty/linux/linbox/build/fflas-ffpack \
#-llinbox -lgivaro -lopenblas -lgmp \
# /tmp/lib/liblinbox.a \
# /tmp/lib/libgivaro.a \
# /tmp/lib/libgmpxx.a \
# /tmp/lib/libopenblas.a \
# /tmp/lib/libgmp.a \
#-lgomp 