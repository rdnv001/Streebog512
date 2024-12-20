#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libakrypt.h>
#include <errno.h>
#include <openssl/rand.h>
#include <string.h> 

const char* function(ak_hmac ctx, const char* password, const char* seed, const char* data) {
    ak_uint8 buffer[128];

    size_t password_len = strlen(password);
    size_t seed_len = strlen(seed);
    size_t data_len = strlen(data);

    ak_hmac_set_key_from_password(ctx, static_cast<void*>(const_cast<char*>(password)), password_len, 
    static_cast<void*>(const_cast<char*>(seed)), seed_len);
    memset(buffer, 0, sizeof(buffer));

    ak_hmac_ptr(ctx, static_cast<void*>(const_cast<char*>(data)), data_len, buffer, sizeof(buffer));

    return ak_ptr_to_hexstr(buffer, ak_hmac_get_tag_size(ctx), ak_false);
}


// Модифицированная функция для вычисления HMAC
char* function_modified(ak_hmac ctx, const char* password, const char* seed, const char* data) {
    size_t password_len = strlen(password);
    size_t seed_len = strlen(seed);
    size_t data_len = strlen(data);

    printf( "data: %s\npassword: %s\nseed: %s\n", data, password, seed);

    ak_hmac_set_key_from_password(ctx, static_cast<void*>(const_cast<char*>(password)), password_len, 
    static_cast<void*>(const_cast<char*>(seed)), seed_len);

    size_t hmac_size = ak_hmac_get_tag_size(ctx);
    char* hmac_buffer = (char*)malloc(hmac_size * 2 + 1); // *2 для hex, +1 для \0
    if (hmac_buffer == NULL){
        perror("Ошибка выделения памяти");
        exit(1);
    }

    if(ak_hmac_ptr(ctx, (void*)data, data_len, (ak_uint8*)hmac_buffer, hmac_size) != ak_true){
        perror("Ошибка вычисления HMAC");
        exit(1);
    }

    ak_ptr_to_hexstr((ak_uint8*)hmac_buffer, hmac_size, ak_false);
    return hmac_buffer;
}
