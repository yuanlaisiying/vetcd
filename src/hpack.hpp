#pragma once
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <utility>
#include <vector>
#include <array>
#include <deque>
#include <map>
#include <limits>
#include <string>
#undef max
namespace HPACK
{
	typedef std::pair< const std::string, const std::string > header_t;
	typedef std::vector< bool > bits_t;
	
	static const std::array< header_t, 62 > predefined_headers = {
		{
			header_t("INVALIDINDEX", "INVALIDINDEX"), header_t(":authority", ""), header_t(":method", "GET"),
			header_t(":method", "POST"), header_t(":path", "/"), header_t(":path", "/index.html"),
			header_t(":scheme", "http"), header_t(":scheme", "https"), header_t(":status", "200"),
			header_t(":status", "204"), header_t(":status", "206"), header_t(":status", "304"),
			header_t(":status", "400"), header_t(":status", "404"), header_t(":status", "500"),
			header_t("accept-charset", ""), header_t("accept-encoding", "gzip, deflate"), header_t("accept-language", ""),
			header_t("accept-ranges", ""), header_t("accept", ""), header_t("access-control-allow-origin", ""),
			header_t("age", ""), header_t("allow", ""), header_t("authorization", ""),
			header_t("cache-control", ""), 	header_t("content-disposition", ""), header_t("content-encoding", ""),
			header_t("content-language", ""), header_t("content-length", ""), header_t("content-location", ""),
			header_t("content-range", ""), header_t("content-type", ""), header_t("cookie", ""),
			header_t("date", ""), header_t("etag", ""), header_t("expect", ""),
			header_t("expires", ""), header_t("from", ""), header_t("host", ""),
			header_t("if-match", ""), header_t("if-modified-since", ""), header_t("if-none-match", ""),
			header_t("if-range", ""), header_t("if-unmodified-since", ""), header_t("last-modified", ""),
			header_t("link", ""), header_t("location", ""), header_t("max-forwards", ""),
			header_t("proxy-authenticate", ""), header_t("proxy-authorization", ""), header_t("range", ""),
			header_t("referer", ""), header_t("refresh", ""), header_t("retry-after", ""),
			header_t("server", ""), header_t("set-cookie", ""), header_t("strict-transport-security", ""),
			header_t("transfer-encoding", ""), header_t("user-agent", ""), header_t("vary", ""),
			header_t("via", ""), header_t("www-authenticate", "")
		}
	};

