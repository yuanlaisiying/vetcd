// testMyFix.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
//#include "stdafx.h"
#include "vetcd3.h"
#include <time.h>
#include <iostream>

#ifdef _MSC_VER
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mstcpip.h>
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")
#define MSG_DONTWAIT 0
#else
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>

#include <string>
#include <vector>
#include <map>
#include <errno.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <netinet/tcp.h>
#include <sys/mman.h>
#define closesocket close
#define Sleep sleep

#ifdef __linux__
#include <error.h>
#endif

#endif

#include "hpack.hpp"

#pragma pack(push, 1)
struct uint3_t
{
    char data[3];
    uint3_t()
    {
        memset(this, 0, 3);
    }
    int getint()
    {
        int rs = 0;
        char* pdata = (char*)&rs;
        pdata[2] = data[0];
        pdata[1] = data[1];
        pdata[0] = data[2];
        return rs;
    }
};

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4200)
#endif

struct pkt_head
{
    uint3_t nlen;  // payload data length (not include this header size 9 bytes)
    char ntype;    // frame type
    char nflags;   // 
    int nstream;   // stream id
    char data[0];
    pkt_head()
    {
        memset(this, 0, sizeof(pkt_head));
    }
};

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#pragma pack(pop)



#define GRPC_HEAD_SIZE 9



int32_t vetcd_ReverseBytes32(int32_t value)
{
    return (value & 0x000000FF) << 24 | (value & 0x0000FF00) << 8 |
        (value & 0x00FF0000) >> 8 | (value & 0xFF000000) >> 24;
}

int16_t vetcd_ReverseBytes16(int16_t value)
{
    return (value & 0xFF) << 8 | (value & 0xFF00) >> 8;
}

void conv_pkthead(pkt_head* phead)
{
    phead->nstream = vetcd_ReverseBytes32(phead->nstream);
}

