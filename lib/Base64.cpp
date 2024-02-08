#include "Pcheader.h"

#include <tpublic/Base64.h>
#include <tpublic/DataErrorHandling.h>

namespace tpublic
{

	namespace Base64
	{

		namespace
		{
			// Base64 encoding/decoding (RFC1341)
			// Copyright (c) 2005-2011, Jouni Malinen <j@w1.fi>

			static const unsigned char base64_table[65] =
				"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

			unsigned char*
			_Base64Encode(
				const unsigned char*	src, 
				size_t					len,
				size_t*					out_len)
			{
				unsigned char *out, *pos;
				const unsigned char *end, *in;
				size_t olen;

				olen = len * 4 / 3 + 4; /* 3-byte blocks to 4-byte */
				olen++; /* nul termination */
				if (olen < len)
					return NULL; /* integer overflow */
				out = new unsigned char[olen];
				if (out == NULL)
					return NULL;

				end = src + len;
				in = src;
				pos = out;
				while (end - in >= 3) {
					*pos++ = base64_table[in[0] >> 2];
					*pos++ = base64_table[((in[0] & 0x03) << 4) | (in[1] >> 4)];
					*pos++ = base64_table[((in[1] & 0x0f) << 2) | (in[2] >> 6)];
					*pos++ = base64_table[in[2] & 0x3f];
					in += 3;
				}

				if (end - in) {
					*pos++ = base64_table[in[0] >> 2];
					if (end - in == 1) {
						*pos++ = base64_table[(in[0] & 0x03) << 4];
						*pos++ = '=';
					} else {
						*pos++ = base64_table[((in[0] & 0x03) << 4) |
							(in[1] >> 4)];
						*pos++ = base64_table[(in[1] & 0x0f) << 2];
					}
					*pos++ = '=';
				}

				*pos = '\0';
				if (out_len)
					*out_len = pos - out;
				return out;
			}

			unsigned char* 
			_Base64Decode(
				const unsigned char*	src, 
				size_t					len,
				size_t*					out_len)
			{
				unsigned char dtable[256], *out, *pos, block[4], tmp;
				size_t i, count, olen;
				int pad = 0;

				memset(dtable, 0x80, 256);
				for (i = 0; i < sizeof(base64_table) - 1; i++)
					dtable[base64_table[i]] = (unsigned char) i;
				dtable[(uint8_t)'='] = 0;

				count = 0;
				for (i = 0; i < len; i++) {
					if (dtable[src[i]] != 0x80)
						count++;
				}

				if (count == 0 || count % 4)
					return NULL;

				olen = count / 4 * 3;
				pos = out = new unsigned char[olen];
				if (out == NULL)
					return NULL;

				count = 0;
				for (i = 0; i < len; i++) {
					tmp = dtable[src[i]];
					if (tmp == 0x80)
						continue;

					if (src[i] == '=')
						pad++;
					block[count] = tmp;
					count++;
					if (count == 4) {
						*pos++ = (block[0] << 2) | (block[1] >> 4);
						*pos++ = (block[1] << 4) | (block[2] >> 2);
						*pos++ = (block[2] << 6) | block[3];
						count = 0;
						if (pad) {
							if (pad == 1)
								pos--;
							else if (pad == 2)
								pos -= 2;
							else {
								/* Invalid padding */
								delete [] out;
								return NULL;
							}
							break;
						}
					}
				}

				*out_len = pos - out;
				return out;
			}
		}

		void	
		Encode(
			const void*					aIn,
			size_t						aInSize,
			std::string&				aOut)
		{
			size_t outLen = 0;
			unsigned char* p = _Base64Encode((const unsigned char*)aIn, (size_t)aInSize, &outLen);
			TP_CHECK(p != NULL, "_Base64Encode() failed");
			aOut = (char*)p;
			delete [] p;
		}

		void	
		Decode(
			const char*					aIn,
			std::vector<uint8_t>&		aOut)
		{
			size_t outLen = 0;
			unsigned char* p = _Base64Decode((const unsigned char*)aIn, (size_t)strlen(aIn), &outLen);
			TP_CHECK(p != NULL, "_Base64Decode() failed");
			aOut.resize(outLen);
			memcpy(&aOut[0], p, outLen);
			delete [] p;
		}

	}


}