	// 256 chars plus end of string
	static const std::array< bits_t, 257 > huffman_table = {
		{
			{ 1,1,1,1,1,1,1,1,1,1,0,0,0 },											// 0
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,0,0,0 },						// 1
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,0 },			// 2
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1 },			// 3
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,0,0 },			// 4
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,0,1 },			// 5
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,0 },			// 6
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1 },			// 7
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,0,0 },			// 8
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,0 },					// 9
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0 },		// 10
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,0,1 },			// 11
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,0 },			// 12
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1 },		// 13
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,1 },			// 14
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,0,0 },			// 15
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,0,1 },			// 16
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,0 },			// 17
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1 },			// 18
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0 },			// 19
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1 },			// 20
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,0 },			// 21
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0 },		// 22
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1 },			// 23
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,0 },			// 24
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1 },			// 25
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,0 },			// 26
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1 },			// 27
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0 },			// 28
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1 },			// 29
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0 },			// 30
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1 },			// 31
			{ 0,1,0,1,0,0 },														// 32 ' '
			{ 1,1,1,1,1,1,1,0,0,0 },												// 33 '!'
			{ 1,1,1,1,1,1,1,0,0,1 },												// 34 '"'
			{ 1,1,1,1,1,1,1,1,1,0,1,0 },											// 35 '#'
			{ 1,1,1,1,1,1,1,1,1,1,0,0,1 },											// 36 '$'
			{ 0,1,0,1,0,1 },														// 37 '%'	
			{ 1,1,1,1,1,0,0,0 },													// 38 '&'
			{ 1,1,1,1,1,1,1,1,0,1,0 },												// 39 '''
			{ 1,1,1,1,1,1,1,0,1,0 },												// 40 '('
			{ 1,1,1,1,1,1,1,0,1,1 },												// 41 ')'
			{ 1,1,1,1,1,0,0,1 },													// 42 '*'
			{ 1,1,1,1,1,1,1,1,0,1,1 },												// 43 '+'	
			{ 1,1,1,1,1,0,1,0 },													// 44 ','
			{ 0,1,0,1,1,0 },														// 45 '-'
			{ 0,1,0,1,1,1 },														// 46 '.'
			{ 0,1,1,0,0,0 },														// 47 '/'
			{ 0,0,0,0,0 },															// 48 '0'
			{ 0,0,0,0,1 },															// 49 '1'
			{ 0,0,0,1,0 },															// 50 '2'
			{ 0,1,1,0,0,1 },														// 51 '3'
			{ 0,1,1,0,1,0 },														// 52 '4'
			{ 0,1,1,0,1,1 },														// 53 '5'
			{ 0,1,1,1,0,0 },														// 54 '6'
			{ 0,1,1,1,0,1 },														// 55 '7'
			{ 0,1,1,1,1,0 },														// 56 '8'
			{ 0,1,1,1,1,1 },														// 57 '9'
			{ 1,0,1,1,1,0,0 },														// 58 ':'
			{ 1,1,1,1,1,0,1,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,0,0 },
			{ 1,0,0,0,0,0 },
			{ 1,1,1,1,1,1,1,1,1,0,1,1 },
			{ 1,1,1,1,1,1,1,1,0,0 },
			{ 1,1,1,1,1,1,1,1,1,1,0,1,0 },
			{ 1,0,0,0,0,1 },
			{ 1,0,1,1,1,0,1 },
			{ 1,0,1,1,1,1,0 },
			{ 1,0,1,1,1,1,1 },
			{ 1,1,0,0,0,0,0 },
			{ 1,1,0,0,0,0,1 },
			{1,1,0,0,0,1,0 },
			{ 1,1,0,0,0,1,1 },
			{ 1,1,0,0,1,0,0 },
			{ 1,1,0,0,1,0,1 },
			{ 1,1,0,0,1,1,0 },
			{ 1,1,0,0,1,1,1 },
			{ 1,1,0,1,0,0,0 },
			{ 1,1,0,1,0,0,1 },
			{ 1,1,0,1,0,1,0 },
			{ 1,1,0,1,0,1,1 },
			{ 1,1,0,1,1,0,0 },
			{ 1,1,0,1,1,0,1 },
			{ 1,1,0,1,1,1,0 },
			{ 1,1,0,1,1,1,1 },
			{ 1,1,1,0,0,0,0 },
			{ 1,1,1,0,0,0,1 },
			{ 1,1,1,0,0,1,0 },
			{ 1,1,1,1,1,1,0 },
			{ 1,1,1,0,0,1,1 },
			{ 1,1,1,1,1,1,0 },
			{ 1,1,1,1,1,1,1,1,1,1,0,1,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,0,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,0,0 },
			{ 1,0,0,0,1,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,0,1 },
			{ 0,0,0,1,1 },
			{ 1,0,0,0,1,1 },
			{ 0,0,1,0,0 },
			{ 1,0,0,1,0,0 },
			{ 0,0,1,0,1 },
			{ 1,0,0,1,0,1 },
			{ 1,0,0,1,1,0 },
			{ 1,0,0,1,1,1 },
			{ 0,0,1,1,0 },
			{ 1,1,1,0,1,0,0 },
			{ 1,1,1,0,1,0,1 },
			{ 1,0,1,0,0,0 },
			{ 1,0,1,0,0,1 },
			{ 1,0,1,0,1,0 },
			{ 0,0,1,1,1 },
			{ 1,0,1,0,1,1 },
			{ 1,1,1,0,1,1,0 },
			{ 1,0,1,1,0,0 },
			{ 0,1,0,0,0 },
			{ 0,1,0,0,1 },
			{ 1,0,1,1,0,1 },
			{ 1,1,1,0,1,1,1 },
			{ 1,1,1,1,0,0,0 },
			{ 1,1,1,1,0,0,1 },
			{ 1,1,1,1,0,1,0 },
			{ 1,1,1,1,0,1,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,0 },
			{ 1,1,1,1,1,1,1,1,1,0,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,0,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,0,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,0,1,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,0,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,0,1,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,0,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,0,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,0,0,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,1,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,0,1,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,0,1,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,0,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,0,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,0,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,0,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,1,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,0,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,0,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,0,0,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,0,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,0,0,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,0,1,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,0,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,0,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,0,1,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,0,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,0,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,0,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,0,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,0,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,0,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,0,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,0,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,0,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,0,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,0,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,0,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,0,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,0,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,0,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,0,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,0,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,0,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,0,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,0,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,0,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,0,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,0,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,0,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,0,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,0,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,0,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,0,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,0,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,0 },
			{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 }
		}
	};

	class huffman_node_t
	{
		private:
		protected:
			huffman_node_t*	m_left;
			huffman_node_t* m_right;
			int16_t		m_code;
		public:
			huffman_node_t(huffman_node_t* l = nullptr, huffman_node_t* r = nullptr, int16_t c = -1) : m_left(l), m_right(r), m_code(c) { }
			virtual ~huffman_node_t(void) { m_left = nullptr; m_right = nullptr; m_code = 0; return; }
			int16_t code(void) const { return m_code; }
			void code(int16_t c) { m_code = c; return; }
			huffman_node_t* left(void) { return m_left; }
			void left(huffman_node_t* l) { m_left = l; return; }
			huffman_node_t* right(void) { return m_right; }
			void right(huffman_node_t* r) { m_right = r; return; }
	};

	class huffman_tree_t
	{
		private:
		protected:
			huffman_node_t* m_root;

			void 
			delete_node( huffman_node_t* n )
			{
				if ( nullptr != n->right() )
					delete_node(n->right());
				if ( nullptr != n->left() )
					delete_node(n->left());

				if ( nullptr == n->left() && nullptr == n->right() ) {
					delete n;
					n = nullptr;
				}

				return;
			}

		public:
			huffman_tree_t(void) : m_root(new huffman_node_t)
			{
				for ( std::size_t idx = 0; idx < huffman_table.size(); idx++ ) {
					const bits_t&		bits = huffman_table.at(idx);
					huffman_node_t*		current = m_root;

					for ( const auto& bit : bits ) {
						if ( true == bit ) {
							if ( nullptr == current->right() )
								current->right(new huffman_node_t);

							current = current->right();
						} else {
							if ( nullptr == current->left() )
								current->left(new huffman_node_t);

							current = current->left();
						}
					}

					current->code(static_cast< int16_t >( idx ));
				}
			}

			virtual ~huffman_tree_t(void) 
			{ 
				delete_node(m_root); 
				return; 
			}

			std::string
			decode(const std::string& src)
			{
				std::string			dst("");
				huffman_node_t*		current(m_root);

				if ( src.length() > std::numeric_limits< unsigned int >::max() )
					throw std::invalid_argument("HPACK::huffman_tree_t::decode(): Overly long input string");

				for ( unsigned int idx = 0; idx < src.length(); idx++ ) {
					for ( int8_t j = 7; j >= 0; j-- ) {
						if ( ( src[ idx ] & ( 1 << j ) ) > 0 ) {
							if ( nullptr == current->right() )
								throw std::runtime_error("HPACK::huffman_tree_t::decode(): Internal state error (right == nullptr)");
							current = current->right();
						} else {
							if ( nullptr == current->left() )
								throw std::runtime_error("HPACK::huffman_tree_t::decode(): Internal state error (left == nullptr)");

							current = current->left();
						}

						if ( current->code() >= 0 ) {
							uint16_t code = current->code();

							if ( 257 == code )
								dst += static_cast< uint8_t >( ( ( code & 0xFF00 ) >> 8 ) & 0xFF );
							
							dst += static_cast< uint8_t >( code & 0xFF );
							current = m_root;
						}
					}
				}

				return dst;
			}
	};

	class ringtable_t
	{
		private:
		protected:
			uint64_t				m_max;
			std::deque< header_t >	m_queue;

		public:
			// 4096 is the default table size per the HTTPv2 RFC
			ringtable_t(void) : m_max(4096) { }
			ringtable_t(uint64_t m) : m_max(m) { }
			virtual ~ringtable_t(void) { }

			void
			max(uint64_t m)
			{
				m_max = m;

				// the RFC dictates that we do this here,
				// so we do.
				while ( length() >= m_max ) {
					m_queue.pop_back();
				}

				return;
			}

			inline uint64_t
			max(void) const
			{
				return m_max;
			}

			inline uint64_t
			entries_count(void) const
			{
				return m_queue.size();
			}

			inline uint64_t
			length(void) const
			{
				uint64_t size(0);
	
				for ( auto& h : m_queue ) {
					uint64_t nl(h.first.length());
					uint64_t vl(h.second.length());
					uint64_t tl(0);

					// In practice it should basically never occur
					// that either of these exceptions are thrown and
					// its probably safe to remove the checks in most instances
					if ( vl > std::numeric_limits< uint64_t >::max() ||
						nl > std::numeric_limits< uint64_t >::max() - vl )
						throw std::runtime_error("HPACK::ringtable_t::length() Additive integer overflow encountered");

					tl = nl + vl;

					if ( tl > std::numeric_limits< uint64_t >::max() - size )
						throw std::runtime_error("HPACK::ringtable_t::length() Additive integer overflow encountered");

					size += tl;

				}

				return size;
			}

			void
			add(const header_t&  h)
			{
				//uint64_t size(h.first.length() + h.second.length());

				// In practice it should be basically implausible to trip these exceptions because
				// you would need 2^(sizeof(uint64_t)*8) bytes of memory to be in use, which in itself
				// will likely fail long before then. In other words its probably safe to remove
				// these checks for the forseeable future, but I left them in because technically I
				// should check even if its an absurd condition.
				if ( h.first.length() > std::numeric_limits< uint64_t >::max() - h.second.length() )
					throw std::runtime_error("HPACK::ringtable_t::add(): Additive integer overflow encountered.");

				// Again the RFC dictates when we resize the queue.
				while ( length() >= m_max ) {
					m_queue.pop_back();
				}

				m_queue.push_front(h);
				return;
			}

			void
			add(const std::string& n, const std::string& v)
			{
				header_t h(n, v);

				add(h);
				return;
			}

			void
			add(const char* n, const char* v)
			{
				std::string name(n), value(v);

				if ( nullptr == n || nullptr == v )
					throw std::runtime_error("HPACK::ringtable_t::add(): Invalid nullptr parameter(s)");

				add(name, value);
				return;
			}

			const header_t&
			at(uint64_t idx)
			{
				if ( idx > m_queue.size() ) {
					// It's not clear these checks even entirely make sense
					// the concern was that someone passes in an out-of-bounds
					// index, but thats because the static and dynamic
					// tables have their indices flattened into one, so we 
					// attempt to address that by subtracting the static 
					// headers index size if we are out of bounds, which
					// might yield a totally bogus index due to implementation
					// bug asking for an invalid index that just happens to line
					// up.
					if ( idx < predefined_headers.size() )
						throw std::invalid_argument("HPACK::ringtable_t::at(): Invalid/out-of-bounds index specified");

					idx -= predefined_headers.size();

					if ( idx > m_queue.size() )
						throw std::invalid_argument("HPACK::ringtable_t::at(): Invalid/out-of-bounds index specified");

				}

				return m_queue.at(static_cast< std::size_t >( idx ));
			}

			bool
			find(const header_t& h, int64_t& index) const
			{
				index = -1;

				if ( index > (int64_t)std::numeric_limits< std::size_t >::max() )
					throw std::invalid_argument("HPACK::ringtable_t::find(): Invalid/overlarge index which results in truncation");

				for ( std::size_t idx = 0; idx < m_queue.size(); idx++ ) {
					if ( !h.first.compare(m_queue.at(idx).first) && !h.second.compare(m_queue.at(idx).second) ) {
						index = predefined_headers.size() + idx;
						return true;
					} else if ( !h.first.compare(m_queue.at(idx).first) ) {
						index = predefined_headers.size() + idx;
						return false;
					}
				}

				return false;
			}
			bool get_header(const header_t** h, const std::size_t index)
			{
				if (index < predefined_headers.size()) {
					*h = &predefined_headers.at(index);
					return true;
				}
				else if (index >= predefined_headers.size() && index < predefined_headers.size() + m_queue.size())
				{
					*h = &m_queue.at(index - predefined_headers.size());
					return true;
				}
				return false;
			}
			const header_t& get_header(const std::size_t index) const
			{
				if ( index < predefined_headers.size() ) {
					return predefined_headers.at(index);
				} else if ( index > predefined_headers.size() && index < predefined_headers.size() + m_queue.size() )
					return m_queue.at(index - predefined_headers.size());

				throw std::runtime_error("HPACK::ringtable_t::get_header(): Invalid index/header not found");
			}

	};
	
	class huffman_encoder_t
	{
		private:
			uint8_t m_byte;
			uint8_t m_count;

		protected:
			inline bool 
			write_bit(uint8_t bit)
			{
				m_byte |= bit;
				m_count--;

				if (0 == m_count ) {
					m_count = 8;
					return true;
				} else
					m_byte <<= 1;

				return false;
			}

		public:
			huffman_encoder_t(void) : m_byte(0), m_count(8) { }
			virtual ~huffman_encoder_t(void) { }

			std::vector< uint8_t >
			encode(std::vector< uint8_t >& src)
			{
				std::vector< uint8_t > ret(0);

				for ( auto& byte : src ) {
					bits_t bits = huffman_table.at(byte);

					for (const auto& bit : bits ) {
						if ( true == write_bit(bit) ) {
							ret.push_back(m_byte);
							m_byte = 0;
							m_count = 8;
						}

					}
				}

				// Apparently the remainder unused bits 
				// are to be set to 1, some sources refer
				// to this as the EOS bit, but the code
				// for EOS is like 30-bits of 1's so its
				// clearly not the EOS code.
				if ( 8 != m_count && 0 != m_count) {
					m_byte = ( m_byte << ( m_count - 1 ) );
					m_byte |= ( 0xFF >> ( 8 - m_count ) );
					ret.push_back(m_byte);
					m_byte = 0;
					m_count = 8;
				}

				return ret;
			}

			std::vector< uint8_t >
			encode(const std::string& src) 
			{
				std::vector< uint8_t > s(src.begin(), src.end());
				return encode(s);
			}

			std::vector< uint8_t >
			encode(const char* ptr)
			{
				std::string str(ptr);

				if ( nullptr == ptr )
					throw std::invalid_argument("HPACK::huffman_encoder_t::encode(): Invalid nullptr parameter");

				return encode(str);
			}
	};

	/*! \Class The HPACK decoder class.
	 *  \Brief A wrapper class that ties together the static, dynamic tables and huffman
	 *  encoding such that one can pass in a HTTPv2 header block and retrieve a map of strings
	 *  that container the headers sent.
	 *
	 * \Warning Never Indexed code paths under tested.
	 */
	class decoder_t
	{
		private:
		protected:
			std::map< std::string, std::string >	m_headers;
			ringtable_t								m_dynamic;
			huffman_tree_t							m_huffman;

			#define dec_vec_itr_t std::vector<uint8_t>::iterator

			void
			decode_integer(dec_vec_itr_t& beg, dec_vec_itr_t& end, uint32_t& dst, uint8_t N)
			{
				const uint16_t two_N = static_cast< uint16_t >( std::pow(2, N) - 1 );
				dec_vec_itr_t&  current(beg);

				if ( current == end )
					throw std::invalid_argument("HPACK::decoder_t::decode_integer(): Attempted to decode integer when at end of input");

				dst = ( *current & two_N );

				if ( dst == two_N ) {
					uint64_t M = 0;

					for ( ; current < end; current++ ) {
						dst += ( ( *current & 0x7F ) << M );
						M += 7;

						if ( !( *current & 0x80 ) )
							break;
					}
				}

				beg = current+1;
				return;
			}
			int decode_int2(unsigned char* p, int len, uint32_t& dst, uint8_t N)
			{
				const uint16_t two_N = static_cast<uint16_t>(std::pow(2, N) - 1);
				if (len <= 0)
					return 0;
				unsigned char* data = p;
				unsigned char* dataend = data + len;
				dst = (*data & two_N);
				if (dst == two_N)
				{
					data++;
					uint64_t M = 0;
					for (; data < dataend; data++) {
						dst += ((*data & 0x7F) << M);
						M += 7;

						if (!(*data & 0x80))
							break;
					}
				}
				return (int)(data - p)+1;
			}

			std::string
			parse_string(dec_vec_itr_t& itr, dec_vec_itr_t& end)
			{
				std::string		dst("");
				unsigned int	len(*itr & 0x7F);
				bool			huff(( *itr & 0x80 ) == 0x80 ? true : false);
				dec_vec_itr_t	cur(itr);

				if ( itr == end )
					throw std::invalid_argument("HPACK::decoder_t::parse_string(): Attempted to parse string when already at end of input");

				for ( ++cur; cur != end; cur++ ) {
					dst += *cur;

					if ( cur - itr == len )
						break;
				}

				itr += len + 1;

				if ( true == huff )
					dst = m_huffman.decode(dst);

				return dst;
			}

			int  parse_str2(unsigned char* itr, int len2, std::string& sout)
			{
				unsigned char* pold = itr;
				std::string		dst("");
				sout = dst;
				unsigned int	len(*itr & 0x7F);
				bool			huff((*itr & 0x80) == 0x80 ? true : false);
				unsigned char* cur(itr);
				unsigned char* end = itr + len2;
				if (len <= 0)
					return 1;

				for (++cur; cur != end; cur++) {
					dst += *cur;

					if (cur - itr == len)
						break;
				}

				itr += len + 1;

				if (true == huff)
					dst = m_huffman.decode(dst);
				sout = dst;
				return (int)(itr - pold);
			}

		public:
			/*!
				\fn encoder_t(uint64_t max = 4096)
				\Brief Constructs the encoder

				\param max the maximum size of the dynamic table; unbounded and allowed to exceed RFC sizes
			*/
			decoder_t(int64_t max = 4096) : m_dynamic(max) { }
			virtual ~decoder_t(void) { }
			
			/*!
				\fn bool decode(const std::string&)
				\Brief Decodes the HTTPv2 Header Block contained within the parameter

				\param str the HTTPv2 Header Block
				\return True if decoding was successful, false if an error such as a protocol decoding error was encountered.

				\Warning Never indexed code paths were under tested.
			*/
			bool
			decode(const std::string& str)
			{
                std::vector< uint8_t > arr;
                arr.assign(str.begin(), str.end());
                return decode(arr);
				//return decode(std::vector< uint8_t >(str.begin(), str.end()));
			}

			/*!
				\fn bool decode(const std::string&)
				\Brief Decodes the HTTPv2 Header Block contained within the parameter

				\param ptr the HTTPv2 Header Block
				\return True if decoding was successful, false if an error such as a protocol decoding error was encountered.

				\Warning Never indexed code paths were under tested.
			*/
			bool
			decode(const char* ptr)
			{
				if ( nullptr == ptr )
					throw std::invalid_argument("HPACK::decoder_t::decode(): Invalid nullptr parameter");

				return decode(std::string(ptr));
			}
			bool decode2(const char* ptr, int len)
			{
				unsigned char* data = (unsigned char*)ptr;
				unsigned char c;
				int offs = 0;
				while (offs < len)
				{
					c = data[offs];
					if (c * 0xE0 == 0x20) // 6.3 Dynamic Table update
					{
						// TODO: support
						uint32_t size(0);

						int tag = decode_int2(&data[offs], len - offs, size, 5);

						if (size > m_dynamic.max()) {
							// decoding error
							return false;
						}
						offs += tag;
						m_dynamic.max(size);
					}
					else if (c & 0x80)// 6.1 Indexed Header Field Representation
					{
						uint32_t index(0);
						int tag = decode_int2(&data[offs], len - offs, index, 7);
						
						if (0 == index) {
							// decoding error
							return false;
						}
						offs += tag;
						const header_t* h;
						if (m_dynamic.get_header(&h, index))
						{
							m_headers[h->first] = h->second;
						}
						else
						{
							return false;
						}
					}
					else // try add to dynamic table
					{
						uint32_t index(0);
						std::string n("");
						bool baddtable = false;
						int tag = 0;
						if (0x40 == (c & 0xC0)) // 6.2.1 Literal Header Field with Incremental Indexing
							tag = decode_int2(&data[offs], len - offs, index, 6);
						else // 6.2.2 Literal Header Field without Indexing
							tag = decode_int2(&data[offs], len - offs, index, 4);
						offs += tag;
						if (0 != index)
						{
							const header_t* h;
							if (m_dynamic.get_header(&h, index))
							{
								n = h->first;
								baddtable = true;
							}
							else
							{
								return false;
							}
						}
						else
						{
							int tag = parse_str2(&data[offs], len - offs, n);
							offs += tag;
							baddtable = true;
						}

						std::string v;
						tag = parse_str2(&data[offs], len - offs, v);
						offs += tag;
						m_headers[n] = v;
						if (baddtable)
						{
							m_dynamic.add(n, m_headers[n]);
						}
					}
				}
				return true;
			}


			/*!
				\fn bool decode(const std::string&)
				\Brief Decodes the HTTPv2 Header Block contained within the parameter

				\param data the HTTPv2 Header Block
				\return True if decoding was successful, false if an error such as a protocol decoding error was encountered.

				\Warning Never indexed code paths were under tested.
			*/
			bool 
			decode(std::vector< uint8_t >& data)
			{

				if ( !data.size() )
					return false;
                std::vector< uint8_t >::iterator itr;
                std::vector< uint8_t >::iterator itrend = data.end();
				for (itr = data.begin(); itr != data.end(); /* itr++ */ ) {
					if ( 0x20 == ( *itr * 0xE0 ) ) { // 6.3 Dynamic Table update
						uint32_t size(0);

						decode_integer(itr, itrend, size, 5);

						if ( size > m_dynamic.max() ) {
							// decoding error
							return false;
						}

						m_dynamic.max(size);
					} else if ( ( *itr & 0x80 ) ) { // 6.1 Indexed Header Field Representation
						uint32_t index(0);

						decode_integer(itr, itrend, index, 7);

						if ( 0 == index ) {
							// decoding error
							return false;
						}
						const header_t* h;
						if (m_dynamic.get_header(&h, index))
						{
							m_headers[h->first] = h->second;
						}
						else
						{
							return false;
						}
						//m_headers[ m_dynamic.get_header(index).first ] = m_dynamic.get_header(index).second;
					} else {
						uint32_t index(0);
						std::string n("");
						bool baddtable = false;
						if ( 0x40 == ( *itr & 0xC0 ) ) // 6.2.1 Literal Header Field with Incremental Indexing
							decode_integer(itr, itrend, index, 6);
						else // 6.2.2 Literal Header Field without Indexing
							decode_integer(itr, itrend, index, 4);

						if (0 != index) {
							const header_t* h;
							if (m_dynamic.get_header(&h, index))
							{
								n = h->first;
								baddtable = true;
							}
							else
							{
								return false;
							}
							//header_t h = m_dynamic.get_header(index);

						} else {
							n = parse_string(itr, itrend);
							baddtable = true;
						}

						m_headers[ n ] = parse_string(itr, itrend);
						if (baddtable)
						{
							m_dynamic.add(n, m_headers[n]);
						}
					}
				}

				return true;
			}


			/*!
				\fn const std::map< std::string, std::string >& headers(void) const
				\Brief Retrieves the interally managed header map of decoded headers

				\Return The map of the decoded headers
			 */
			const std::map< std::string, std::string >&
			headers(void) const 
			{
				return m_headers;
			}
	};



