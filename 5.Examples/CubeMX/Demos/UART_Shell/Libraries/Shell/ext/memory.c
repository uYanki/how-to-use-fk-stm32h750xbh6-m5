#include "./memory.h"

// hexdump

// int aos_log_hexdump(const char* tag, char* buffer, int len)
// {
//     int i;

//     if (aos_ulog_init == false)
//     {
//         return -1;
//     }

//     aos_cust_output_func("[%s]\n", tag);
//     aos_cust_output_func("0x0000: ");
//     for (i = 0; i < len; i++)
//     {
//         aos_cust_output_func("0x%02x ", buffer[i]);

//         if (i % 8 == 7)
//         {
//             aos_cust_output_func("\n");
//             aos_cust_output_func("0x%04x: ", i + 1);
//         }
//     }

//     aos_cust_output_func("\n");
//     return 0;
// }