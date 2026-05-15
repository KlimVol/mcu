#include "stdio-task.h"
#include "stdio.h"
#include "pico/stdlib.h"


#define COMMAND_BUF_LEN 128

char command[COMMAND_BUF_LEN] = { 0 };

int command_buf_idx;
void stdio_task_init()
{
	command_buf_idx = 0;
}
char* stdio_task_handle()
{
    int symbol = getchar_timeout_us(0);

    // Если данных нет, сразу выходим
    if (symbol == PICO_ERROR_TIMEOUT)
    {
        return NULL;
    }

    // 1. Защита от пустых вводов и "хвостов" от предыдущих команд
    // Если пришел символ переноса строки, а в буфере еще ничего нет — игнорируем
    if ((symbol == '\r' || symbol == '\n') && command_buf_idx == 0)
    {
        return NULL;
    }

    // 2. Эхо: возвращаем символ обратно в терминал, чтобы пользователь видел, что пишет
    putchar(symbol);

    // 3. Если нажали Enter (\r или \n)
    if (symbol == '\r' || symbol == '\n')
    {
        command[command_buf_idx] = '\0'; // Завершаем строку
        command_buf_idx = 0;             // Сбрасываем индекс для следующего раза

        // Переходим на новую строку в терминале для красоты
        printf("\n");

        return command;                  // Возвращаем готовую команду на обработку
    }

    // 4. Если нажали Backspace (код 8 или 127) — опционально, для удобства правки
    if (symbol == 8 || symbol == 127) {
        if (command_buf_idx > 0) {
            command_buf_idx--;
            // Удаляем символ из терминала (курсор назад -> пробел -> курсор назад)
            printf("\b \b");
        }
        return NULL;
    }

    // 5. Запись символа в буфер с защитой от переполнения
    if (command_buf_idx < COMMAND_BUF_LEN - 1)
    {
        command[command_buf_idx] = (char)symbol;
        command_buf_idx++;
    }
    else
    {
        // Если буфер переполнен, сбрасываем его, чтобы не было ошибок
        command_buf_idx = 0;
        printf("\nError: Command too long\n");
    }

    return NULL;
}
