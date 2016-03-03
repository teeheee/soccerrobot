/*
 * wifistream.h
 *
 *  Created on: 04.08.2015
 *      Author: alexander
 */


#ifndef WIFISTREAM_H_
#define WIFISTREAM_H_

#include "jpeg.h"
#include "wifi.h"



#define BUFFER_SIZE 1400

class output_stream
{
public:
	int buf_ofs;
	void init_buffer();
	output_stream(const output_stream &);
	output_stream &operator= (const output_stream &);
   output_stream();
   virtual ~output_stream();
   bool put_buf(const void* pBuf, int len);
   uint get_size() const;
   void reset_size();
   template<class T> inline bool put_obj(const T& obj) {
         return put_buf(&obj, sizeof(T));
       }
};

#endif /* WIFISTREAM_H_ */
