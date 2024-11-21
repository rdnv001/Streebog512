#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <iostream>
#include <vector>
#include <iomanip>
#include <cstring>

using namespace std;

// Константы
const unsigned char C1[] = {0x01, 0x02, 0x03, 0x04, 0x05};
const unsigned char C2[] = {0x0A, 0x0B, 0x0C, 0x0D, 0x0E}; 
const unsigned char DATA[] = "Пример данных для обработки";


// Утилита для печати ключей в hex-формате
void printHex(const char* label, const unsigned char* data, size_t len) {
    cout << label << ": ";
    for (size_t i = 0; i < len; i++) {
        cout << hex << setw(2) << setfill('0') << (int)data[i];
    }
    cout << dec << endl;
}

// Функция для вычисления HMAC
vector<unsigned char> hmac_streebog512(const unsigned char* key, size_t key_len, const unsigned char* data, size_t data_len) {
    // Выбираем GOST алгоритм
    const EVP_MD* md = EVP_get_digestbyname("streebog512");
    if (!md) {
        cerr << "Ошибка: Не удалось загрузить streebog512" << endl;
        exit(1);
    }

    unsigned char result[EVP_MAX_MD_SIZE];
    unsigned int result_len = 0;

    HMAC_CTX* ctx = HMAC_CTX_new();
    if (!ctx) {
        cerr << "Ошибка: Не удалось создать контекст HMAC" << endl;
        exit(1);
    }

    // Инициализация HMAC
    if (HMAC_Init_ex(ctx, key, key_len, md, nullptr) != 1 ||
        HMAC_Update(ctx, data, data_len) != 1 ||
        HMAC_Final(ctx, result, &result_len) != 1) {
        cerr << "Ошибка: HMAC не выполнен" << endl;
        HMAC_CTX_free(ctx);
        exit(1);
    }

    HMAC_CTX_free(ctx);
    return vector<unsigned char>(result, result + result_len);
}

int main() {
    // Исходный ключ
    const unsigned char K[] = "MASTER_KEY";
    size_t K_len = strlen((const char*)K);

    // Промежуточные хэши
    vector<unsigned char> hash1 = hmac_streebog512(K, K_len, C1, sizeof(C1) - 1);
    vector<unsigned char> hash2 = hmac_streebog512(K, K_len, C2, sizeof(C2) - 1);

    // Итоговый хэш
    hash1.insert(hash1.end(), hash2.begin(), hash2.end()); // K+C1 || K+C2
    vector<unsigned char> finalHash = hmac_streebog512(hash1.data(), hash1.size(), DATA, sizeof(DATA) - 1);

    // Разделение на K1 и K2
    size_t half = finalHash.size() / 2;
    vector<unsigned char> K1(finalHash.begin(), finalHash.begin() + half);
    vector<unsigned char> K2(finalHash.begin() + half, finalHash.end());

    // Вывод результатов
    printHex("K1", K1.data(), K1.size());
    printHex("K2", K2.data(), K2.size());

    return 0;
}
