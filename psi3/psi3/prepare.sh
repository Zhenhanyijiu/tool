#当前目录是psi3/psi3/
##编译xxhash
echo "编译xxhash==============>"
cd ../libPaXoS/xxHash 
g++ -g -fPIC -shared -c ./*.c
ar rc libxxhash.a ./*.o
cd -



##boost 安装boost
echo "安装boost==============>"
cd ../thirdparty/linux
if [ ! -f "boost_1_64_0.tar.bz2" ]; then
    wget https://boostorg.jfrog.io/artifactory/main/release/1.64.0/source/boost_1_64_0.tar.bz2
fi
tar xfj boost_1_64_0.tar.bz2
mv boost_1_64_0 boost
cd ./boost
./bootstrap.sh
./b2 stage --with-system --with-thread link=static -mt cxxflags="-fPIC -shared"
mkdir includes
##cp -r boost includes/(或者用软链接也可以)
##用软链接
cd includes
ln -s ../boost boost
cd ../../../../psi3

##编译miracl
echo "编译miracl==============>"
cd ../thirdparty/linux/miracl/miracl/source/
bash linux64
cd -

##编译ntl
echo "编译ntl==============>"
cd ../thirdparty/linux/ntl/src
#make clean
make ./ntl.a 
mv ./ntl.a ./libntl.a
cd -
##安装依赖linbox,gmp,openblas,fflas-ffpack,givaro等(todo)



