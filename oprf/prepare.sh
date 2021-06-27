#告诉bash如果任何语句的执行结果不是true则应该退出
#set -o errexit
set -e
cd ..
if [ ! -d libOTe ]
then
    echo "======= downloading libOTe"
    git clone --recursive https://github.com/osu-crypto/libOTe.git
    cd libOTe
    git checkout a2bc326
    cd cryptoTools
    git checkout 2607541
    cd ../../    
fi
ls ./libOTe/cryptoTools/thirdparty/linux

if [ ! -d ./libOTe/cryptoTools/thirdparty/linux/miracl ]
then
    cd libOTe/cryptoTools/thirdparty/linux
    echo "======= downloading Miracl"
    git clone https://github.com/ladnir/miracl
    cd ../../../..
fi
# if [ ! -d ./libOTe/cryptoTools/thirdparty/linux/boost ]; then
#     cd libOTe/cryptoTools/thirdparty/linux
#     #        https://boostorg.jfrog.io/artifactory/main/release/1.69.0/source/boost_1_69_0.tar.bz2
#     wget -c 'https://boostorg.jfrog.io/artifactory/main/release/1.69.0/source/boost_1_69_0.tar.bz2' -O ./boost_1_69_0.tar.bz2
#     tar xfj boost_1_69_0.tar.bz2
#     mv boost_1_69_0 boost
#     rm  boost_1_69_0.tar.bz2
#     cd ../../../..
# fi
#有一个文件需要改动BitVector.h中，去掉头文件<cryptoTools/Network/IoBuffer.h>
cd oprf
ls .