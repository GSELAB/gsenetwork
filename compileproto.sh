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


