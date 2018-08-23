#
#
#
#
#
#



# 1. Install cmake system




# 2. Install protobuf

# if the protoc exist?
# if not, download it

# ./configure
# make
# make check
# sudo make install

#mkdir -p gen
protoc --proto_path=proto --cpp_out=proto proto/PResult.proto

protoc --proto_path=proto --cpp_out=proto proto/PNet.proto proto/PChain.proto


#3. Install boost (Move it to cmake file ? or create file deps-install.sh)
# download

# ./bootstrap.sh
# ./b2
# ./b2 install




# 4. Install secp256k1
# download

# $ ./autogen.sh
# $ ./configure
# $ make
# $ ./tests
# $ sudo make install  # optional



# 5. install cryptopp (??????????? not working)
# $ export CXXFLAGS="-DNDEBUG -g2 -O2 -stdlib=libc++"
# $ CXX=clang++ make -j 4
# $ make install

