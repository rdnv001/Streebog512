/* ----------------------------------------------------------------------------------------------- */
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <libakrypt.h>

/* ----------------------------------------------------------------------------------------------- */
void function(ak_hmac ctx)
{
    ak_uint8 buffer[128];

    const auto password = "password";
    const auto seed = "seed";
    const auto data = "data";

    ak_hmac_set_key_from_password(ctx, static_cast<void*>(const_cast<char*>(password)), 8, static_cast<void*>(const_cast<char*>(seed)), 4);
    memset(buffer, 0, sizeof(buffer));

    ak_hmac_ptr(ctx, static_cast<void*>(const_cast<char*>(data)), 4, buffer, sizeof(buffer));

    printf("%s (%lu bytes: %s, %s)\n",
        ak_ptr_to_hexstr(buffer, ak_hmac_get_tag_size(ctx), ak_false),
        (unsigned long int)ak_hmac_get_tag_size(ctx), ctx->key.oid->name[0], ctx->key.oid->id[0]);
}

/* ----------------------------------------------------------------------------------------------- */
int main(void)
{
    ak_oid oid;
    hmac* ptr;

    /* проверяем создание/удаление контекстов с алгоритмами hmac */
    //function(ptr = ak_oid_new_object(oid = ak_oid_find_by_name("nmac-streebog")));
    //ak_oid_delete_object(oid, ptr);

    //function(ptr = ak_oid_new_object(oid = ak_oid_find_by_name("hmac-streebog256")));
    //ak_oid_delete_object(oid, ptr);

    function(ptr = ak_oid_new_object(oid = ak_oid_find_by_name("hmac-streebog512")));
    ak_oid_delete_object(oid, ptr);

    /* проверяем выработку производной ключевой информации с исопльзованием указанных алгоритмов */
    // ak_skey_derive_key_to_ptr

    return EXIT_SUCCESS;
}

/* ----------------------------------------------------------------------------------------------- */
/*                                                                                    test-hmac.c  */
/* ----------------------------------------------------------------------------------------------- */
