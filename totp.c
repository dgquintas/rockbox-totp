#include <string.h>
#include <stdint.h>

#include "hmac-sha1.h"
#include "base32.h"

#include "plugin.h"

void change_endianness64(uint64_t* pi){
    register uint64_t i = *pi;
    // swap 32 bit words
    i = (i << 32) | (i >> 32);
    // swap 16 bit words
    uint64_t m = 0xffff0000ffff0000;
    i = ((i & m) >> 16) | ((i & ~m) << 16);
    // swap bytes
    m = 0xff00ff00ff00ff00;
    i = ((i & m) >> 8) | ((i & ~m) << 8);
    *pi = i;
}


uint32_t HOTP(const uint8_t* key, int keyLength, uint64_t counter){
    // we need to transform C into a 8-byte array
    // we naturally need to determine the endianness of the system
#ifdef ROCKBOX_LITTLE_ENDIAN
    change_endianness64(&counter);
#endif
    uint8_t HS[20];
    hmac_sha1(key, keyLength, (const uint8_t*)&counter, 8, HS);
    int offset = HS[19] & 0xf;

    // we now want the 31 lsb's of the 32 bit dword starting at byte "offset"
    // of HS
    uint8_t* offsetHS = HS + offset;
    uint32_t p;
#ifdef ROCKBOX_BIG_ENDIAN
    memcpy(&p, offsetHS, 4);
#else
    p = *(offsetHS+3);
    p |= *(offsetHS+2) << 8;
    p |= *(offsetHS+1) << 16;
    p |= *(offsetHS)   << 24;
#endif

    p &= 0x7fffffff;
    // now get only the least significant 6 decimal digits
    p %= 1000000;

    return p;
}

enum plugin_status plugin_start(const void* parameter){
    char keyb32[17];
    keyb32[0] = '\0';
    //const char* keyb32 = "wa2oatxr5y65iugt";
	char* fname = (char*)parameter;
    if(fname && *fname){
		int fd = rb->open(parameter, O_RDONLY);
		const int rsize = rb->read(fd, keyb32, 16);
        rb->close(fd);
        rb->splashf(1.5*HZ, "Read %d bytes", rsize);
    	keyb32[16] = '\0';
	}
	else{ 
		if(rb->kbd_input(keyb32, sizeof(keyb32)) < 0){
        	return PLUGIN_OK; // cancelled
    	}
	}
    uint8_t key[20];
    base32_decode((const uint8_t*)keyb32, key, 20);
//    char key_str[41];
//    int i;
//    for(i=0; i < 20; i++){
//        rb->snprintf(key_str+i*2, 3, "%x", key[i]);
//    }
//    key_str[41] = '\0';
        
    const int X = 30;
    int button, quit = 0;
	time_t t;
	struct tm* tt;
	int c;
	int rem_secs;
    uint32_t h = 0;
    while(!quit){
        button = rb->button_get(false); // dont block
		if(button == BUTTON_POWER){
			quit=true;
		}
		else{ 
			rb->lcd_clear_display();
			tt = rb->get_time();
		 	t = rb->mktime(tt);
    		c = t / X;
			rem_secs = t % X;
			if(rem_secs == 0 ){ 
				rem_secs = X;
			}
            if( !h /* 1st iter */ || rem_secs >= 25 /* X-cycle change */ ){
                h = HOTP(key, rb->strlen(key), c);
            }

			rb->lcd_putsf(0,0,"%d", t);
			rb->lcd_putsf(0,1,"%d/%02d/%02d %02d:%02d:%02d GMT", 
					1900+tt->tm_year, tt->tm_mon+1, tt->tm_mday, 
					tt->tm_hour, tt->tm_min, tt->tm_sec);
//			rb->lcd_putsf(0,1,"%s", key_str);
			//rb->lcd_putsf(0,2,"%d", c);
			rb->lcd_putsf(0,3,"%d secs left", X-rem_secs);
			rb->lcd_putsf(0,4,"%06d", h);

			//rb->lcd_putsf(0,4,"%s", keyb32);
			rb->lcd_update();
			rb->sleep(1*HZ);
		}
    }
    return PLUGIN_OK;
}

