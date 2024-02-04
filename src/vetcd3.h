/**************************************************************************
vetcd3.h

    a etcd3 demo interface, for study purposec, for client tools, not for production
    all (grpc/protobuf/etcdv3) implement by protocol rewrite, no google grpc, protobuf depends

    Copyright (c) ly
**************************************************************************/
#ifndef __VETCD3_H__
#define __VETCD3_H__

#include <vector>
#include <string>
#include <map>
#include <unordered_map>
#include <stdint.h>

//NOTE: can replace lock and event with exists library
#include "ui_event.h"
#include "ui_lock.h"

struct vetcd_keyvalue
{
    std::string skey;
    std::string sval;
    int64_t ncreate_rev;
    int64_t nmod_rev;
    int64_t nver;
    int64_t nleaseid;
    int ninit_ttl;
    int ncur_ttl;
    vetcd_keyvalue(): ncreate_rev(0), nmod_rev(0), nver(0), nleaseid(0), ninit_ttl(0), ncur_ttl(0) {}
};

struct vetcd_member
{
    std::string sname;
    std::string scliurl;
    std::string speerurl;
    uint64_t nid;
    bool islearner;
    vetcd_member() : nid(0), islearner(false) {}
};

typedef int(*cb_vetcd_watch)(const std::string& stype, const std::string& skey, const std::string& sval, bool bcreate, int modver, void* userdata);

class http2_stream;

class vetcd3
{
public:
    vetcd3();
    ~vetcd3();

public:
    bool connect(const std::string& str, std::string& serr);
    bool login(const std::string& suser, const std::string& spwd, std::string& serr);
    bool connect_ex(const std::string& str, const std::string& suser, const std::string& spwd, std::string& serr);
    void close();
    // bmodify: true update and keep lease(ttl), false will put a new key(overwrite old key)
    int put(const std::string& skey, const std::string& sval, bool bmodify, std::string& serr);
    int put(const std::string& skey, const std::string& sval, int64_t leaseid, bool bmodify, std::string& serr);
    int get(const std::string& skey, std::string& sval, std::string& serr);
    // input ver is global version(create or modify version), not key self change version
    // return actual mod ver for next time query previous value
    // like svn number can be hole as 1, 2, 30, the actual modify version before 30 is 2 not 29
    int get_history(const std::string& skey, int ver, int& mod_ver, std::string& sval, std::string& serr);
    int del(const std::string& skey, std::string& serr);
    int ls_prefix(const std::string& skey, std::vector<vetcd_keyvalue>& arrout, std::string& serr);
    int ls_all(std::vector<vetcd_keyvalue>& arrout, std::string& serr);
    // bug, etcd server not return watch id, -1 failed, 0 ok
    int watch(const std::string& skey, void* cb, void* cbdata, int64_t& watchstreamid, std::string& serr);
    int stop_watch(int64_t watchstreamid, std::string& serr);
    int create_lease(int ttl, int64_t& leaseid, std::string& serr);
    int update_lease(int64_t leaseid, std::string& serr);
    int revoke_lease(int64_t leaseid, std::string& serr);
    int get_lease_ttl(int64_t leaseid, int& init_ttl, int& cur_ttl, std::string& serr);
    int ls_server_member(std::vector<vetcd_member>& arrout, std::string& serr);
    std::string get_token();

public:
    void run_recv_thread();

private:
    void add_stream(int64_t streamid, http2_stream* stm);
    bool remove_stream(int64_t streamid);

private:
    volatile bool bstop_;
    volatile int64_t cli_streamid_;

    int sock_;
    int readlen_;
    int svr_frame_size_;
    int svr_win_size_;
    char* reqbuf_;
    char* ansbuf_;

    std::string saddr_;
    std::string stoken_;
    std::map<int64_t, int> map_lease_;
    UiLock lock_mapstream_;
    std::unordered_map<int64_t, http2_stream*> map_stream_;

#ifdef _MSC_VER
    void* tid_;
#else
    pthread_t tid_;
#endif
};

#endif // __VETCD3_H__