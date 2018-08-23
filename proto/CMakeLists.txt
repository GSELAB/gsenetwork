
# compile proto file
#add_custom_command(TARGET proto
#  PRE_BUILD
#  COMMAND protoc --proto_path=proto --cpp_out=proto proto/PResult.proto proto/PNet.proto proto/PChain.proto
#  COMMENT "This command will be executed before build target proto"
#)

set (proto_source
      PResult.pb.cc
      PNet.pb.cc
      PChain.pb.cc)

#add_library (proto STATIC ${proto_source})

add_library (proto ${proto_source})