int vetcd_send_sock(int sock, const char* buffer, uint32_t size)
{
    int index = 0, ret;
    while (size)
    {
        ret = (int)send(sock, &buffer[index], (int)size, 0);
        if (ret <= 0)
        {
            if (ret == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
                continue;
            printf("send failed, code: %d, error: %s\n", errno, strerror(errno));
            return (!ret) ? index : -1;
        }
        index += ret;
        size -= ret;
    }
    return index;
}

int vetcd_connect_to_ip(const char* ip, int port)
{
    int sockfd = (int)socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd <= 0)
    {
        return -1;
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons((u_short)port);
    serv_addr.sin_addr.s_addr = inet_addr(ip);
    return !connect(sockfd, (const sockaddr*)& serv_addr, sizeof(serv_addr)) ? sockfd : -1;
}



int pb_read_int(char* data, int len, uint64_t& val)
{
    uint64_t bg = 0;
    int i = 0;
    while (i < len)
    {
        uint64_t c = data[i];
        if (c < 0x80)
        {
            bg |= c << (i * 7);
            i++;
            break;
        }
        bg |= ((c & 0x7F) << (i * 7));
        i++;
    }
    val = bg;
    return i;
}

int pb_write_int(char* data, uint64_t v)
{
    if (v == 0)
    {
        *data = 0;
        return 1;
    }
    int i = 0;
    while (v != 0)
    {
        unsigned char d = v & 0x7F;
        if (v >= 0x80) d |= 0x80;
        data[i] = d;
        i++;
        v = v >> 7;
    }
    return i;
}

int pb_calc_int(uint64_t v)
{
    if (v == 0)
        return 1;
    unsigned char data[10];
    int i = 0;
    while (v != 0)
    {
        unsigned char d = v & 0x7F;
        if (v >= 0x80) d |= 0x80;
        data[i] = d;
        i++;
        v = v >> 7;
    }
    return i;
}

#define STEAM_TYPE_DATA      0
#define STEAM_TYPE_HEADER    1
#define STEAM_TYPE_2         2 // PRIORITY
#define STEAM_TYPE_RST       3
#define STEAM_TYPE_SETTING   4
#define STEAM_TYPE_5         5 // PUSH_PROMISE
#define STEAM_TYPE_PING      6
#define STEAM_TYPE_7         7 // GOAWAY
#define STEAM_TYPE_WIN       8
#define STEAM_TYPE_9         9 // CONTINUATION

#define GRPC_FIELD_INT      0
#define GRPC_FIELD_BYTES    2


std::string etcd_typename(int ntype)
{
    switch (ntype)
    {
    case 0: return "data";
    case 1: return "header";
    case 2: return "priority";
    case 3: return "rst-stream";
    case 4: return "setting";
    case 5: return "push-promise";
    case 6: return "ping";
    case 7: return "goaway";
    case 8: return "window";
    case 9: return "continuation";
    }
    return "unknown:" + std::to_string(ntype);
}

class magic_stream
{
public:
    static int fill(char* buf)
    {
        std::string smagic = "PRI * HTTP/2.0\r\n\r\nSM\r\n\r\n";
        memcpy(buf, smagic.c_str(), smagic.length());
        return (int)smagic.length();
    }
};

class setting_stream
{
public:
    char* data_;
    char* head_;
    int len_;
public:
    setting_stream(char* buf)
    {
        len_ = 0;
        data_ = buf + 9;
        head_ = buf;
        memset(buf, 0, 9);
    }
    void fill_item(short ntype, int nval)
    {
        *(int16_t*)data_ = vetcd_ReverseBytes16(ntype);
        *(int32_t*)&data_[2] = vetcd_ReverseBytes32(nval);
        //memcpy(&data_[2], &nval, 4);
        data_ += 6;
        len_ += 6;
    }
    int fill_over()
    {
        int total = len_ + 9;
        char* plen = (char *)&len_;
        head_[0] = plen[2];
        head_[1] = plen[1];
        head_[2] = plen[0];
        head_[3] = 0x04;
        return total;
    }
};

class window_stream
{
public:
    static int fill(char* buf, int streamid, int val)
    {
        // 3 pkt len
        // 1 type 0x06
        // 1 flag
        // 4 data len 4
        // total 13
        int len = 13;
        memset(buf, 0, len);
        buf[2] = 0x04;
        buf[3] = 0x08;
        *(int*)&buf[5] = vetcd_ReverseBytes32(streamid);
        *(int*)&buf[9] = vetcd_ReverseBytes32(val);
        return len;
    }
    static int parse(char* buf, int len, int& win, int defwin)
    {
        win = defwin;
        pkt_head* head = (pkt_head*)buf;
        if (len < GRPC_HEAD_SIZE)
            return 0;
        if (len < GRPC_HEAD_SIZE + head->nlen.getint())
            return 0;
        if (head->ntype != 0x08)
            return 0;
        win = vetcd_ReverseBytes32(*(int*)head->data);
        return head->nlen.getint() + GRPC_HEAD_SIZE;
    }
};

class ping_stream
{
public:
    static int fill(char* buf)
    {
        // 3 pkt len
        // 1 type 0x06
        // 1 flag
        // 4 reserve
        // 8 data len 8
        // total 17
        int len = 17;
        memset(buf, 0, len);
        buf[2] = 0x08;
        buf[3] = 0x06;
        return len;
    }
};

class parse_setting_stream
{
public:
    std::map<short, int> mapitem_;
public:
    parse_setting_stream() {};
    int parse(char* buf, int len)
    {
        if (len < GRPC_HEAD_SIZE)
            return 0;
        pkt_head* head = (pkt_head*)buf;
        if (head->ntype != 0x04)
            return 0;
        int len2 = head->nlen.getint();
        if (len2 == 0)
            return GRPC_HEAD_SIZE;
        if (len2 % 6 != 0)
        {
            return -1;
        }
        if (head->nflags == 1) // ignore ack
        {
            return len2 + GRPC_HEAD_SIZE;
        }
        char* data = buf + GRPC_HEAD_SIZE;
        for (int i = 0; i < len2/6; i ++)
        {
            short nk = vetcd_ReverseBytes16(*(int16_t  *)data);
            int nv = vetcd_ReverseBytes32(*(int*)&data[2]);
            mapitem_[nk] = nv;
            data += 6;
        }
        return len2 + GRPC_HEAD_SIZE;
    }
    int get(short nk, int def)
    {
        auto it = mapitem_.find(nk);
        if (it != mapitem_.end())
            return it->second;
        return def;
    }
};

class header_stream
{
public:
    char* data_;
    char* head_;
    int len_;
public:
    header_stream(char* buf, int streamid)
    {
        // 3 pkt len
        // 1 type 0x01
        // 1 flag 0x04 end headers
        // 4 reserve
        // 8 data len 8
        // total 17
        //int len = 0;
        memset(buf, 0, 9);
        // buf head
        buf[3] = 0x01;
        buf[4] = 0x04;
        *(int*)&buf[5] = vetcd_ReverseBytes32(streamid);
        data_ = buf + 9;
        head_ = buf;
        len_ = 0;
        data_[0] = (char)0x86; // schema http
        len_ += 1;
        data_++;
        data_[0] = (char)0x83; // method post
        len_ += 1;
        data_++;
    }
    void fill_item(const std::string& skey, const std::string& sval)
    {
        data_[0] = 0x40; // @
        len_++;
        data_++;
        // key length 1 byte
        data_[0] = (char)skey.length();
        len_++;
        data_++;
        // key content
        memcpy(data_, skey.c_str(), skey.length());
        data_ += skey.length();
        len_ += (int)skey.length();
        // val length 1 byte
        data_[0] = (char)sval.length();
        len_++;
        data_++;
        // va; content
        memcpy(data_, sval.c_str(), sval.length());
        data_ += sval.length();
        len_ += (int)sval.length();
    }
    int fill_over()
    {
        int total = len_ + 9;
        char* plen = (char*)&len_;
        head_[0] = plen[2];
        head_[1] = plen[1];
        head_[2] = plen[0];
        return total;
    }
};

class put_data_stream
{
public:
    static int make_grpc_packet(std::string& sout, const std::string& skey, const std::string& sval, uint64_t lease = 0, bool bmodify = false)
    {
        char* buf = new char[skey.length() + sval.length() + 512];
        char* data = buf;
        // compress: not
        *data = '\0';
        data += 1;
        // pblen
        data += 4; // skip 4 bytes as pb len, modify at last
        // pb start
        // PutRequest
        int pblen = 0;
        {
            // field
            int fid = 1;
            int ftype = 2;
            int tag = (fid << 3) + ftype;
            *data = (char)tag;
            data += 1;
            pblen++;
            // length
            tag = pb_write_int(data, skey.length());
            data += tag;
            pblen += tag;
            // data
            memcpy(data, skey.c_str(), skey.length());
            data += skey.length();
            pblen += (int)skey.length();
        }
        {
            // field
            int fid = 2;
            int ftype = 2;
            int tag = (fid << 3) + ftype;
            *data = (char)tag;
            data++;
            pblen++;
            // length
            tag = pb_write_int(data, sval.length());
            data += tag;
            pblen += tag;
            // data
            memcpy(data, sval.c_str(), sval.length());
            data += sval.length();
            pblen += (int)sval.length();
        }
        if (lease != 0)
        {
            // field
            int fid = 3;
            int ftype = 0;
            int tag = (fid << 3) + ftype;
            *data = (char)tag;
            data++;
            pblen++;
            // data
            tag = pb_write_int(data, lease);
            data += tag;
            pblen += tag;
        }
        else if (bmodify)// set ignore lease then update not remove lease
        {
            int fid = 6;
            int ftype = 0;
            int tag = (fid << 3) + ftype;
            *data = (char)tag;
            data++;
            pblen++;
            // data
            tag = pb_write_int(data, 1);
            data += tag;
            pblen += tag;
        }
        // update grpc len
        data = &buf[1];
        *(int*)data = vetcd_ReverseBytes32(pblen);
        sout.assign(buf, pblen + 5);
        delete[] buf;
        return 0;
    }
    static int fill(char* buf, int streamid, const std::string& skey, const std::string& sval, uint64_t lease = 0, bool bmodify = false)
    {
        char* data = buf;
        memset(data, 0, GRPC_HEAD_SIZE);
        data[3] = 0x00; // data stream
        //data[4] = 0x01; // end stream
        *(int*)&data[5] = vetcd_ReverseBytes32(streamid);
        // grpc start
        data = &buf[9];
        data[0] = 0; // not compress
        data++;
        //*(int*)data = ReverseBytes32(2);
        data += 4;
        // pb start
        // PutRequest
        int pblen = 0;
        {
            // field
            int fid = 1;
            int ftype = 2;
            int tag = (fid << 3) + ftype;
            *data = (char)tag;
            data++;
            pblen++;
            // length
            tag = pb_write_int(data, skey.length());
            data += tag;
            pblen += tag;
            // data
            memcpy(data, skey.c_str(), skey.length());
            data += skey.length();
            pblen += (int)skey.length();
        }
        {
            // field
            int fid = 2;
            int ftype = 2;
            int tag = (fid << 3) + ftype;
            *data = (char)tag;
            data++;
            pblen++;
            // length
            tag = pb_write_int(data, sval.length());
            data += tag;
            pblen += tag;
            // data
            memcpy(data, sval.c_str(), sval.length());
            data += sval.length();
            pblen += (int)sval.length();
        }
        if (lease != 0)
        {
            // field
            int fid = 3;
            int ftype = 0;
            int tag = (fid << 3) + ftype;
            *data = (char)tag;
            data++;
            pblen++;
            // data
            tag = pb_write_int(data, lease);
            data += tag;
            pblen += tag;
        }
        else if (bmodify)// set ignore lease then update not remove lease
        {
            int fid = 6;
            int ftype = 0;
            int tag = (fid << 3) + ftype;
            *data = (char)tag;
            data++;
            pblen++;
            // data
            tag = pb_write_int(data, 1);
            data += tag;
            pblen += tag;
        }
        // update grpc len
        data = &buf[10];
        *(int*)data = vetcd_ReverseBytes32(pblen);
        // update head len
        int len = 5 + pblen;
        char* plen = (char*)&len;
        buf[0] = plen[2];
        buf[1] = plen[1];
        buf[2] = plen[0];
        buf[4] = 0x01; // end stream
        return GRPC_HEAD_SIZE + 5 + pblen;
    }
};

class get_data_stream
{
public:
    static int fill(char* buf, int streamid, const std::string& skey, bool brange, int ver = 0)
    {
        char* data = buf;
        memset(data, 0, GRPC_HEAD_SIZE);
        data[3] = 0x00; // data stream
        data[4] = 0x01; // end stream
        *(int*)&data[5] = vetcd_ReverseBytes32(streamid);
        // grpc start
        data = &buf[9];
        data[0] = 0; // not compress
        int pblen = 0;
        data += 5;
        // pb start
        // field 1 key
        {
            int fid = 1;
            int ftype = 2;
            int tag = (fid << 3) + ftype;
            *data = (char)tag;
            pblen++;
            data++;
            if (skey.empty())
            {
                int tag_size = pb_write_int(data, 1);
                data += tag_size;
                pblen += tag_size;
                *data = 0;
                data += 1;
                pblen += 1;
            }
            else
            {
                int tag_size = pb_write_int(data, skey.length());
                data += tag_size;
                pblen += tag_size;
                memcpy(data, skey.c_str(), skey.length());
                data += skey.length();
                pblen += (int)skey.length();
            }
        }
        if (brange)
        {
            int fid = 2;
            int ftype = 2;
            int tag = (fid << 3) + ftype;
            *data = (char)tag;
            pblen++;
            data++;
            if (skey.empty())
            {
                int tag_size = pb_write_int(data, 1);
                data += tag_size;
                pblen += tag_size;
                *data = 0;
                data += 1;
                pblen += 1;
            }
            else
            {
                int tag_size = pb_write_int(data, skey.length());
                data += tag_size;
                pblen += tag_size;
                memcpy(data, skey.c_str(), skey.length());
                data += skey.length();
                pblen += (int)skey.length();
                char* data2 = data - 1;
                *data2 = *data2 + 1;
            }
        }
        if (ver != 0) // try to get histroy, may error if has compact
        {
            int fid = 4;
            int ftype = 0;
            int tag = (fid << 3) + ftype;
            *data = (char)tag;
            data++;
            pblen++;
            // data
            tag = pb_write_int(data, ver);
            pblen += tag;
            data += tag;
        }
        // field 7 serializable
        {
            int fid = 7;
            int ftype = 0;
            int tag = (fid << 3) + ftype;
            *data = (char)tag;
            pblen++;
            data++;
            int tag_size = pb_write_int(data, 1);
            pblen += tag_size;
            data += tag_size;
        }
        // update grpc len
        data = &buf[10];
        *(int*)data = vetcd_ReverseBytes32(pblen);
        // update head len
        int len = 5 + pblen;
        char* plen = (char*)&len;
        buf[0] = plen[2];
        buf[1] = plen[1];
        buf[2] = plen[0];
        return GRPC_HEAD_SIZE + 5 + pblen;
    }
};

class delete_data_stream
{
public:
    static int fill(char* buf, int streamid, const std::string& skey)
    {
        char* data = buf;
        memset(data, 0, GRPC_HEAD_SIZE);
        data[3] = 0x00; // data stream
        data[4] = 0x01; // end stream
        *(int*)&data[5] = vetcd_ReverseBytes32(streamid);
        // grpc start
        data = &buf[9];
        data[0] = 0; // not compress
        int pblen = 0;
        data += 5;
        // pb start
        // field 1 key
        {
            int fid = 1;
            int ftype = 2;
            int tag = (fid << 3) + ftype;
            *data = (char)tag;
            pblen++;
            data++;
            if (skey.empty())
            {
                int tag_size = pb_write_int(data, 1);
                data += tag_size;
                pblen += tag_size;
                *data = 0;
                data += 1;
                pblen += 1;
            }
            else
            {
                int tag_size = pb_write_int(data, skey.length());
                data += tag_size;
                pblen += tag_size;
                memcpy(data, skey.c_str(), skey.length());
                data += skey.length();
                pblen += (int)skey.length();
            }
        }
        // update grpc len
        data = &buf[10];
        *(int*)data = vetcd_ReverseBytes32(pblen);
        // update head len
        int len = 5 + pblen;
        char* plen = (char*)&len;
        buf[0] = plen[2];
        buf[1] = plen[1];
        buf[2] = plen[0];
        return GRPC_HEAD_SIZE + 5 + pblen;
    }
};

class watch_data_stream
{
public:
    static int fill_rst(char* buf, int streamid)
    {
        char* data = buf;
        memset(buf, 0, GRPC_HEAD_SIZE + 4);
        data[2] = 0x04;
        data[3] = STEAM_TYPE_RST;
        //data[4] = 0x01; // end stream
        *(int*)&data[5] = vetcd_ReverseBytes32(streamid);
        return GRPC_HEAD_SIZE + 4;
    }
    static int fill(char* buf, int streamid, const std::string& skey)
    {
        char* data = buf;
        memset(data, 0, GRPC_HEAD_SIZE);
        data[3] = STEAM_TYPE_DATA; // data stream
        data[4] = 0x01; // end stream
        *(int*)&data[5] = vetcd_ReverseBytes32(streamid);
        // grpc start
        data = &buf[9];
        data[0] = 0; // not compress
        int pblen = 0;
        data += 5;
        // pb start
        // WatchRequest
        {
            // field
            int fid = 1;
            int ftype = 2;
            int tag = (fid << 3) + ftype;
            *data = (char)tag;
            pblen++;
            data++;
            // data
            int keylen = (int)skey.length();
            if (keylen == 0) keylen = 1;
            int sublen = 2 + pb_calc_int(keylen)*2 + keylen*2;
            tag = pb_write_int(data, sublen);
            pblen += tag;
            data += tag;
        }
        // WatchCreateRequest
        // field 1 key
        {
            int fid = 1;
            int ftype = 2;
            int tag = (fid << 3) + ftype;
            *data = (char)tag;
            pblen++;
            data++;
            if (skey.empty())
            {
                int tag_size = pb_write_int(data, 1);
                data += tag_size;
                pblen += tag_size;
                *data = 0;
                data += 1;
                pblen += 1;
            }
            else
            {
                int tag_size = pb_write_int(data, skey.length());
                data += tag_size;
                pblen += tag_size;
                memcpy(data, skey.c_str(), skey.length());
                data += skey.length();
                pblen += (int)skey.length();
            }
        }
        // fid 2 range
        {
            int fid = 2;
            int ftype = 2;
            int tag = (fid << 3) + ftype;
            *data = (char)tag;
            pblen++;
            data++;
            if (skey.empty())
            {
                int tag_size = pb_write_int(data, 1);
                data += tag_size;
                pblen += tag_size;
                *data = 0;
                data += 1;
                pblen += 1;
            }
            else
            {
                int tag_size = pb_write_int(data, skey.length());
                data += tag_size;
                pblen += tag_size;
                memcpy(data, skey.c_str(), skey.length());
                data += skey.length();
                pblen += (int)skey.length();
                char* data2 = data - 1;
                *data2 = *data2 + 1;
            }
        }
        // fid 7 watch id (not in proto file)
        {
            static int watchid = 0;
            watchid++;
            int fid = 7;
            int ftype = 0;
            int tag = (fid << 3) + ftype;
            *data = (char)tag;
            pblen++;
            data++;
            tag = pb_write_int(data, watchid);
            pblen += tag;
            data += tag;
        }
        // update grpc len
        data = &buf[10];
        *(int*)data = vetcd_ReverseBytes32(pblen);
        // update head len
        int len = 5 + pblen;
        char* plen = (char*)&len;
        buf[0] = plen[2];
        buf[1] = plen[1];
        buf[2] = plen[0];
        return GRPC_HEAD_SIZE + 5 + pblen;
    }
    static int fill_cancel(char* buf, int streamid, int64_t watchid)
    {
        char* data = buf;
        memset(data, 0, GRPC_HEAD_SIZE);
        data[3] = 0x00; // data stream
        data[4] = 0x01; // end stream
        *(int*)&data[5] = vetcd_ReverseBytes32(streamid);
        // grpc start
        data = &buf[9];
        data[0] = 0; // not compress
        int pblen = 0;
        data += 5;
        // pb start
        // WatchRequest
        {
            // field 2
            int fid = 2;
            int ftype = 2;
            int tag = (fid << 3) + ftype;
            *data = (char)tag;
            pblen++;
            data++;
            // data
            int sublen = 1 + pb_calc_int(watchid);
            tag = pb_write_int(data, sublen);
            pblen += tag;
            data += tag;
        }
        // WatchCancelRequest
        // field 1
        {
            int fid = 1;
            int ftype = 0;
            int tag = (fid << 3) + ftype;
            *data = (char)tag;
            pblen++;
            data++;
            tag = pb_write_int(data, watchid);
            pblen += tag;
            data += tag;
        }
        // update grpc len
        data = &buf[10];
        *(int*)data = vetcd_ReverseBytes32(pblen);
        // update head len
        int len = 5 + pblen;
        char* plen = (char*)&len;
        buf[0] = plen[2];
        buf[1] = plen[1];
        buf[2] = plen[0];
        return GRPC_HEAD_SIZE + 5 + pblen;
    }
};

class member_data_stream
{
public:
    static int fill(char* buf, int streamid)
    {
        int len = 5;
        memset(buf, 0, GRPC_HEAD_SIZE + len);

        char* data = buf;
        data[3] = 0x00; // data stream
        data[4] = 0x01; // end stream
        *(int*)&data[5] = vetcd_ReverseBytes32(streamid);

        char* plen = (char*)&len;
        buf[0] = plen[2];
        buf[1] = plen[1];
        buf[2] = plen[0];
        return GRPC_HEAD_SIZE + len;
    }
};

class auth_data_stream
{
public:
    static int fill(char* buf, int streamid, const std::string& suser, const std::string& spwd)
    {
        char* data = buf;
        memset(data, 0, GRPC_HEAD_SIZE);
        data[3] = 0x00; // data stream
        data[4] = 0x01; // end stream
        *(int*)&data[5] = vetcd_ReverseBytes32(streamid);
        // grpc start
        data = &buf[9];
        data[0] = 0; // not compress
        int pblen = 0;
        data += 5;
        // pb start
        // field 1 user
        {
            int fid = 1;
            int ftype = 2;
            int tag = (fid << 3) + ftype;
            *data = (char)tag;
            pblen++;
            data++;
            if (suser.empty())
            {
                int tag_size = pb_write_int(data, 1);
                data += tag_size;
                pblen += tag_size;
                *data = 0;
                data += 1;
                pblen += 1;
            }
            else
            {
                int tag_size = pb_write_int(data, suser.length());
                data += tag_size;
                pblen += tag_size;
                memcpy(data, suser.c_str(), suser.length());
                data += suser.length();
                pblen += (int)suser.length();
            }
        }
        // field 2 pwd
        {
            int fid = 2;
            int ftype = 2;
            int tag = (fid << 3) + ftype;
            *data = (char)tag;
            pblen++;
            data++;
            if (spwd.empty())
            {
                int tag_size = pb_write_int(data, 1);
                data += tag_size;
                pblen += tag_size;
                *data = 0;
                data += 1;
                pblen += 1;
            }
            else
            {
                int tag_size = pb_write_int(data, spwd.length());
                data += tag_size;
                pblen += tag_size;
                memcpy(data, spwd.c_str(), spwd.length());
                data += spwd.length();
                pblen += (int)spwd.length();
            }
        }
        
        // update grpc len
        data = &buf[10];
        *(int*)data = vetcd_ReverseBytes32(pblen);
        // update head len
        int len = 5 + pblen;
        char* plen = (char*)&len;
        buf[0] = plen[2];
        buf[1] = plen[1];
        buf[2] = plen[0];
        return GRPC_HEAD_SIZE + 5 + pblen;
    }
};

class lease_data_stream
{
public:
    static int fill(char* buf, int streamid, int ttl)
    {
        char* data = buf;
        memset(data, 0, GRPC_HEAD_SIZE);
        data[3] = 0x00; // data stream
        data[4] = 0x01; // end stream
        *(int*)&data[5] = vetcd_ReverseBytes32(streamid);
        // grpc start
        data = &buf[9];
        data[0] = 0; // not compress
        data++;
        //*(int*)data = ReverseBytes32(2);
        data += 4;
        // pb start
        // LeaseGrantRequest
        int pblen = 0;
        {
            // field
            int fid = 1;
            int ftype = 0;
            int tag = (fid << 3) + ftype;
            *data = (char)tag;
            data++;
            pblen++;
            // data
            tag = pb_write_int(data, ttl);
            pblen += tag;
            data += tag;
        }
        // update grpc len
        data = &buf[10];
        *(int*)data = vetcd_ReverseBytes32(pblen);
        // update head len
        int len = 5 + pblen;
        char* plen = (char*)&len;
        buf[0] = plen[2];
        buf[1] = plen[1];
        buf[2] = plen[0];
        return GRPC_HEAD_SIZE + 5 + pblen;
    }
};

class leasekeepalive_data_stream
{
public:
    static int fill(char* buf, int streamid, int64_t leaseid)
    {
        char* data = buf;
        memset(data, 0, GRPC_HEAD_SIZE);
        data[3] = 0x00; // data stream
        data[4] = 0x01; // end stream
        *(int*)&data[5] = vetcd_ReverseBytes32(streamid);
        // grpc start
        data = &buf[9];
        data[0] = 0; // not compress
        data++;
        //*(int*)data = ReverseBytes32(2);
        data += 4;
        // pb start
        // LeaseKeepAliveRequest
        int pblen = 0;
        {
            // field
            int fid = 1;
            int ftype = 0;
            int tag = (fid << 3) + ftype;
            *data = (char)tag;
            data++;
            pblen++;
            // data
            tag = pb_write_int(data, leaseid);
            pblen += tag;
            data += tag;
        }
        // update grpc len
        data = &buf[10];
        *(int*)data = vetcd_ReverseBytes32(pblen);
        // update head len
        int len = 5 + pblen;
        char* plen = (char*)&len;
        buf[0] = plen[2];
        buf[1] = plen[1];
        buf[2] = plen[0];
        return GRPC_HEAD_SIZE + 5 + pblen;
    }
};

class leasettl_data_stream
{
public:
    static int fill(char* buf, int streamid, int64_t leaseid, bool bgetkeys)
    {
        char* data = buf;
        memset(data, 0, GRPC_HEAD_SIZE);
        data[3] = 0x00; // data stream
        data[4] = 0x01; // end stream
        *(int*)&data[5] = vetcd_ReverseBytes32(streamid);
        // grpc start
        data = &buf[9];
        data[0] = 0; // not compress
        data++;
        //*(int*)data = ReverseBytes32(2);
        data += 4;
        // pb start
        // LeaseTimeToLiveRequest
        int pblen = 0;
        {
            // field
            int fid = 1;
            int ftype = 0;
            int tag = (fid << 3) + ftype;
            *data = (char)tag;
            data++;
            pblen++;
            // data
            tag = pb_write_int(data, leaseid);
            pblen += tag;
            data += tag;
        }
        // not suppport get keys
        // update grpc len
        data = &buf[10];
        *(int*)data = vetcd_ReverseBytes32(pblen);
        // update head len
        int len = 5 + pblen;
        char* plen = (char*)&len;
        buf[0] = plen[2];
        buf[1] = plen[1];
        buf[2] = plen[0];
        return GRPC_HEAD_SIZE + 5 + pblen;
    }
};

class http2_stream
{
public:
    volatile int64_t ref_;
    ui_event evt;
    int error_code = -1; // grpc 0 ok, other > 0 failed
    std::string error_message;
    std::string sdata;
public:
    http2_stream():ref_(1){};
    virtual ~http2_stream() {};
    virtual int parse(char* buf, int len, HPACK::decoder_t& headdec) { (void)buf; (void)headdec; return len; }
    virtual bool wait(int timeout_ms = 10000) { return evt.Wait(timeout_ms); }
    void notify_response() { evt.Set(); }
    void AddRef()
    {
#ifdef _MSC_VER
        ::InterlockedIncrement64(&ref_);
#else
        __sync_fetch_and_add(&ref_, 1);
#endif
    }
    void Release()
    {
#ifdef _MSC_VER
        if (::InterlockedDecrement64(&ref_) == 0)
        {
            delete this;
        }
#else
        int64_t v = __sync_fetch_and_add(&ref_, -1);
        if (v == 0) {
            delete this;
        }
#endif
    }
};

class parse_response_head
{
    //  message ResponseHeader{
//    // cluster_id is the ID of the cluster which sent the response.
//    uint64 cluster_id = 1;
//    // member_id is the ID of the member which sent the response.
//    uint64 member_id = 2;
//    // revision is the key-value store revision when the request was applied.
//    int64 revision = 3;
//    // raft_term is the raft term when the request was applied.
//    uint64 raft_term = 4;
//  }
public:
    uint64_t cluster_id = 0;
    uint64_t member_id = 0;
    int64_t revision = 0;
    uint64_t raft_term = 0;
    int parse(char* buf, int len)
    {
        int offs = 0;
        uint64_t val = 0;
        char* data = buf;

        while (offs < len)
        {
            // first tag
            int tag_size = pb_read_int(&data[offs], len, val);
            int fid = (int)val;
            int ftype = fid & 0x7;
            fid = fid >> 3;
            offs += tag_size;

            switch (fid)
            {
            case 1: // cluster_id
                tag_size = pb_read_int(&data[offs], len - offs, val);
                cluster_id = val;
                break;
            case 2: // memer_id
                tag_size = pb_read_int(&data[offs], len - offs, val);
                member_id = val;
                break;
            case 3: // revision
                tag_size = pb_read_int(&data[offs], len - offs, val);
                revision = val;
                break;
            case 4: // term
                tag_size = pb_read_int(&data[offs], len - offs, val);
                raft_term = val;
                break;
            default:
                if (ftype == 0)
                {
                    tag_size = pb_read_int(&data[offs], len - offs, val);
                }
                else if (ftype == 2) // bytes
                {
                    int v = pb_read_int(&data[offs], len - offs, val);
                    tag_size += v + (int)val;
                    // ignore
                }
                else
                {
                    //TODO: support other type
                    return len;
                }
                break;
            }
            offs += tag_size;
        }
        return len;
    }
};

class parse_keyvalue
{
public:
/*
message KeyValue {
  // key is the key in bytes. An empty key is not allowed.
  bytes key = 1;
  // create_revision is the revision of last creation on this key.
  int64 create_revision = 2;
  // mod_revision is the revision of last modification on this key.
  int64 mod_revision = 3;
  // version is the version of the key. A deletion resets
  // the version to zero and any modification of the key
  // increases its version.
  int64 version = 4;
  // value is the value held by the key, in bytes.
  bytes value = 5;
  // lease is the ID of the lease that attached to key.
  // When the attached lease expires, the key will be deleted.
  // If lease is 0, then no lease is attached to the key.
  int64 lease = 6;
}
*/
    vetcd_keyvalue kv;
public:
    int parse(char* buf, int len)
    {
        int offs = 0;
        uint64_t val = 0;
        char* data = buf;

        while (offs < len)
        {
            // first tag
            int tag_size = pb_read_int(&data[offs], len, val);
            int fid = (int)val;
            int ftype = fid & 0x7;
            fid = fid >> 3;
            offs += tag_size;

            switch (fid)
            {
            case 1: // key
                tag_size = pb_read_int(&data[offs], len - offs, val);
                offs += tag_size;
                tag_size = (int)val;
                kv.skey.assign(&data[offs], val);
                break;
            case 2: // create_revision
                tag_size = pb_read_int(&data[offs], len - offs, val);
                kv.ncreate_rev = val;
                break;
            case 3: // mod_revision
                tag_size = pb_read_int(&data[offs], len - offs, val);
                kv.nmod_rev = val;
                break;
            case 4: // version
                tag_size = pb_read_int(&data[offs], len - offs, val);
                kv.nver = val;
                break;
            case 5: // value
                tag_size = pb_read_int(&data[offs], len - offs, val);
                offs += tag_size;
                tag_size = (int)val;
                kv.sval.assign(&data[offs], val);
                break;
            case 6: // lease
                tag_size = pb_read_int(&data[offs], len - offs, val);
                kv.nleaseid = val;
                break;
            default:
                if (ftype == 0)
                {
                    tag_size = pb_read_int(&data[offs], len - offs, val);
                }
                else if (ftype == 2) // bytes
                {
                    int v = pb_read_int(&data[offs], len - offs, val);
                    tag_size += v + (int)val;
                    // ignore
                }
                else
                {
                    //TODO: support other type
                    return len;
                }
                break;
            }
            offs += tag_size;
        }
        return len;
    }
};

class parse_member
{
public:
    /*
message Member {
  // ID is the member ID for this member.
  uint64 ID = 1;
  // name is the human-readable name of the member. If the member is not started, the name will be an empty string.
  string name = 2;
  // peerURLs is the list of URLs the member exposes to the cluster for communication.
  repeated string peerURLs = 3;
  // clientURLs is the list of URLs the member exposes to clients for communication. If the member is not started, clientURLs will be empty.
  repeated string clientURLs = 4;
  // isLearner indicates if the member is raft learner.
  bool isLearner = 5;
}
    */
    vetcd_member kv;
public:
    int parse(char* buf, int len)
    {
        int offs = 0;
        uint64_t val = 0;
        char* data = buf;

        while (offs < len)
        {
            // first tag
            int tag_size = pb_read_int(&data[offs], len, val);
            int fid = (int)val;
            int ftype = fid & 0x7;
            fid = fid >> 3;
            offs += tag_size;

            switch (fid)
            {
            case 1: // ID
                tag_size = pb_read_int(&data[offs], len - offs, val);
                kv.nid = val;
                break;
            case 5: // isLearner
                tag_size = pb_read_int(&data[offs], len - offs, val);
                kv.islearner = (val != 0);
                break;
            case 2: // name
            case 3: // peerURLs
            case 4: // clientURLs
            {
                tag_size = pb_read_int(&data[offs], len - offs, val);
                offs += tag_size;
                std::string s1;
                s1.assign(&data[offs], val);
                tag_size = (int)val;
                if (fid == 2) kv.sname = s1;
                else if (fid == 3) { if (!kv.speerurl.empty()) kv.speerurl += ","; kv.speerurl += s1; }
                else if (fid == 4) { if (!kv.scliurl.empty()) kv.scliurl += ","; kv.scliurl += s1; }
                break;
            }
            default:
                if (ftype == 0)
                {
                    tag_size = pb_read_int(&data[offs], len - offs, val);
                }
                else if (ftype == 2) // bytes
                {
                    int v = pb_read_int(&data[offs], len - offs, val);
                    tag_size += v + (int)val;
                    // ignore
                }
                else
                {
                    //TODO: support other type
                    return len;
                }
                break;
            }
            offs += tag_size;
        }
        return len;
    }
};

class parse_event
{
public:
    int ntype = 0;
    vetcd_keyvalue kv;
    vetcd_keyvalue pre;
public:
    int parse(char* buf, int len)
    {
        int offs = 0;
        uint64_t val = 0;
        char* data = buf;

        while (offs < len)
        {
            // first tag
            int tag_size = pb_read_int(&data[offs], len, val);
            int fid = (int)val;
            //int ftype = fid & 0x7;
            fid = fid >> 3;
            offs += tag_size;

            switch (fid)
            {
            case 1: // key
                tag_size = pb_read_int(&data[offs], len - offs, val);
                ntype = (int)val;
                offs += tag_size;
                break;
            case 2: // kv
            case 3:
            {
                tag_size = pb_read_int(&data[offs], len - offs, val);
                offs += tag_size;
                parse_keyvalue pk;
                tag_size = pk.parse(&data[offs], (int)val);
                if (fid == 2)
                    kv = pk.kv;
                else
                    pre = pk.kv;
                offs += tag_size;
                break;
            }
            default:
                return len;
                break;
            }
        }
        return len;
    }
};

class lease_ans_stream: public http2_stream
{
public:
    int64_t lease_id_ = -1;
    int ttl_ = 0;
    parse_response_head response;
public:
    lease_ans_stream() {}
    virtual ~lease_ans_stream() {}

    virtual int parse(char* buf, int len, HPACK::decoder_t& headdec)
    {
        pkt_head* head = (pkt_head*)buf;
        if (head->ntype == STEAM_TYPE_HEADER)
            return parse_header(buf, len, headdec);
        if (head->ntype == STEAM_TYPE_DATA)
            return parse_data(buf, len);
        if (head->ntype == STEAM_TYPE_RST)
            return parse_rst(buf, len);
        return 0;
    }
    int parse_rst(char* buf, int len)
    {
        if (len < GRPC_HEAD_SIZE)
            return 0;
        pkt_head* head = (pkt_head*)buf;
        if (head->ntype != STEAM_TYPE_RST)
            return 0;
        int len2 = head->nlen.getint();
        if (len2 == 0)
            return GRPC_HEAD_SIZE;
        // parse grpc
        char* data = buf + GRPC_HEAD_SIZE;
        error_code = -abs(vetcd_ReverseBytes32(*(int*)data));
        if (error_code == -1)
            error_message = "grpc protocol error";
        notify_response();
        return 0;
    }
    int parse_header(char* buf, int len, HPACK::decoder_t& headdec)
    {
        (void)len;
        pkt_head* head = (pkt_head*)buf;
        if (head->ntype != STEAM_TYPE_HEADER)
            return 0;
        bool rs = headdec.decode2(head->data, head->nlen.getint());
        if (!(head->nflags & 0x1)) // end stream
        {
            return 0;
        }
        if (true == rs)
        {
            auto it = headdec.headers().find("grpc-status");
            if (it != headdec.headers().end())
                error_code = -abs(atoi(it->second.c_str()));
            it = headdec.headers().find("grpc-message");
            if (it != headdec.headers().end())
                error_message = it->second;
        }
        notify_response();
        return 0;
    }
    int parse_data(char* buf, int len)
    {
        if (len < GRPC_HEAD_SIZE)
            return 0;
        pkt_head* head = (pkt_head*)buf;
        if (head->ntype != STEAM_TYPE_DATA)
            return 0;
        int len2 = head->nlen.getint();
        if (len2 == 0)
            return GRPC_HEAD_SIZE;
        // parse grpc
        char* data = buf + GRPC_HEAD_SIZE;
        if (data != 0)
        {
            //TODO: de-compress
        }
        data++;
        int pblen = vetcd_ReverseBytes32(*(int*)data);
        data += 4;
        // parse pb
        int offs = 0;
        uint64_t val = 0;
        while (offs < pblen)
        {
            // t - l -v
            val = 0;
            int tag_size = pb_read_int(&data[offs], pblen - offs, val);
            int fid = (int)val;
            int ftype = fid & 0x7;
            fid = fid >> 3;
            offs += tag_size;
            if (fid == 1) // ResponseHeader
            {
                // length
                tag_size = pb_read_int(&data[offs], pblen - offs, val);
                offs += tag_size;
                // content
                tag_size = response.parse(&data[offs], (int)val);
                offs += tag_size;
            }
            else if (fid == 2) // lease id
            {
                tag_size = pb_read_int(&data[offs], pblen - offs, val);
                lease_id_ = val;
                offs += tag_size;
            }
            else if (fid == 3) // lease ttl
            {
                tag_size = pb_read_int(&data[offs], pblen - offs, val);
                ttl_ = (int)val;
                offs += tag_size;
            }
            else
            {
                // skip field
                if (ftype == 0)
                {
                    tag_size = pb_read_int(&data[offs], len - offs, val);
                }
                else if (ftype == 2) // bytes
                {
                    int v = pb_read_int(&data[offs], len - offs, val);
                    tag_size += v + (int)val;
                }
                else
                {
                    //TODO: support 
                    break;
                }
                offs += tag_size;
            }
        }
        return len2 + GRPC_HEAD_SIZE;
    }
};

class leasettl_ans_stream : public http2_stream
{
public:
    int64_t lease_id_ = -1;
    int ttl_ = 0;
    int cur_ttl_ = 0;
    parse_response_head response;
public:
    leasettl_ans_stream() {}
    virtual ~leasettl_ans_stream() {}

    virtual int parse(char* buf, int len, HPACK::decoder_t& headdec)
    {
        pkt_head* head = (pkt_head*)buf;
        if (head->ntype == STEAM_TYPE_HEADER)
            return parse_header(buf, len, headdec);
        if (head->ntype == STEAM_TYPE_DATA)
            return parse_data(buf, len);
        if (head->ntype == STEAM_TYPE_RST)
            return parse_rst(buf, len);
        return 0;
    }
    int parse_rst(char* buf, int len)
    {
        if (len < GRPC_HEAD_SIZE)
            return 0;
        pkt_head* head = (pkt_head*)buf;
        if (head->ntype != STEAM_TYPE_RST)
            return 0;
        int len2 = head->nlen.getint();
        if (len2 == 0)
            return GRPC_HEAD_SIZE;
        // parse grpc
        char* data = buf + GRPC_HEAD_SIZE;
        error_code = -abs(vetcd_ReverseBytes32(*(int*)data));
        if (error_code == -1)
            error_message = "grpc protocol error";
        notify_response();
        return 0;
    }
    int parse_header(char* buf, int len, HPACK::decoder_t& headdec)
    {
        (void)len;
        pkt_head* head = (pkt_head*)buf;
        if (head->ntype != STEAM_TYPE_HEADER)
            return 0;
        bool rs = headdec.decode2(head->data, head->nlen.getint());
        if (!(head->nflags & 0x1)) // end stream
        {
            return 0;
        }
        if (true == rs)
        {
            auto it = headdec.headers().find("grpc-status");
            if (it != headdec.headers().end())
                error_code = -abs(atoi(it->second.c_str()));
            it = headdec.headers().find("grpc-message");
            if (it != headdec.headers().end())
                error_message = it->second;
        }
        notify_response();
        return 0;
    }
    int parse_data(char* buf, int len)
    {
        if (len < GRPC_HEAD_SIZE)
            return 0;
        pkt_head* head = (pkt_head*)buf;
        if (head->ntype != STEAM_TYPE_DATA)
            return 0;
        int len2 = head->nlen.getint();
        if (len2 == 0)
            return GRPC_HEAD_SIZE;
        // parse grpc
        char* data = buf + GRPC_HEAD_SIZE;
        if (data != 0)
        {
            //TODO: de-compress
        }
        data++;
        int pblen = vetcd_ReverseBytes32(*(int*)data);
        data += 4;
        // parse pb
        int offs = 0;
        uint64_t val = 0;
        while (offs < pblen)
        {
            // t - l -v
            val = 0;
            int tag_size = pb_read_int(&data[offs], pblen - offs, val);
            int fid = (int)val;
            int ftype = fid & 0x7;
            fid = fid >> 3;
            offs += tag_size;
            if (fid == 1) // ResponseHeader
            {
                // length
                tag_size = pb_read_int(&data[offs], pblen - offs, val);
                offs += tag_size;
                // content
                tag_size = response.parse(&data[offs], (int)val);
                offs += tag_size;
            }
            else if (fid == 2) // lease id
            {
                tag_size = pb_read_int(&data[offs], pblen - offs, val);
                lease_id_ = val;
                offs += tag_size;
            }
            else if (fid == 3) // lease ttl
            {
                tag_size = pb_read_int(&data[offs], pblen - offs, val);
                cur_ttl_ = (int)val;
                offs += tag_size;
            }
            else if (fid == 4) // lease ttl
            {
                tag_size = pb_read_int(&data[offs], pblen - offs, val);
                ttl_ = (int)val;
                offs += tag_size;
            }
            else
            {
                // skip field
                if (ftype == 0)
                {
                    tag_size = pb_read_int(&data[offs], len - offs, val);
                }
                else if (ftype == 2) // bytes
                {
                    int v = pb_read_int(&data[offs], len - offs, val);
                    tag_size += v + (int)val;
                }
                else
                {
                    //TODO: support 
                    break;
                }
                offs += tag_size;
            }
        }
        return len2 + GRPC_HEAD_SIZE;
    }
};

class put_ans_stream: public http2_stream
{
  //  message PutResponse{
  //    ResponseHeader header = 1;
  //    // if prev_kv is set in the request, the previous key-value pair will be returned.
  //    mvccpb.KeyValue prev_kv = 2;
  //  }
public:
    parse_response_head response;

public:
    put_ans_stream() {}
    virtual ~put_ans_stream() {}

    virtual int parse(char* buf, int len, HPACK::decoder_t& headdec)
    {
        pkt_head* head = (pkt_head*)buf;
        if (head->ntype == STEAM_TYPE_HEADER)
            return parse_header(buf, len, headdec);
        if (head->ntype == STEAM_TYPE_DATA)
            return parse_data(buf, len);
        if (head->ntype == STEAM_TYPE_RST)
            return parse_rst(buf, len);
        return 0;
    }
    int parse_rst(char* buf, int len)
    {
        if (len < GRPC_HEAD_SIZE)
            return 0;
        pkt_head* head = (pkt_head*)buf;
        if (head->ntype != STEAM_TYPE_RST)
            return 0;
        int len2 = head->nlen.getint();
        if (len2 == 0)
            return GRPC_HEAD_SIZE;
        // parse grpc
        char* data = buf + GRPC_HEAD_SIZE;
        error_code = -abs(vetcd_ReverseBytes32(*(int*)data));
        if (error_code == -1)
            error_message = "grpc protocol error";
        notify_response();
        return 0;
    }
    int parse_header(char* buf, int len, HPACK::decoder_t& headdec)
    {
        (void)len;
        pkt_head* head = (pkt_head*)buf;
        if (head->ntype != STEAM_TYPE_HEADER)
            return 0;
        bool rs = headdec.decode2(head->data, head->nlen.getint());
        if (!(head->nflags & 0x1)) // end stream
        {
            return 0;
        }
        if (true == rs)
        {
            auto it = headdec.headers().find("grpc-status");
            if (it != headdec.headers().end())
                error_code = -abs(atoi(it->second.c_str()));
            it = headdec.headers().find("grpc-message");
            if (it != headdec.headers().end())
                error_message = it->second;
        }
        notify_response();
        return 0;
    }
    int parse_data(char* buf, int len)
    {
        if (len < GRPC_HEAD_SIZE)
            return 0;
        pkt_head* head = (pkt_head*)buf;
        if (head->ntype != STEAM_TYPE_DATA)
            return 0;
        int len2 = head->nlen.getint();
        if (len2 == 0)
            return GRPC_HEAD_SIZE;
        // parse grpc
        char* data = buf + GRPC_HEAD_SIZE;
        if (data != 0)
        {
            //TODO: de-compress
        }
        data++;
        int pblen = vetcd_ReverseBytes32(*(int*)data);
        data += 4;
        // parse pb
        int offs = 0;
        uint64_t val = 0;
        while (offs < pblen)
        {
            // t - l -v
            val = 0;
            int tag_size = pb_read_int(&data[offs], pblen - offs, val);
            int fid = (int)val;
            //int ftype = fid & 0x7;
            fid = fid >> 3;
            offs += tag_size;
            if (fid == 1) // ResponseHeader
            {
                // length
                tag_size = pb_read_int(&data[offs], pblen - offs, val);
                offs += tag_size;
                // content
                tag_size = response.parse(&data[offs], (int)val);
                offs += tag_size;
            }
            else
            {
                break;
            }
        }
        return len2 + GRPC_HEAD_SIZE;
    }
};

class get_ans_stream : public http2_stream
{
    //  message PutResponse{
    //    ResponseHeader header = 1;
    //    // if prev_kv is set in the request, the previous key-value pair will be returned.
    //    mvccpb.KeyValue prev_kv = 2;
    //  }
public:
    bool has_pad = false;
    bool has_more = false;
    int row_count = 0;
    parse_response_head response;
    std::vector<vetcd_keyvalue> arritem;

public:
    get_ans_stream() {}
    virtual ~get_ans_stream() {}
    std::string get_first_value()
    {
        if (arritem.empty())
            return std::string();
        return arritem[0].sval;
    }
    virtual int parse(char* buf, int len, HPACK::decoder_t& headdec)
    {
        pkt_head* head = (pkt_head*)buf;
        if (head->ntype == STEAM_TYPE_HEADER)
        {
            if (head->nflags & 0x8)
            {
                has_pad = true;
            }
            if ((head->nflags & 0x1) && sdata.length() > 0)
                parse_data((char*)sdata.c_str(), sdata.length());
            return parse_header(buf, len, headdec);
        }
            
        if (head->ntype == STEAM_TYPE_DATA)
        {
            char* data = buf + sizeof(pkt_head);
            int len2 = len - sizeof(pkt_head);
            if (has_pad)
            {
                int npad = *(uint8_t *)data;
                sdata.append(data + 1, len2 - 1 - (npad));
            }
            else sdata.append(data, len2);
            return len;
            //return parse_data(buf, len);
        }
        if (head->ntype == STEAM_TYPE_RST)
            return parse_rst(buf, len);
        return 0;
    }
    int parse_rst(char* buf, int len)
    {
        if (len < GRPC_HEAD_SIZE)
            return 0;
        pkt_head* head = (pkt_head*)buf;
        if (head->ntype != STEAM_TYPE_RST)
            return 0;
        int len2 = head->nlen.getint();
        if (len2 == 0)
            return GRPC_HEAD_SIZE;
        // parse grpc
        char* data = buf + GRPC_HEAD_SIZE;
        error_code = -abs(vetcd_ReverseBytes32(*(int*)data));
        if (error_code == -1)
            error_message = "grpc protocol error";
        notify_response();
        return 0;
    }
    int parse_header(char* buf, int len, HPACK::decoder_t& headdec)
    {
        (void)len;
        pkt_head* head = (pkt_head*)buf;
        if (head->ntype != STEAM_TYPE_HEADER)
            return 0;
        bool rs = headdec.decode2(head->data, head->nlen.getint());
        if (!(head->nflags & 0x1)) // end stream
        {
            return 0;
        }
        if (true == rs)
        {
            auto it = headdec.headers().find("grpc-status");
            if (it != headdec.headers().end())
                error_code = -abs(atoi(it->second.c_str()));
            it = headdec.headers().find("grpc-message");
            if (it != headdec.headers().end())
                error_message = it->second;
        }
        if (arritem.empty() && error_code == 0)
        {
            error_code = -1;
            error_message = "key not exists";
        }
        notify_response();
        return 0;
    }
    int parse_data(char* buf, int len)
    {
        //if (len < GRPC_HEAD_SIZE)
        //    return 0;
        //pkt_head* head = (pkt_head*)buf;
        //if (head->ntype != STEAM_TYPE_DATA)
        //    return 0;
        //int len2 = head->nlen.getint();
        //if (len2 == 0)
        //    return GRPC_HEAD_SIZE;
        ////TODO: may be not enough data
        //char* data = buf + GRPC_HEAD_SIZE;
        //if (data != 0)
        //{
        //    //TODO: de-compress
        //}
        //data++;
        char* data = buf;
        data++;
        int len2 = len;
        int pblen = vetcd_ReverseBytes32(*(int*)data);
        data += 4;
        // parse pb
        int offs = 0;
        uint64_t val = 0;
        while (offs < pblen)
        {
            // t - l -v
            val = 0;
            int tag_size = pb_read_int(&data[offs], pblen - offs, val);
            int fid = (int)val;
            //int ftype = fid & 0x7;
            fid = fid >> 3;
            offs += tag_size;
            if (fid == 1) // ResponseHeader
            {
                // length
                tag_size = pb_read_int(&data[offs], pblen - offs, val);
                offs += tag_size;
                // content
                tag_size = response.parse(&data[offs], (int)val);
                offs += tag_size;
            }
            else if (fid == 2) // repeat key-value
            {
                parse_keyvalue kv;
                // length
                tag_size = pb_read_int(&data[offs], pblen - offs, val);
                offs += tag_size;
                // content
                tag_size = kv.parse(&data[offs], (int)val);
                offs += tag_size;
                if (kv.kv.skey.empty() == false)
                {
                    arritem.push_back(kv.kv);
                }
            }
            else if (fid == 3) //more
            {
                tag_size = pb_read_int(&data[offs], pblen - offs, val);
                has_more = val ? true : false;
                offs += tag_size;
            }
            else if (fid == 4) // count
            {
                tag_size = pb_read_int(&data[offs], pblen - offs, val);
                row_count = (int)val;
                offs += tag_size;
            }
            else
            {
                //TODO: implement skip
                break;
            }
        }
        return len2 + GRPC_HEAD_SIZE;
    }
};

class member_ans_stream : public http2_stream
{
public:
    parse_response_head response;
    std::vector<vetcd_member> arritem;

public:
    member_ans_stream() {}
    virtual ~member_ans_stream() {}
    virtual int parse(char* buf, int len, HPACK::decoder_t& headdec)
    {
        pkt_head* head = (pkt_head*)buf;
        if (head->ntype == STEAM_TYPE_HEADER)
            return parse_header(buf, len, headdec);
        if (head->ntype == STEAM_TYPE_DATA)
            return parse_data(buf, len);
        if (head->ntype == STEAM_TYPE_RST)
            return parse_rst(buf, len);
        return 0;
    }
    int parse_rst(char* buf, int len)
    {
        if (len < GRPC_HEAD_SIZE)
            return 0;
        pkt_head* head = (pkt_head*)buf;
        if (head->ntype != STEAM_TYPE_RST)
            return 0;
        int len2 = head->nlen.getint();
        if (len2 == 0)
            return GRPC_HEAD_SIZE;
        // parse grpc
        char* data = buf + GRPC_HEAD_SIZE;
        error_code = -abs(vetcd_ReverseBytes32(*(int*)data));
        if (error_code == -1)
            error_message = "grpc protocol error";
        notify_response();
        return 0;
    }
    int parse_header(char* buf, int len, HPACK::decoder_t& headdec)
    {
        (void)len;
        pkt_head* head = (pkt_head*)buf;
        if (head->ntype != STEAM_TYPE_HEADER)
            return 0;
        bool rs = headdec.decode2(head->data, head->nlen.getint());
        if (!(head->nflags & 0x1)) // end stream
        {
            return 0;
        }
        if (true == rs)
        {
            auto it = headdec.headers().find("grpc-status");
            if (it != headdec.headers().end())
                error_code = -abs(atoi(it->second.c_str()));
            it = headdec.headers().find("grpc-message");
            if (it != headdec.headers().end())
                error_message = it->second;
        }
        if (arritem.empty() && error_code == 0)
        {
            error_code = -1;
            error_message = "no member return";
        }
        notify_response();
        return 0;
    }
    int parse_data(char* buf, int len)
    {
        if (len < GRPC_HEAD_SIZE)
            return 0;
        pkt_head* head = (pkt_head*)buf;
        if (head->ntype != STEAM_TYPE_DATA)
            return 0;
        int len2 = head->nlen.getint();
        if (len2 == 0)
            return GRPC_HEAD_SIZE;
        // parse grpc
        char* data = buf + GRPC_HEAD_SIZE;
        if (data != 0)
        {
            //TODO: de-compress
        }
        data++;
        int pblen = vetcd_ReverseBytes32(*(int*)data);
        data += 4;
        // parse pb
        int offs = 0;
        uint64_t val = 0;
        while (offs < pblen)
        {
            // t - l -v
            val = 0;
            int tag_size = pb_read_int(&data[offs], pblen - offs, val);
            int fid = (int)val;
            //int ftype = fid & 0x7;
            fid = fid >> 3;
            offs += tag_size;
            if (fid == 1) // ResponseHeader
            {
                // length
                tag_size = pb_read_int(&data[offs], pblen - offs, val);
                offs += tag_size;
                // content
                tag_size = response.parse(&data[offs], (int)val);
                offs += tag_size;
            }
            else if (fid == 2) // repeat member
            {
                parse_member pm;
                // length
                tag_size = pb_read_int(&data[offs], pblen - offs, val);
                offs += tag_size;
                // content
                tag_size = pm.parse(&data[offs], (int)val);
                offs += tag_size;
                if (pm.kv.nid != 0)
                {
                    arritem.push_back(pm.kv);
                }
            }
            else
            {
                //TODO: implement skip
                break;
            }
        }
        return len2 + GRPC_HEAD_SIZE;
    }
};

class delete_ans_stream : public http2_stream
{
public:
    int row_count = 0;
    parse_response_head response;
    std::vector<vetcd_keyvalue> arritem;

public:
    delete_ans_stream() {}
    virtual ~delete_ans_stream() {}

    virtual int parse(char* buf, int len, HPACK::decoder_t& headdec)
    {
        pkt_head* head = (pkt_head*)buf;
        if (head->ntype == STEAM_TYPE_HEADER)
            return parse_header(buf, len, headdec);
        if (head->ntype == STEAM_TYPE_DATA)
            return parse_data(buf, len);
        if (head->ntype == STEAM_TYPE_RST)
            return parse_rst(buf, len);
        return 0;
    }
    int parse_rst(char* buf, int len)
    {
        if (len < GRPC_HEAD_SIZE)
            return 0;
        pkt_head* head = (pkt_head*)buf;
        if (head->ntype != STEAM_TYPE_RST)
            return 0;
        int len2 = head->nlen.getint();
        if (len2 == 0)
            return GRPC_HEAD_SIZE;
        // parse grpc
        char* data = buf + GRPC_HEAD_SIZE;
        error_code = -abs(vetcd_ReverseBytes32(*(int*)data));
        if (error_code == -1)
            error_message = "grpc protocol error";
        notify_response();
        return 0;
    }
    int parse_header(char* buf, int len, HPACK::decoder_t& headdec)
    {
        (void)len;
        pkt_head* head = (pkt_head*)buf;
        if (head->ntype != STEAM_TYPE_HEADER)
            return 0;
        bool rs = headdec.decode2(head->data, head->nlen.getint());
        if (!(head->nflags & 0x1)) // end stream
        {
            return 0;
        }
        if (true == rs)
        {
            auto it = headdec.headers().find("grpc-status");
            if (it != headdec.headers().end())
                error_code = -abs(atoi(it->second.c_str()));
            it = headdec.headers().find("grpc-message");
            if (it != headdec.headers().end())
                error_message = it->second;
        }
        notify_response();
        return 0;
    }
    int parse_data(char* buf, int len)
    {
        if (len < GRPC_HEAD_SIZE)
            return 0;
        pkt_head* head = (pkt_head*)buf;
        if (head->ntype != STEAM_TYPE_DATA)
            return 0;
        int len2 = head->nlen.getint();
        if (len2 == 0)
            return GRPC_HEAD_SIZE;
        // parse grpc
        char* data = buf + GRPC_HEAD_SIZE;
        if (data != 0)
        {
            //TODO: de-compress
        }
        data++;
        int pblen = vetcd_ReverseBytes32(*(int*)data);
        data += 4;
        // parse pb
        int offs = 0;
        uint64_t val = 0;
        while (offs < pblen)
        {
            // t - l -v
            val = 0;
            int tag_size = pb_read_int(&data[offs], pblen - offs, val);
            int fid = (int)val;
            //int ftype = fid & 0x7;
            fid = fid >> 3;
            offs += tag_size;
            if (fid == 1) // ResponseHeader
            {
                // length
                tag_size = pb_read_int(&data[offs], pblen - offs, val);
                offs += tag_size;
                // content
                tag_size = response.parse(&data[offs], (int)val);
                offs += tag_size;
            }
            else if (fid == 3) // repeat key-value
            {
                parse_keyvalue kv;
                // length
                tag_size = pb_read_int(&data[offs], pblen - offs, val);
                offs += tag_size;
                // content
                tag_size = kv.parse(&data[offs], (int)val);
                offs += tag_size;
                if (kv.kv.skey.empty() == false)
                {
                    arritem.push_back(kv.kv);
                }
            }
            else if (fid == 2) //more
            {
                tag_size = pb_read_int(&data[offs], pblen - offs, val);
                row_count = (int)val;
                offs += tag_size;
            }
            else
            {
                //TODO: implement skip
                break;
            }
        }
        return len2 + GRPC_HEAD_SIZE;
    }
};

class watch_ans_stream : public http2_stream
{
public:
    int64_t watch_id_ = -1;
    parse_response_head response;
    cb_vetcd_watch watch_cb = NULL;
    void* cb_data = NULL;
    std::string sdata;
public:
    watch_ans_stream() {}
    virtual ~watch_ans_stream() {}

    virtual int parse(char* buf, int len, HPACK::decoder_t& headdec)
    {
        pkt_head* head = (pkt_head*)buf;
        if (head->ntype == STEAM_TYPE_HEADER)
            return parse_header(buf, len, headdec);
        if (head->ntype == STEAM_TYPE_DATA)
            return parse_data(buf, len);
        if (head->ntype == STEAM_TYPE_RST)
            return parse_rst(buf, len);
        return 0;
    }
    int parse_header(char* buf, int len, HPACK::decoder_t& headdec)
    {
        (void)len;
        pkt_head* head = (pkt_head*)buf;
        if (head->ntype != STEAM_TYPE_HEADER)
            return 0;
        bool rs = headdec.decode2(head->data, head->nlen.getint());
        if (!(head->nflags & 0x1)) // end stream
        {
            return 0;
        }

        if (true == rs)
        {
            auto it = headdec.headers().find("grpc-status");
            if (it != headdec.headers().end())
                error_code = -abs(atoi(it->second.c_str()));
            it = headdec.headers().find("grpc-message");
            if (it != headdec.headers().end())
                error_message = it->second;
        }
        notify_response();
        return 0;
    }
    int parse_rst(char* buf, int len)
    {
        if (len < GRPC_HEAD_SIZE)
            return 0;
        pkt_head* head = (pkt_head*)buf;
        if (head->ntype != STEAM_TYPE_RST)
            return 0;
        int len2 = head->nlen.getint();
        if (len2 == 0)
            return GRPC_HEAD_SIZE;
        // parse grpc
        char* data = buf + GRPC_HEAD_SIZE;
        error_code = -abs(vetcd_ReverseBytes32(*(int*)data));
        if (error_code == -1)
            error_message = "grpc protocol error";
        notify_response();
        return 0;
    }
    int parse_data(char* buf, int len)
    {
        if (len < GRPC_HEAD_SIZE)
            return 0;
        pkt_head* head = (pkt_head*)buf;
        if (head->ntype != STEAM_TYPE_DATA)
            return 0;
        int len2 = head->nlen.getint();
        if (len2 == 0)
            return GRPC_HEAD_SIZE;
        sdata.append(buf + 9, len2);
        // parse grpc
        char* data = (char*)sdata.c_str();
        if (data != 0)
        {
            //TODO: de-compress
        }
        data++;
        int pblen = vetcd_ReverseBytes32(*(int*)data);
        data += 4;
        if (pblen == sdata.length() - 5)
        {
            parse_data2(data, pblen);
            sdata.clear();
        }
        return len2 + 9;
    }
    int parse_data2(char* data, int len)
    {
        int pblen = len;
        // parse pb
        int offs = 0;
        uint64_t val = 0;
        error_code = -1; // watch stream no end header if not cancel
        error_message = "watch not create on server";
        watch_id_ = -1; //TODO: maybe server not return watch id
        while (offs < pblen)
        {
            // t - l -v
            val = 0;
            int tag_size = pb_read_int(&data[offs], pblen - offs, val);
            int fid = (int)val;
            int ftype = fid & 0x7;
            fid = fid >> 3;
            offs += tag_size;
            if (fid == 1) // ResponseHeader
            {
                // length
                tag_size = pb_read_int(&data[offs], pblen - offs, val);
                offs += tag_size;
                // content
                tag_size = response.parse(&data[offs], (int)val);
                offs += tag_size;
            }
            else if (fid == 2) // watch id
            {
                tag_size = pb_read_int(&data[offs], pblen - offs, val);
                watch_id_ = val;
                offs += tag_size;
            }
            else if (fid == 3) // bcreate = true;
            {
                tag_size = pb_read_int(&data[offs], pblen - offs, val);
                if (val == 1)
                {
                    if (watch_id_ == -1)  watch_id_ = 0;
                    error_code = 0;
                    error_message.clear();
                }
                offs += tag_size;
            }
            else if (fid == 11) // event
            {
                tag_size = pb_read_int(&data[offs], pblen - offs, val);
                offs += tag_size;
                parse_event ev;
                tag_size = ev.parse(&data[offs], (int)val);
                //printf("--event:%d, key:%s,val:%s, pre key:%s,val:%s\n", ev.ntype, ev.kv.skey.c_str(), ev.kv.sval.c_str(),
                //    ev.pre.skey.c_str(), ev.pre.sval.c_str() );
                if (watch_cb != NULL)
                    (watch_cb)(ev.ntype == 0 ? "PUT" : "DELETE", ev.kv.skey, ev.kv.sval, ev.kv.nver == 1, ev.kv.nmod_rev, cb_data);
                offs += tag_size;
            }
            else
            {
                // skip field
                if (ftype == 0)
                {
                    tag_size = pb_read_int(&data[offs], len - offs, val);
                }
                else if (ftype == 2) // bytes
                {
                    int v = pb_read_int(&data[offs], len - offs, val);
                    tag_size += v + (int)val;
                }
                else
                {
                    //TODO: support 
                    break;
                }
                offs += tag_size;
            }
        }
        notify_response();
        return len;
    }
};

class auth_ans_stream : public http2_stream
{
/*
message AuthenticateResponse {
  ResponseHeader header = 1;
  // token is an authorized token that can be used in succeeding RPCs
  string token = 2;
}
*/
public:
    parse_response_head response;
    std::string stoken;

public:
    auth_ans_stream() {}
    virtual ~auth_ans_stream() {}

    virtual int parse(char* buf, int len, HPACK::decoder_t& headdec)
    {
        pkt_head* head = (pkt_head*)buf;
        if (head->ntype == STEAM_TYPE_HEADER)
            return parse_header(buf, len, headdec);
        if (head->ntype == STEAM_TYPE_DATA)
            return parse_data(buf, len);
        if (head->ntype == STEAM_TYPE_RST)
            return parse_rst(buf, len);
        return 0;
    }
    int parse_rst(char* buf, int len)
    {
        if (len < GRPC_HEAD_SIZE)
            return 0;
        pkt_head* head = (pkt_head*)buf;
        if (head->ntype != STEAM_TYPE_RST)
            return 0;
        int len2 = head->nlen.getint();
        if (len2 == 0)
            return GRPC_HEAD_SIZE;
        // parse grpc
        char* data = buf + GRPC_HEAD_SIZE;
        error_code = -abs(vetcd_ReverseBytes32(*(int*)data));
        if (error_code == -1)
            error_message = "grpc protocol error";
        notify_response();
        return 0;
    }
    int parse_header(char* buf, int len, HPACK::decoder_t& headdec)
    {
        (void)len;
        pkt_head* head = (pkt_head*)buf;
        if (head->ntype != STEAM_TYPE_HEADER)
            return 0;
        bool rs = headdec.decode2(head->data, head->nlen.getint());
        if (!(head->nflags & 0x1)) // end stream
        {
            return 0;
        }
        if (true == rs)
        {
            auto it = headdec.headers().find("grpc-status");
            if (it != headdec.headers().end())
                error_code = -abs(atoi(it->second.c_str()));
            it = headdec.headers().find("grpc-message");
            if (it != headdec.headers().end())
                error_message = it->second;
        }
        notify_response();
        return 0;
    }
    int parse_data(char* buf, int len)
    {
        if (len < GRPC_HEAD_SIZE)
            return 0;
        pkt_head* head = (pkt_head*)buf;
        if (head->ntype != STEAM_TYPE_DATA)
            return 0;
        int len2 = head->nlen.getint();
        if (len2 == 0)
            return GRPC_HEAD_SIZE;
        // parse grpc
        char* data = buf + GRPC_HEAD_SIZE;
        if (data != 0)
        {
            //TODO: de-compress
        }
        data++;
        int pblen = vetcd_ReverseBytes32(*(int*)data);
        data += 4;

        // parse pb
        int offs = 0;
        uint64_t val = 0;
        while (offs < pblen)
        {
            // t - l -v
            val = 0;
            int tag_size = pb_read_int(&data[offs], pblen - offs, val);
            int fid = (int)val;
            //int ftype = fid & 0x7;
            fid = fid >> 3;
            offs += tag_size;
            if (fid == 1) // ResponseHeader
            {
                // length
                tag_size = pb_read_int(&data[offs], pblen - offs, val);
                offs += tag_size;
                // content
                tag_size = response.parse(&data[offs], (int)val);
                offs += tag_size;
            }
            else if (fid == 2) // token
            {
                // length
                tag_size = pb_read_int(&data[offs], pblen - offs, val);
                offs += tag_size;
                // content
                stoken.assign(&data[offs], val);
                offs += val;
            }
            else
            {
                //TODO: implement skip
                break;
            }
        }
        return len2 + GRPC_HEAD_SIZE;
    }
};


vetcd3::vetcd3()
{
    bstop_ = false;
    sock_ = -1;
    readlen_ = 0;
    reqbuf_ = new char[1024 * 1024];
    ansbuf_ = new char[1024 * 1024];
    tid_ = 0;
    svr_win_size_ = 0;
    svr_frame_size_ = 16384;
    cli_streamid_ = 1;
}

vetcd3::~vetcd3()
{
    bstop_ = true;
    if (tid_ != 0)
    {
#ifdef _MSC_VER
        WaitForSingleObject(tid_, -1);
        CloseHandle(tid_);
#else
        pthread_join(tid_, 0);
#endif
        tid_ = 0;
    }
#ifdef _MSC_VER
    closesocket(sock_);
#else
    ::close(sock_);
#endif
    sock_ = -1;
}

#ifdef _MSC_VER
DWORD WINAPI vetcd3_thr_recv(LPVOID param)
#else
void* vetcd3_thr_recv(void* param)
#endif
{
    vetcd3* _this = (vetcd3*)param;
    if (_this)
    {
        _this->run_recv_thread();
    }
    return 0;
}

bool vetcd3::connect(const std::string& str, std::string& serr)
{
    std::string saddr = str;
    if (saddr.find("ipv4:///") != std::string::npos)
    {
        saddr = saddr.substr(8, saddr.length());
    }
    int port = 2379;
    // ip:port
    int p = (int)saddr.find(':');
    if (p < 0)
    {
        serr = "addr format invalid, need ip:port or ipv4:///ip:port,ip:port";
        return false;
    }
    saddr_ = saddr;
    std::string sip = saddr.substr(0, p);
    std::string sport = saddr.substr(p + 1, saddr.length());
    port = atoi(sport.c_str());
    sock_ = vetcd_connect_to_ip(sip.c_str(), port);
    if (sock_ <= 0)
    {
#ifdef _MSC_VER
        serr = "connect failed, error:" + std::to_string(WSAGetLastError());
#else
        serr = "connect failed";
#endif
        return false;
    }

    char* data = (char*)reqbuf_;
    int total = 0;
    int len = magic_stream::fill(data);
    data += len;
    total += len;
    setting_stream ss(data);
    ss.fill_item(0x02, 0); // enable push
    ss.fill_item(0x03, 0); // max concurrent streams
    ss.fill_item(0x04, 0x400000); // initial windows size 4M
    ss.fill_item(0x05, 0x1000000); // max frame size 16M
    ss.fill_item(0x06, 8192); // max header list size
    ss.fill_item((short)65027, 1); // unkown
    len = ss.fill_over();
    data += len;
    total += len;
    len = window_stream::fill(data, 0, 0x400000);
    data += len;
    total += len;
    len = ping_stream::fill(data);
    data += len;
    total += len;

    // try send header first time (for test, not standard operate)
    //cli_streamid_ = 1;
    //header_stream hss(data, cli_streamid_);
    //hss.fill_item(":authority", str);
    //hss.fill_item(":path", "/etcdserverpb.KV/Put");
    ////hss.fill_item("te", "trailers");
    //hss.fill_item("content-type", "application/grpc");
    //hss.fill_item("user-agent", "grpc-c++/1.20.0 grpc-c/7.0.0 (linux; chttp2; godric)");
    ////hss.fill_item("grpc-accept-encoding", "identity,deflate,gzip");
    ////hss.fill_item("accept-encoding", "identity,gzip");
    //hss.fill_item("grpc-accept-encoding", "identity");
    //hss.fill_item("accept-encoding", "identity");
    //len = hss.fill_over();
    //data += len;
    //total += len;
    //cli_streamid_ += 2;

    int nsend = vetcd_send_sock(sock_, reqbuf_, total);
    if (nsend != total)
    {
        return false;
    }
    Sleep(32); // serial wait for all answer packets
    int nread = recv(sock_, ansbuf_, 0x100000, 0);
    parse_setting_stream pss;
    int nparse = 0;
    char* buf = ansbuf_;
    // maybe have 3~4 packet of setting stream
    while (nparse < nread)
    {
        int len = 0;
        pkt_head* head = (pkt_head*)buf;
        if (head->ntype == 0x04)
        {
            len = pss.parse(buf, nread - nparse);
            if (len <= 0)
            {
                return false;
            }
        }
        else if (head->ntype == 0x08)
        {
            len = window_stream::parse(buf, nread - nparse, svr_win_size_, 0x1000);
        }
        else if (head->ntype == 0x01) // header response
        {
            //HPACK::decoder_t dec;
            /*if (true == headdec_recv_.decode(head->data)) {
                for (auto& hdr : headdec_recv_.headers())
                    std::cout << hdr.first << ": " << hdr.second << std::endl;
            }*/
            len = head->nlen.getint(); // skip unknown/un-need packet;
            len += GRPC_HEAD_SIZE;
        }
        else
        {
            len = head->nlen.getint(); // skip unknown/un-need packet;
            len += GRPC_HEAD_SIZE;
        }
        buf += len;
        nparse += len;
    }
    svr_frame_size_ = pss.get(0x05, 7 * 1024 * 1024);
#ifdef _MSC_VER
    tid_ = CreateThread(NULL, 0, vetcd3_thr_recv, this, 0, NULL);
    if (tid_ == INVALID_HANDLE_VALUE)
    {
        closesocket(sock_);
        sock_ = -1;
        serr = "create recv thread failed, error:" + std::to_string(WSAGetLastError());
        tid_ = NULL;
        return false;
    }
#else
#endif

    return true;
}

bool vetcd3::connect_ex(const std::string& str, const std::string& suser, const std::string& spwd, std::string& serr)
{
    if (!connect(str, serr))
        return false;
    return login(suser, spwd, serr);
}

bool vetcd3::login(const std::string& suser, const std::string& spwd, std::string& serr)
{
#ifdef _MSC_VER
    int64_t streamid = ::InterlockedAdd64(&cli_streamid_, 2);
#else
    int64_t streamid = __sync_fetch_and_add(&cli_streamid_, 2);
#endif

    int len = 0;
    int total = 0;
    char* data = reqbuf_;

    header_stream hss(data, (int)streamid);
    hss.fill_item(":path", "/etcdserverpb.Auth/Authenticate");
    hss.fill_item("content-type", "application/grpc");
    //hss.fill_item("grpc-accept-encoding", "identity");
    //hss.fill_item("accept-encoding", "identity");
    len = hss.fill_over();
    data += len;
    total += len;
    // window update
    //len = window_stream::fill(data, (int)streamid, 5);
    //data += len;
    //total += len;
    // grpc data
    bool brange = true;
    len = auth_data_stream::fill(data, (int)streamid, suser, spwd);
    data += len;
    total += len;
    // window update stream 0, size = pre grpc content len = len - 9(head)
    len = window_stream::fill(data, 0, len - GRPC_HEAD_SIZE);
    data += len;
    total += len;

    auth_ans_stream* stm = new auth_ans_stream();
    add_stream(streamid, stm);

#ifdef _DEBUG
    char buf[256];
    sprintf(buf, "---send stream: %d, type: auth, user: %s, pwd: %s\n", streamid, suser.c_str(), spwd.c_str());
    OutputDebugStringA(buf);
#endif

    int nsend = vetcd_send_sock(sock_, reqbuf_, total);
    if (nsend != total)
    {
        serr = "send failed";
        remove_stream(streamid);
        return -1;
    }

    if (stm->wait() == false)
    {
        remove_stream(streamid);
        serr = "wait response timeout";
        return -1;
    }

    int retcode = stm->error_code;
    serr = stm->error_message;
    stoken_ = stm->stoken;
    remove_stream(streamid);
    if (retcode == -9)
        return true;//"etcdserver: authentication is not enabled"
    return (retcode >= 0);
}

void vetcd3::close()
{
    if (tid_ != NULL)
    {
#ifdef _MSC_VER
        bstop_ = true;
        closesocket(sock_);
        sock_ = -1;
        WaitForSingleObject(tid_, INFINITE);
        tid_ = NULL;
#else
        bstop_ = true;
        ::close(sock_);
		sock_ = -1;
        pthread_join(tid_, NULL);
        tid_ = NULL;
#endif
    }
}

int vetcd3::put(const std::string& skey, const std::string& sval, bool bmodify, std::string& serr)
{
    return put(skey, sval, 0, bmodify, serr);
}

int vetcd3::put(const std::string& skey, const std::string& sval, int64_t leaseid, bool bmodify, std::string& serr)
{
 #ifdef _MSC_VER
        int64_t streamid = ::InterlockedAdd64(&cli_streamid_, 2);
    #else
        int64_t streamid = __sync_fetch_and_add(&cli_streamid_, 2);
    #endif

    put_ans_stream* stm = new put_ans_stream();
    add_stream(streamid, stm);

    int len = 0;
    int total = 0;
    char* data = reqbuf_;

    header_stream hss(data, (int)streamid);
    hss.fill_item(":path", "/etcdserverpb.KV/Put");
    hss.fill_item("content-type", "application/grpc");
    if (!stoken_.empty()) hss.fill_item("token", stoken_);
    //hss.fill_item("grpc-accept-encoding", "identity");
    //hss.fill_item("accept-encoding", "identity");
    len = hss.fill_over();
    data += len;
    total += len;
    // window update
    //len = window_stream::fill(data, (int)streamid, 5);
    //data += len;
    //total += len;
    // grpc data
    if (sval.length() < 16384 - 1024)
    {
        len = put_data_stream::fill(data, (int)streamid, skey, sval, leaseid, bmodify);
        data += len;
        total += len;

        // window update stream 0, size = pre grpc content len = len - 9(head)
        len = window_stream::fill(data, 0, len - GRPC_HEAD_SIZE);
        data += len;
        total += len;

        int nsend = vetcd_send_sock(sock_, reqbuf_, total);
        if (nsend != total)
        {
            serr = "send failed";
            remove_stream(streamid);
            return -1;
        }
    }
    else // need split frame
    {
        // send header first
        int nsend = vetcd_send_sock(sock_, reqbuf_, total);
        if (nsend != total)
        {
            serr = "send failed";
            remove_stream(streamid);
            return -1;
        }
        data = reqbuf_;
        total = 0;
        len = 0;

        std::string sgrpcdata;
        put_data_stream::make_grpc_packet(sgrpcdata, skey, sval, leaseid, bmodify);
        const char* svdata = sgrpcdata.c_str();
        int svlen = (int)sgrpcdata.length();
        int nframe = 16384 - 9;
        while (svlen > 0)
        {
            int nlen = (svlen > nframe) ? nframe : svlen;
            svlen -= nlen;

            memset(data, 0, GRPC_HEAD_SIZE);
            data[3] = 0x00; // data stream
            if (svlen == 0) data[4] = 0x01; // end stream
            *(int*)&data[5] = vetcd_ReverseBytes32(streamid);

            char* plen = (char*)&nlen;
            data[0] = plen[2];
            data[1] = plen[1];
            data[2] = plen[0];

            memcpy(&data[9], svdata, nlen);
            svdata += nlen;
            total = nlen + 9;

            nsend = vetcd_send_sock(sock_, reqbuf_, total);
            if (nsend != total)
            {
                serr = "send failed";
                remove_stream(streamid);
                return -1;
            }

            data = reqbuf_;
            total = 0;
            len = 0;
        }

    }


    if (stm->wait() == false)
    {
        remove_stream(streamid);
        serr = "wait response timeout";
        return -1;
    }
    int retcode = stm->error_code;
    serr = stm->error_message;
    remove_stream(streamid);
    return retcode;
}

int vetcd3::get(const std::string& skey, std::string& sval, std::string& serr)
{
    #ifdef _MSC_VER
        int64_t streamid = ::InterlockedAdd64(&cli_streamid_, 2);
    #else
        int64_t streamid = __sync_fetch_and_add(&cli_streamid_, 2);
    #endif

    int len = 0;
    int total = 0;
    char* data = reqbuf_;

    header_stream hss(data, (int)streamid);
    hss.fill_item(":path", "/etcdserverpb.Auth/Authenticate");
    hss.fill_item("content-type", "application/grpc");
    if (!stoken_.empty()) hss.fill_item("token", stoken_);
    //hss.fill_item("grpc-accept-encoding", "identity");
    //hss.fill_item("accept-encoding", "identity");
    len = hss.fill_over();
    data += len;
    total += len;
    // window update
    //len = window_stream::fill(data, (int)streamid, 5);
    //data += len;
    //total += len;
    // grpc data
    len = get_data_stream::fill(data, (int)streamid, skey, false);
    data += len;
    total += len;
    // window update stream 0, size = pre grpc content len = len - 9(head)
    len = window_stream::fill(data, 0, len - GRPC_HEAD_SIZE);
    data += len;
    total += len;

    get_ans_stream* stm = new get_ans_stream();
    add_stream(streamid, stm);

    int nsend = vetcd_send_sock(sock_, reqbuf_, total);
    if (nsend != total)
    {
        serr = "send failed";
        remove_stream(streamid);
        return -1;
    }

    if (stm->wait() == false)
    {
        remove_stream(streamid);
        serr = "wait response timeout";
        return -1;
    }
    sval = stm->get_first_value();
    int retcode = stm->error_code;
    serr = stm->error_message;
    remove_stream(streamid);
    return retcode;
}

int vetcd3::get_history(const std::string& skey, int ver, int& mod_ver, std::string& sval, std::string& serr)
{
#ifdef _MSC_VER
    int64_t streamid = ::InterlockedAdd64(&cli_streamid_, 2);
#else
    int64_t streamid = __sync_fetch_and_add(&cli_streamid_, 2);
#endif
    mod_ver = ver;

    int len = 0;
    int total = 0;
    char* data = reqbuf_;

    header_stream hss(data, (int)streamid);
    hss.fill_item(":path", "/etcdserverpb.KV/Range");
    hss.fill_item("content-type", "application/grpc");
    if (!stoken_.empty()) hss.fill_item("token", stoken_);
    //hss.fill_item("grpc-accept-encoding", "identity");
    //hss.fill_item("accept-encoding", "identity");
    len = hss.fill_over();
    data += len;
    total += len;
    // window update
    //len = window_stream::fill(data, (int)streamid, 5);
    //data += len;
    //total += len;
    // grpc data
    len = get_data_stream::fill(data, (int)streamid, skey, false, ver);
    data += len;
    total += len;
    // window update stream 0, size = pre grpc content len = len - 9(head)
    len = window_stream::fill(data, 0, len - GRPC_HEAD_SIZE);
    data += len;
    total += len;

    get_ans_stream* stm = new get_ans_stream();
    add_stream(streamid, stm);

    int nsend = vetcd_send_sock(sock_, reqbuf_, total);
    if (nsend != total)
    {
        serr = "send failed";
        remove_stream(streamid);
        return -1;
    }

    if (stm->wait() == false)
    {
        remove_stream(streamid);
        serr = "wait response timeout";
        return -1;
    }
    sval = stm->get_first_value();
    int retcode = stm->error_code;
    serr = stm->error_message;
    if (stm->arritem.empty() == false)
        mod_ver = stm->arritem[0].nmod_rev;
    remove_stream(streamid);
    return retcode;
}


int vetcd3::del(const std::string& skey, std::string& serr)
{
    #ifdef _MSC_VER
        int64_t streamid = ::InterlockedAdd64(&cli_streamid_, 2);
    #else
        int64_t streamid = __sync_fetch_and_add(&cli_streamid_, 2);
    #endif

    int len = 0;
    int total = 0;
    char* data = reqbuf_;

    header_stream hss(data, (int)streamid);
    hss.fill_item(":path", "/etcdserverpb.KV/DeleteRange");
    hss.fill_item("content-type", "application/grpc");
    if (!stoken_.empty()) hss.fill_item("token", stoken_);
    //hss.fill_item("grpc-accept-encoding", "identity");
    //hss.fill_item("accept-encoding", "identity");
    len = hss.fill_over();
    data += len;
    total += len;
    // window update
    //len = window_stream::fill(data, (int)streamid, 5);
    //data += len;
    //total += len;
    // grpc data
    len = delete_data_stream::fill(data, (int)streamid, skey);
    data += len;
    total += len;
    // window update stream 0, size = pre grpc content len = len - 9(head)
    len = window_stream::fill(data, 0, len - GRPC_HEAD_SIZE);
    data += len;
    total += len;

    delete_ans_stream* stm = new delete_ans_stream();
    add_stream(streamid, stm);

    int nsend = vetcd_send_sock(sock_, reqbuf_, total);
    if (nsend != total)
    {
        serr = "send failed";
        remove_stream(streamid);
        return -1;
    }

    if (stm->wait() == false)
    {
        remove_stream(streamid);
        serr = "wait response timeout";
        return -1;
    }
    int retcode = stm->error_code;
    serr = stm->error_message;
    remove_stream(streamid);
    return retcode;
}

int vetcd3::ls_all(std::vector<vetcd_keyvalue>& arrout, std::string& serr)
{
    //TODO: etcd 不支持授权所有key到角色, ""这个无法授权, 开启认证的时候就查不到了
    // 一个方式是把root角色授权给普通用户
    // etcd3 i have not find the way support grant path "" for role, so can not ls all key for admin
    // we assume user grant key at least with prefix /
    int ret = ls_prefix("", arrout, serr);
    if (ret < 0 && stoken_.empty() == false) // retry 
    {
        return ls_prefix("/", arrout, serr);
    }
    return ret;
}

int vetcd3::ls_prefix(const std::string& skey, std::vector<vetcd_keyvalue>& arrout, std::string& serr)
{
    #ifdef _MSC_VER
        int64_t streamid = ::InterlockedAdd64(&cli_streamid_, 2);
    #else
        int64_t streamid = __sync_fetch_and_add(&cli_streamid_, 2);
    #endif

    int len = 0;
    int total = 0;
    char* data = reqbuf_;

    header_stream hss(data, (int)streamid);
    hss.fill_item(":path", "/etcdserverpb.KV/Range");
    hss.fill_item("content-type", "application/grpc");
    if (!stoken_.empty()) hss.fill_item("token", stoken_);
    //hss.fill_item("grpc-accept-encoding", "identity");
    //hss.fill_item("accept-encoding", "identity");
    len = hss.fill_over();
    data += len;
    total += len;
    // window update
    //len = window_stream::fill(data, (int)streamid, 5);
    //data += len;
    //total += len;
    // grpc data
    bool brange = true;
    len = get_data_stream::fill(data, (int)streamid, skey, brange);
    data += len;
    total += len;
    // window update stream 0, size = pre grpc content len = len - 9(head)
    len = window_stream::fill(data, 0, len - GRPC_HEAD_SIZE);
    data += len;
    total += len;

    get_ans_stream* stm = new get_ans_stream();
    add_stream(streamid, stm);

#ifdef _DEBUG
    char buf[256];
    sprintf(buf, "---send stream: %d, type: ls_prefix, str: %s\n", streamid, skey.c_str());
    OutputDebugStringA(buf);
#endif

    int nsend = vetcd_send_sock(sock_, reqbuf_, total);
    if (nsend != total)
    {
        serr = "send failed";
        remove_stream(streamid);
        return -1;
    }

    if (stm->wait() == false)
    {
        remove_stream(streamid);
        serr = "wait response timeout";
        return -1;
    }
    arrout = stm->arritem;
    int retcode = stm->error_code;
    serr = stm->error_message;
    remove_stream(streamid);
    return retcode;
}

int vetcd3::watch(const std::string& skey, void* cb, void* cbdata, int64_t& watchid, std::string& serr)
{
    #ifdef _MSC_VER
        int64_t streamid = ::InterlockedAdd64(&cli_streamid_, 2);
    #else
        int64_t streamid = __sync_fetch_and_add(&cli_streamid_, 2);
    #endif

    int len = 0;
    int total = 0;
    char* data = reqbuf_;

    header_stream hss(data, (int)streamid);
    hss.fill_item(":path", "/etcdserverpb.Watch/Watch");
    hss.fill_item("content-type", "application/grpc");
    if (!stoken_.empty()) hss.fill_item("token", stoken_); // 如果没有权限, watch会失败, 但是返回的code是 0, 没有错误信息?
    //hss.fill_item("grpc-accept-encoding", "identity");
    //hss.fill_item("accept-encoding", "identity");
    len = hss.fill_over();
    data += len;
    total += len;
    // window update
    //len = window_stream::fill(data, (int)streamid, 5);
    //data += len;
    //total += len;
    // grpc data
    len = watch_data_stream::fill(data, (int)streamid, skey);
    data += len;
    total += len;
    // window update stream 0, size = pre grpc content len = len - 9(head)
    len = window_stream::fill(data, 0, len - GRPC_HEAD_SIZE);
    data += len;
    total += len;

    watch_ans_stream* stm = new watch_ans_stream();
    stm->watch_cb = (cb_vetcd_watch)cb;
    stm->cb_data = cbdata;
    add_stream(streamid, stm);

    int nsend = vetcd_send_sock(sock_, reqbuf_, total);
    if (nsend != total)
    {
        serr = "send failed";
        remove_stream(streamid);
        return -1;
    }
    if (stm->wait() == false)
    {
        serr = "wait response time out";
        remove_stream(streamid);
        return -1;
    }

    watchid = stm->watch_id_;
    int retcode = stm->error_code;
    serr = stm->error_message;
    if (watchid < 0)
        remove_stream(streamid);
    else
        watchid = streamid;
    // else keep stream watch
    return retcode;
}

int vetcd3::stop_watch(int64_t watchstreamid, std::string& serr)
{
    if (remove_stream(watchstreamid) == false)
    {
        serr = "watch stream not exists";
        return -1;
    }

    int len = 0;
    int total = 0;
    char* data = reqbuf_;

    // grpc data
    len = watch_data_stream::fill_rst(data, (int)watchstreamid);
    data += len;
    total += len;
    // window update stream 0, size = pre grpc content len = len - 9(head)
    len = window_stream::fill(data, 0, len - GRPC_HEAD_SIZE);
    data += len;
    total += len;

    int nsend = vetcd_send_sock(sock_, reqbuf_, total);
    if (nsend != total)
    {
        serr = "send failed";
        return -1;
    }

    return 1;
}

int vetcd3::create_lease(int ttl, int64_t& leaseid, std::string& serr)
{
    #ifdef _MSC_VER
        int64_t streamid = ::InterlockedAdd64(&cli_streamid_, 2);
    #else
        int64_t streamid = __sync_fetch_and_add(&cli_streamid_, 2);
    #endif

    int len = 0;
    int total = 0;
    char* data = reqbuf_;

    header_stream hss(data, (int)streamid);
    hss.fill_item(":path", "/etcdserverpb.Lease/LeaseGrant");
    hss.fill_item("content-type", "application/grpc");
    if (!stoken_.empty()) hss.fill_item("token", stoken_);
    //hss.fill_item("grpc-accept-encoding", "identity");
    //hss.fill_item("accept-encoding", "identity");
    len = hss.fill_over();
    data += len;
    total += len;
    // window update
    //len = window_stream::fill(data, (int)streamid, 5);
    //data += len;
    //total += len;
    // grpc data

    std::string skey;
    len = lease_data_stream::fill(data, (int)streamid, ttl);
    data += len;
    total += len;
    // window update stream 0, size = pre grpc content len = len - 9(head)
    len = window_stream::fill(data, 0, len - GRPC_HEAD_SIZE);
    data += len;
    total += len;

    lease_ans_stream* stm = new lease_ans_stream();
    add_stream(streamid, stm);

    int nsend = vetcd_send_sock(sock_, reqbuf_, total);
    if (nsend != total)
    {
        serr = "send failed";
        remove_stream(streamid);
        return -1;
    }
    if (stm->wait() == false)
    {
        serr = "wait response time out";
        remove_stream(streamid);
        return -1;
    }

    leaseid = stm->lease_id_;
    int retcode = stm->error_code;
    serr = stm->error_message;
    remove_stream(streamid);
    return retcode;
}
int vetcd3::update_lease(int64_t leaseid, std::string& serr)
{
    #ifdef _MSC_VER
        int64_t streamid = ::InterlockedAdd64(&cli_streamid_, 2);
    #else
        int64_t streamid = __sync_fetch_and_add(&cli_streamid_, 2);
    #endif

    int len = 0;
    int total = 0;
    char* data = reqbuf_;

    header_stream hss(data, (int)streamid);
    hss.fill_item(":path", "/etcdserverpb.Lease/LeaseKeepAlive");
    hss.fill_item("content-type", "application/grpc");
    if (!stoken_.empty()) hss.fill_item("token", stoken_);
    //hss.fill_item("grpc-accept-encoding", "identity");
    //hss.fill_item("accept-encoding", "identity");
    len = hss.fill_over();
    data += len;
    total += len;
    // window update
    //len = window_stream::fill(data, (int)streamid, 5);
    //data += len;
    //total += len;
    // grpc data

    std::string skey;
    len = leasekeepalive_data_stream::fill(data, (int)streamid, leaseid);
    data += len;
    total += len;
    // window update stream 0, size = pre grpc content len = len - 9(head)
    len = window_stream::fill(data, 0, len - GRPC_HEAD_SIZE);
    data += len;
    total += len;

    lease_ans_stream* stm = new lease_ans_stream();
    add_stream(streamid, stm);

    int nsend = vetcd_send_sock(sock_, reqbuf_, total);
    if (nsend != total)
    {
        serr = "send failed";
        remove_stream(streamid);
        return -1;
    }
    if (stm->wait() == false)
    {
        serr = "wait response time out";
        remove_stream(streamid);
        return -1;
    }
    serr = stm->error_message;
    // server always return 0, so use ttl for result check
    int retcode = stm->error_code;
    if (retcode == 0 && stm->ttl_ <= 0)
    {
        retcode = -1;
        serr = "leaseid invalid";
    }
    remove_stream(streamid);
    return retcode;
}
int vetcd3::revoke_lease(int64_t leaseid, std::string& serr)
{
#ifdef _MSC_VER
    int64_t streamid = ::InterlockedAdd64(&cli_streamid_, 2);
#else
    int64_t streamid = __sync_fetch_and_add(&cli_streamid_, 2);
#endif
    int len = 0;
    int total = 0;
    char* data = reqbuf_;

    header_stream hss(data, (int)streamid);
    hss.fill_item(":path", "/etcdserverpb.Lease/LeaseRevoke");
    hss.fill_item("content-type", "application/grpc");
    if (!stoken_.empty()) hss.fill_item("token", stoken_);
    //hss.fill_item("grpc-accept-encoding", "identity");
    //hss.fill_item("accept-encoding", "identity");
    len = hss.fill_over();
    data += len;
    total += len;
    // window update
    //len = window_stream::fill(data, (int)streamid, 5);
    //data += len;
    //total += len;
    // grpc data

    std::string skey;
    len = leasekeepalive_data_stream::fill(data, (int)streamid, leaseid);
    data += len;
    total += len;
    // window update stream 0, size = pre grpc content len = len - 9(head)
    len = window_stream::fill(data, 0, len - GRPC_HEAD_SIZE);
    data += len;
    total += len;

    lease_ans_stream* stm = new lease_ans_stream();
    add_stream(streamid, stm);

    int nsend = vetcd_send_sock(sock_, reqbuf_, total);
    if (nsend != total)
    {
        serr = "send failed";
        remove_stream(streamid);
        return -1;
    }
    if (stm->wait() == false)
    {
        serr = "wait response time out";
        remove_stream(streamid);
        return -1;
    }
    serr = stm->error_message;
    int retcode = stm->error_code;
    remove_stream(streamid);
    return retcode;
}

int vetcd3::get_lease_ttl(int64_t leaseid, int& init_ttl, int& cur_ttl, std::string& serr)
{
    init_ttl = 0;
    cur_ttl = 0;
    #ifdef _MSC_VER
        int64_t streamid = ::InterlockedAdd64(&cli_streamid_, 2);
    #else
        int64_t streamid = __sync_fetch_and_add(&cli_streamid_, 2);
    #endif

    int len = 0;
    int total = 0;
    char* data = reqbuf_;

    header_stream hss(data, (int)streamid);
    hss.fill_item(":path", "/etcdserverpb.Lease/LeaseTimeToLive");
    hss.fill_item("content-type", "application/grpc");
    if (!stoken_.empty()) hss.fill_item("token", stoken_);
    //hss.fill_item("grpc-accept-encoding", "identity");
    //hss.fill_item("accept-encoding", "identity");
    len = hss.fill_over();
    data += len;
    total += len;
    // window update
    //len = window_stream::fill(data, (int)streamid, 5);
    //data += len;
    //total += len;
    // grpc data

    std::string skey;
    bool bgetkeys = false;
    len = leasettl_data_stream::fill(data, (int)streamid, leaseid, bgetkeys);
    data += len;
    total += len;
    // window update stream 0, size = pre grpc content len = len - 9(head)
    len = window_stream::fill(data, 0, len - GRPC_HEAD_SIZE);
    data += len;
    total += len;

    leasettl_ans_stream* stm = new leasettl_ans_stream();
    add_stream(streamid, stm);

    int nsend = vetcd_send_sock(sock_, reqbuf_, total);
    if (nsend != total)
    {
        serr = "send failed";
        remove_stream(streamid);
        return -1;
    }
    if (stm->wait() == false)
    {
        serr = "wait response time out";
        remove_stream(streamid);
        return -1;
    }
    init_ttl = stm->ttl_;
    cur_ttl = stm->cur_ttl_;
    serr = stm->error_message;
    int retcode = stm->error_code;
    remove_stream(streamid);
    return retcode;
}

int vetcd3::ls_server_member(std::vector<vetcd_member>& arrout, std::string& serr)
{
    #ifdef _MSC_VER
        int64_t streamid = ::InterlockedAdd64(&cli_streamid_, 2);
    #else
        int64_t streamid = __sync_fetch_and_add(&cli_streamid_, 2);
    #endif

    int len = 0;
    int total = 0;
    char* data = reqbuf_;

    header_stream hss(data, (int)streamid);
    hss.fill_item(":path", "/etcdserverpb.Cluster/MemberList");
    hss.fill_item("content-type", "application/grpc");
    if (!stoken_.empty()) hss.fill_item("token", stoken_);
    //hss.fill_item("grpc-accept-encoding", "identity");
    //hss.fill_item("accept-encoding", "identity");
    len = hss.fill_over();
    data += len;
    total += len;
    // window update
    len = window_stream::fill(data, (int)streamid, 5);
    data += len;
    total += len;
    // grpc data, it's blank packet of member list request
    len = member_data_stream::fill(data, (int)streamid);
    data += len;
    total += len;
    // window update stream 0, size = pre grpc content len = len - 9(head)
    len = window_stream::fill(data, 0, len - GRPC_HEAD_SIZE);
    data += len;
    total += len;

    member_ans_stream* stm = new member_ans_stream();
    add_stream(streamid, stm);

    int nsend = vetcd_send_sock(sock_, reqbuf_, total);
    if (nsend != total)
    {
        serr = "send failed";
        remove_stream(streamid);
        return -1;
    }

    if (stm->wait() == false)
    {
        remove_stream(streamid);
        serr = "wait response timeout";
        return -1;
    }
    arrout = stm->arritem;
    int retcode = stm->error_code;
    serr = stm->error_message;
    remove_stream(streamid);
    return retcode;
}

void vetcd3::run_recv_thread()
{
    int ntotal = 0;
    int nrecv = 0, noffs = 0, nremain = 0;
    pkt_head* phead = (pkt_head*)ansbuf_;
    HPACK::decoder_t local_dec;
    http2_stream* stm = NULL;
    std::unordered_map<int64_t, http2_stream*>::iterator it;
    while (!bstop_)
    {
        nrecv = recv(sock_, &ansbuf_[ntotal], 0x100000 - ntotal, 0);
        if (nrecv <= 0)
        {
            printf("connection close\n");
            break;
        }
        ntotal += nrecv;

        noffs = 0;
        nremain = 0;
        phead = (pkt_head*)&ansbuf_[noffs];
        while (noffs < ntotal)
        {
            nremain = ntotal - noffs;
            if (nremain < GRPC_HEAD_SIZE)
                continue;
            //if (phead->nlen.getint() + GRPC_HEAD_SIZE > svr_frame_size_)
            //{
            //    printf("invalid svr packet size, bigger than svr max:%d \n", svr_frame_size_);
            //    goto lab_end;
            //}
            if (phead->nlen.getint() + GRPC_HEAD_SIZE > 16 * 1024 * 1024)
            {
                printf("invalid svr packet size, bigger than 16M \n");
                goto lab_end;
            }
            if (phead->nlen.getint() + GRPC_HEAD_SIZE > nremain)
            {
                // not enough
                break;
            }
            conv_pkthead(phead);

            stm = NULL;
            {
                lock_mapstream_.Lock();
                it = map_stream_.find(phead->nstream);
                if (it != map_stream_.end())
                {
                    stm = (http2_stream*)it->second;
                    stm->AddRef();
                }
                lock_mapstream_.Unlock();
            }
            if (phead->nlen.getint() == 16384)
            {
                int a = 1;
            }
            if (stm)
            {
                stm->parse((char*)phead, phead->nlen.getint() + GRPC_HEAD_SIZE, local_dec);
                stm->Release();
            }
            //else
            {
#ifdef _DEBUG
                char buf[256];
                sprintf(buf, "---recv stream: %d, type: %s, flag: %d, len: %d, total: %d, stm: %p\n", phead->nstream, etcd_typename(phead->ntype).c_str(), phead->nflags, phead->nlen.getint(), ntotal, stm);
                OutputDebugStringA(buf);
#endif
            }

            noffs += GRPC_HEAD_SIZE + phead->nlen.getint();
            phead = (pkt_head*)&ansbuf_[noffs];
        }
        ntotal -= noffs;
        if (ntotal && noffs > 0) memmove(ansbuf_, &ansbuf_[noffs], ntotal);
    }
lab_end:
    std::vector<http2_stream*> arr;
    lock_mapstream_.Lock();
    for (auto& it2: map_stream_)
    {
        stm = (http2_stream*)it2.second;
        stm->AddRef();
        arr.push_back(stm);
    }
    map_stream_.clear();
    lock_mapstream_.Unlock();
    for (auto& it: arr)
    {
        stm = it;
        stm->error_code = -1;
        stm->error_message = "connection lost";
        stm->notify_response();
        stm->Release();
    }
    return;
}

void vetcd3::add_stream(int64_t streamid, http2_stream* stm)
{
    lock_mapstream_.Lock();
    auto it = map_stream_.find(streamid);
    if (it != map_stream_.end())
    {
        // if bug, remove old stream
        it->second->Release();
        map_stream_.erase(it);
    }
    map_stream_[streamid] = stm;
    lock_mapstream_.Unlock();
}

bool vetcd3::remove_stream(int64_t streamid)
{
    bool rs = false;
    http2_stream* stm = NULL;

    lock_mapstream_.Lock();
    auto it = map_stream_.find(streamid);
    if (it != map_stream_.end())
    {
        stm = it->second;
        map_stream_.erase(it);
        rs = true;
    }
    lock_mapstream_.Unlock();

    if (stm) stm->Release();
    return rs;
}

std::string vetcd3::get_token()
{
    return stoken_;
}
