#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libakrypt.h>
#include <errno.h>
#include <openssl/rand.h>
#include <string.h> 

unsigned long long generate_random_number(unsigned long long min, unsigned long long max = ULLONG_MAX) {
    unsigned long long random_number;

    unsigned char buffer[sizeof(unsigned long long)];
    if (!RAND_bytes(buffer, sizeof(buffer))) {
        perror("RAND_bytes failed");
        exit(1);
    }

    // Преобразуем байты из buffer в unsigned long long
    memcpy(&random_number, buffer, sizeof(unsigned long long));

    // Масштабируем к нужному диапазону
    random_number %= max - min + 1;
    return min + random_number;
}

// функция для генерации случайных строк заданной длины
char* generate_random_string(size_t length) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    char* str = (char*)malloc(length + 1);
    if (str == NULL) {
        perror("malloc failed");
        exit(1);
    }
    for (size_t i = 0; i < length; i++) {
        str[i] = charset[generate_random_number(0, sizeof(charset) - 1)];
    }
    str[length] = '\0';
    return str;
}


// Функция генерации файлов с тестовыми данными
void generate_test_files(size_t num_strings) {
    unsigned counter = 0;
    char filename[100];

    while (1) {
        sprintf(filename, "sample_passwords_%zu_%u.txt", num_strings, counter);
        FILE *f = fopen(filename, "r");
        if (f == NULL && errno == ENOENT) {
            break; // Файл не существует - создадим его
        }
        fclose(f);
        counter++;
    }

    char passwords_filename[100];
    char seeds_filename[100];
    char data_filename[100];

    sprintf(passwords_filename, "sample_passwords_%zu_%u.txt", num_strings, counter);
    sprintf(seeds_filename, "sample_seed_%zu_%u.txt", num_strings, counter);
    sprintf(data_filename, "sample_data_%zu_%u.txt", num_strings, counter);

    FILE *passwords;
    FILE *seeds;
    FILE *data;
    
    passwords = fopen(passwords_filename, "w");
    seeds = fopen(seeds_filename, "w");
    data = fopen(data_filename, "w");

    
    if (passwords == NULL || seeds == NULL || data == NULL) {
        perror("Ошибка открытия файла");
        exit(1);
    }

    for (size_t i = 0; i < num_strings; ++i) {
        fprintf(passwords, "%s\n", generate_random_string(generate_random_number(8, 40))); //Пароль от 8 до 40 символов
        fprintf(seeds, "%s\n", generate_random_string(generate_random_number(4, 20))); // Seed от 4 до 20 символов
        fprintf(data, "%s\n", generate_random_string(generate_random_number(64, 512))); //Данные от 64 до 512 символов
    }

    fclose(passwords);
    fclose(seeds);
    fclose(data);
}

// Модифицированная функция для вычисления HMAC
char* function(ak_hmac ctx, const char* password, const char* seed, const char* data) {
    size_t password_len = strlen(password);
    size_t seed_len = strlen(seed);
    size_t data_len = strlen(data);

    if (ak_hmac_set_key_from_password(ctx, (void*)password, password_len, (void*)seed, seed_len) != ak_true) {
          perror("Ошибка установки ключа");
          exit(1);
    }

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


// Функция для обработки тестовых файлов и записи результатов
void process_test_files(size_t num_strings, unsigned file_counter) {
  char filename[100];
  FILE *passwords_file, *seeds_file, *data_file, *output_file;
  ak_oid oid = ak_oid_find_by_name("hmac-streebog512");
  ak_pointer ptr = ak_oid_new_object(oid);
  ak_hmac ctx = (ak_hmac) ptr;

  sprintf(filename, "./sample_passwords_%zu_%u.txt", num_strings, file_counter);
  passwords_file = fopen(filename, "r");
  sprintf(filename, "./sample_seed_%zu_%u.txt", num_strings, file_counter);
  seeds_file = fopen(filename, "r");
  sprintf(filename, "./sample_data_%zu_%u.txt", num_strings, file_counter);
  data_file = fopen(filename, "r");
  sprintf(filename, "test_results_%zu_%u.txt", num_strings, file_counter);
  output_file = fopen(filename, "w");

  if (passwords_file == NULL || seeds_file == NULL || data_file == NULL || output_file == NULL) {
      perror("Ошибка открытия файла");
      exit(1);
  }


  char password[1024], seed[1024], data_line[1024];

  for (size_t i = 0; i < num_strings; ++i)
    if (fgets(password, sizeof(password), passwords_file) == NULL){
        perror("Ошибка чтения из файла");
        exit(1);
        }
        for (size_t j = 0; j < num_strings; ++j)
            if(fgets(seed, sizeof(seed), seeds_file) == NULL){
                    perror("Ошибка чтения из файла");
                    exit(1);
                    }
                    for (size_t k = 0; k < num_strings; ++k){
                        if (fgets(data_line, sizeof(data_line), data_file) == NULL) {
                            perror("Ошибка чтения из файла");
                            exit(1);
                        }
                        password[strcspn(password, "\n")] = 0; //Удаление символа новой строки
                        seed[strcspn(seed, "\n")] = 0;
                        data_line[strcspn(data_line, "\n")] = 0;


                        char* hmac_result = function(ctx, password, seed, data_line);
                        fprintf(output_file, "password: %s\nseed: %s\ndata: %s\nhmac: %s\n\n", 
                        password, seed, data_line, hmac_result);
                        free(hmac_result);
                    }

  fclose(passwords_file);
  fclose(seeds_file);
  fclose(data_file);
  fclose(output_file);
  ak_oid_delete_object(oid, ptr);
}


int main() {

    //Генерация тестовых файлов
    generate_test_files(100); //Генерируем файлы с 100 строками
    process_test_files(100, 0); //Обрабатываем сгенерированные файлы

    return 0;
}