#include <stdint.h>
#include <stddef.h>
#include <unistd.h>
#include <stdlib.h>
#include "pet_decode.h"

bool fuzz(const uint8_t *data, size_t size)
{
    uint32_t payload_len_out = 0;
    struct Pet result;
    bool ret = cbor_decode_Pet(data, size,
                               &result,
                               &payload_len_out);
    return ret;
}

#ifdef LIBFUZZER
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    fuzz(data, size);
    return 0;
}
#else
int main(void)
{
    uint8_t buffer[512];
    ssize_t size = read(0, buffer, sizeof(buffer));
    if(size <= 0)
    {
        return 0;
    }
    uint8_t *cpy = malloc(size);
    if (cpy == NULL)
    {
        return 0;
    }
    memcpy(cpy, buffer, size);
    bool ret = fuzz(cpy, size);
    free(cpy);
    return (ret) ? 0 : -1;
}
#endif