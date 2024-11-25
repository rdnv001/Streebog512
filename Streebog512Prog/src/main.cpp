#include <stdio.h>
#include <stdint.h>
#include <string>
#include <vector>
#include <libakrypt.h>

int main(void)
{
    if (ak_libakrypt_create(NULL) != ak_true) 
    {
        /* инициализация выполнена не успешно, следовательно, выходим из программы */
        ak_libakrypt_destroy();
        return EXIT_FAILURE;
    }

    /* ... здесь основной код программы ... */

    // -

    const std::string C1 = "Constant_C1"; // Константа C1
    const std::string C2 = "Constant_C2"; // Константа C2

    std::vector<uint8_t> HMAC_Streebog512(const std::vector<uint8_t>&key, const std::vector<uint8_t>&data) 
    {

        ak_handle hmac_handle;
        ak_buffer result = NULL;
        std::vector<uint8_t> hash_output(64);  // Выходное значение для Стрибог-512 (512 бит = 64 байта)

        // Создание контекста для HMAC со Стрибог-512
        if (ak_hmac_create_streebog512(&hmac_handle) != ak_error_ok) 
        {
            printf("Ошибка создания HMAC контекста\n");
            return {};
        }

        // Входные данные для первого уровня (K + C2 || DATA)
        std::vector<uint8_t> first_input(key);
        first_input.insert(first_input.end(), C2.begin(), C2.end());
        first_input.insert(first_input.end(), data.begin(), data.end());

        // Первый уровень хеширования
        if (ak_hmac_context_ptr(&hmac_handle, first_input.data(), first_input.size(), &result) != ak_error_ok) 
        {
            printf("Ошибка вычисления HMAC для первого уровня\n");
            ak_hmac_destroy(&hmac_handle);
            return {};
        }

        // Результат первого уровня
        std::vector<uint8_t> inner_hash(ak_buffer_get_size(&result));
        memcpy(inner_hash.data(), ak_buffer_get_ptr(&result), inner_hash.size());
        ak_buffer_free(&result);

        // Входные данные для второго уровня (K + C1 || inner_hash)
        std::vector<uint8_t> second_input(key);
        second_input.insert(second_input.end(), C1.begin(), C1.end());
        second_input.insert(second_input.end(), inner_hash.begin(), inner_hash.end());

        // Второй уровень хеширования
        if (ak_hmac_context_ptr(&hmac_handle, second_input.data(), second_input.size(), &result) != ak_error_ok) 
        {
            printf("Ошибка вычисления HMAC для второго уровня\n");
            ak_hmac_destroy(&hmac_handle);
            return {};
        }

        // Результат второго уровня
        memcpy(hash_output.data(), ak_buffer_get_ptr(&result), hash_output.size());
        ak_buffer_free(&result);
        ak_hmac_destroy(&hmac_handle);

        return hash_output;
    }

    int main() 
    {
        // Пример ключа и данных
        std::string key = "ExampleKey";
        std::string data = "ExampleData";

        // Вычисление HMAC
        std::vector<uint8_t> result = HMAC_Streebog512(
            std::vector<uint8_t>(key.begin(), key.end()),
            std::vector<uint8_t>(data.begin(), data.end())
        );

        if (!result.empty()) {
            printf("HMAC Стрибог512 успешно вычислен: ");
            for (uint8_t byte : result) {
                printf("%02X", byte);
            }
            printf("\n");
        }
        else {
            printf("Ошибка вычисления HMAC\n");
        }

        return 0;
    }

    // -

    ak_libakrypt_destroy();
    return EXIT_SUCCESS;
}