#define INDEXED_BIT_PATTERN 0x80
#define LITERAL_INDEXED_BIT_PATTERN 0x40
#define LITERAL_WITHOUT_INDEXING_BIT_PATTERN 0x00
#define LITERAL_NEVER_INDEXED_BIT_PATTERN 0x10
#define HUFFMAN_ENCODED 0x80

	/*! \Class The HPACK encoder class. 
     *  \Brief A wrapper class that ties together the ringtable_t dynamic table implementation
	 * with the prior static table such that one can simply add(name, value) into an
	 * internally managed buffer (a std::vector< uint8_t >) which can be retrieved at
	 * the end of operations. Huffman encoding and dynamic table references are handled 
	 * automatically; 
	 * 
	 * \Warning Never Indexed code paths untested.
	 */
	class encoder_t
	{
		private:
		protected:
			std::vector< uint8_t >	m_buf;
			ringtable_t				m_dynamic;
			huffman_encoder_t		m_huffman;

			void
			huff_encode(const std::string& str)
			{
				std::vector< uint8_t >	huffbuff(0);
				//std::size_t				len(0);


				huffbuff = m_huffman.encode(str);	

				if ( 128 > huffbuff.size() )
					m_buf.push_back(static_cast< uint8_t >( HUFFMAN_ENCODED | huffbuff.size() ));
				else {
					std::vector< uint8_t > tmp;
					encode_integer(tmp, (uint32_t)huffbuff.size(), 7);
					tmp.front() |= HUFFMAN_ENCODED;
					m_buf.insert(m_buf.end(), tmp.begin(), tmp.end());
				}
				
				m_buf.insert(m_buf.end(), huffbuff.begin(), huffbuff.end());				
				return;
			}

			bool
			find(const header_t& h, int64_t& index)
			{
				int64_t saved_index(-1);
				index = -1;

				for ( uint64_t idx = 1; idx < predefined_headers.size(); idx++ ) {
					if ( !h.first.compare(predefined_headers.at(static_cast< std::size_t >( idx )).first) &&
						!h.second.compare(predefined_headers.at(static_cast< std::size_t >( idx )).second) ) {
						index = idx;
						return true;
					} else if ( !h.first.compare(predefined_headers.at(static_cast< std::size_t >( idx )).first) ) {
						index = idx;
					}
				}

				saved_index = index;

				if ( true == m_dynamic.find(h, index) )
					return true;
				else if ( -1 != index )
					return false;

				if ( -1 != saved_index )
					index = saved_index;

				return false;
			}

			uint64_t
			encode_integer(std::vector< uint8_t >& dst, uint32_t I, uint8_t N)
			{
				const uint16_t two_N = static_cast< uint16_t >( std::pow(2, N) - 1 );

				if ( I < two_N ) {
					dst.push_back(static_cast< uint8_t >( I ));
					return 1;
				} else {
					I -= two_N;
					dst.push_back(static_cast< uint8_t >( two_N ));

					while ( I >= 128 ) {
						dst.push_back(static_cast< uint8_t >( ( I & 0x7F ) | 0x80 ));
						I >>= 7;
					}

					dst.push_back(static_cast< uint8_t >( I ));
					return dst.size();
				}

				// unreached
				throw std::runtime_error("HPACK::encoder_t::encode_integer(): Impossible code path");
			}

		public:
			/*!
			\fn encoder_t(uint64_t max = 4096)
			\Brief Constructs the encoder

			\param max the maximum size of the dynamic table; unbounded and allowed to exceed RFC sizes
			*/
			encoder_t(uint64_t max = 4096) : m_dynamic(max) { }
			virtual ~encoder_t(void) { }

			/*!
			\fn void max_table_size(uint64_t max)
			\Brief Resizes the dynamic table

			\param max the maximum size of the dynamic table; unbounded and allowed to exceed RFC sizes
			*/
			void 
			max_table_size(uint64_t max) 
			{ 
				m_dynamic.max(max); 
				return; 
			}

			/*!
			\fn inline uint64_t max_table_size(void) const
			\Brief Retrieve the size of dynamic table

			\return max the maximum size of the dynamic table; unbounded and allowed to exceed RFC sizes
			*/
			inline uint64_t 
			max_table_size(void) const 
			{ 
				return m_dynamic.max(); 
			}

			/*!
			\fn void add(const std::string& n, const std::string& v, bool huffman = true, bool never_indexed = false)
			\Brief Add a header name-value pair to the header list
			
			\param n the name-value pair name member
			\param v the name-value pair value member
			\param huffman a boolean value that indicates whether to huffman encode any related string literals
			\param never_indexed whether to set the never indexed flag for the name-value pair
			*/
			void
			add(const std::string& n, const std::string& v, bool huffman = true, bool never_indexed = false)
			{
				header_t h(n, v);
				add(h, huffman, never_indexed);
			}

			/*!
			\fn void add(const char* n, const char* v, bool huffman = true, bool never_indexed = false)
			\Brief Add a header name-value pair to the header list
			\Throws std::invalid_argument() when n or v is null

			\param n the name-value pair name member
			\param v the name-value pair value member
			\param huffman a boolean value that indicates whether to huffman encode any related string literals
			\param never_indexed whether to set the never indexed flag for the name-value pair
			*/
			void
			add(const char* n, const char* v, bool huffman = true, bool never_indexed = false)
			{
				header_t h(n, v);

				if ( nullptr == n || nullptr == v )
					throw std::invalid_argument("HPACK::encoder_t::add(): Invalid nullptr parameter.");

				add(h, huffman, never_indexed);
			}

			/*!
			\fn void add(const header_t& h, bool huffman = true, bool never_indexed = false)
			\Brief Add a header name-value pair to the header block

			\param h the name-value header_t pair to be added to the header block
			\param huffman a boolean value that indicates whether to huffman encode any related string literals
			\param never_indexed whether to set the never indexed flag for the name-value pair
			*/
			void
			add(const header_t& h, bool huffman = true, bool never_indexed = false)
			{
				int64_t					index(0);
				std::vector< uint8_t >	buf(0), huffbuff(0);

				if ( false == never_indexed && true == find(h, index) ) {
					encode_integer(buf, static_cast< uint32_t >(index), 7);

					buf.front() |= INDEXED_BIT_PATTERN;
					m_buf.insert(m_buf.end(), buf.begin(), buf.end());
					buf.clear();
				} else if ( false == never_indexed && -1 != index ) {
					m_dynamic.add(h.first, h.second);

					if ( false == never_indexed ) {
						encode_integer(buf, static_cast< uint32_t >(index), 6);
						buf.front() |= LITERAL_INDEXED_BIT_PATTERN;
						m_buf.insert(m_buf.end(), buf.begin(), buf.end());
						buf.clear();
					} else {
						encode_integer(buf, static_cast< uint32_t >(index), 4);
						buf.front() |= LITERAL_NEVER_INDEXED_BIT_PATTERN;
						m_buf.insert(m_buf.end(), buf.begin(), buf.end());
						buf.clear();
					}

					if ( true == huffman )
						huff_encode(h.second);
					else {
						buf.clear();
						encode_integer(buf, (uint32_t)h.second.length(), 7);
						m_buf.insert(m_buf.end(), h.second.begin(), h.second.end());
					}
				} else {
					if ( false == never_indexed ) {
						m_dynamic.add(h.first, h.second);
						m_buf.push_back(LITERAL_INDEXED_BIT_PATTERN);
					} else
						m_buf.push_back(LITERAL_NEVER_INDEXED_BIT_PATTERN);

					if ( true == huffman && false == never_indexed )
						huff_encode(h.first);
					else {
						buf.clear();
						encode_integer(buf, (uint32_t)h.first.length(), 7);
						m_buf.insert(m_buf.end(), buf.begin(), buf.end());
						m_buf.insert(m_buf.end(), h.first.begin(), h.first.end());
					}
					if ( true == huffman && false == never_indexed )
						huff_encode(h.second);
					else {
						buf.clear();
						encode_integer(buf, (uint32_t)h.second.length(), 7);
						m_buf.insert(m_buf.end(), buf.begin(), buf.end());
						m_buf.insert(m_buf.end(), h.second.begin(), h.second.end());
					}	
				}

				return;
			}



			std::vector< uint8_t >& data(void) { return m_buf; }
		};
}
