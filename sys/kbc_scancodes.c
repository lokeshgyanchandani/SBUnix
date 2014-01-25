#include <defs.h>
#include <sys/kbc_scancodes.h>

static uint8_t char_shift_adjust(uint8_t c, uint8_t shift)
{
    return c + (shift ? ('A'-'a') : 0);
}

uint8_t kbcsc_tochar(uint8_t b, uint8_t shift)
{
    if(b==KBCSC_SPACE)
        return ' ';

    if(b==KBCSC_RETURN)
	 return '\n';

    if((b >= KBCSC_Q) && (b <= KBCSC_P))
	 return char_shift_adjust("qwertyuiop"[b-KBCSC_Q], shift);

    if((b >= KBCSC_A) && (b <= KBCSC_L))
	 return char_shift_adjust("asdfghjkl"[b-KBCSC_A], shift);

    if((b >= KBCSC_Z) && (b <= KBCSC_M))
	 return char_shift_adjust("zxcvbnm"[b-KBCSC_Z], shift);

    if((b >= KBCSC_1) && (b <= KBCSC_9)) {
	 if(shift)
	     return "!@#$%^&*("[b-KBCSC_1];
	 return '1' + b-KBCSC_1;
    }

    if((b >= KBCSC_0) && (b <= KBCSC_BACKSPACE)) {
	 if(shift)
	     return ")_+"[b-KBCSC_0];
	 return "0-="[b-KBCSC_0];
    }

    if((b >= KBCSC_COLON) && (b <= KBCSC_TILDA)) {
	 if(shift)
	     return ":\"~"[b-KBCSC_COLON];
	 return ";''"[b-KBCSC_COLON];
    }

    if((b >= KBCSC_OPEN_CURL) && (b <= KBCSC_CLOSE_CURL)) {
        if(shift)
	     return "{}"[b-KBCSC_OPEN_CURL];
	 return "[]"[b-KBCSC_OPEN_CURL];
    }

    if((b >= KBCSC_LT) && (b <= KBCSC_FORWARDSLASH)) {
	 if(shift)
	     return "<>/"[b-KBCSC_LT];
	 return ",./"[b-KBCSC_LT];
    }

    if(b == KBCSC_BACKSLASH){
        if(shift)
	     return '|' ;
	 return  '\\';	}
    return 0;
}

