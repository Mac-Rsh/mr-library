#include "mrboard.h"
#include "mrdef.h"

void mr_delay_us(mr_size_t us)
{
    Delay_Us(us);
}

void mr_delay_ms(mr_size_t ms)
{
    Delay_Ms(ms);
}