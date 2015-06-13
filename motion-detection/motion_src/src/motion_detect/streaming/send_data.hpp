#include <vector>
#include <opencv2/core/types_c.h>

#ifndef SEND_DATA_HPP
#define	SEND_DATA_HPP

void closeSock();
int initRemote();
int remote(std::vector<uchar>& buff);

#endif	/* SEND_DATA_HPP */

