vetcd3.h

    a etcd3 demo interface, for study purpose, for client tools, not for production
    all (grpc/protobuf/etcdv3) implement by protocol rewrite
	no submodules, no thirdparty, no google grpc, no protobuf, no proto files

    Copyright (c) ly
	

# about this

for study purpose, will not update
学习了解etcdv3协议, 学习http2/grpc协议, 学习protobuf格式, 手写的库.
请勿用于生产, 丢在github是发现找不到不依赖于google全家桶的C++库, 简单修改可跨平台, 做做工具挺好

test version
	etcd 3.4
	etcd 3.5

# compile
require:
	std <= C++11  (vs2013)

windows:
	open test/test_vetcd.vcxproj
	
linux/macos:
	maybe easy to support, write a makefile yourself

# reference

hpack.hpp
	forgot where to get it...

etcdv3 proto defines
	officer site

grpc/http2/protobuf protocol
	search

etcdv3api
	https://github.com/Proheeler/etcdv3API