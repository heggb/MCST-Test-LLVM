#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

#define MAX_TESTS 100
#define MAX_PATH 1024

// Описание тестового случая (входной и ожидаемый выходной файлы)
typedef struct 
{
    char input_file[MAX_PATH];
    char output_file[MAX_PATH];
} TestCase;

// Результаты тестирования (количество пройденных/не пройденных тестов)
typedef struct 
{
    int passed;
    int failed;
} TestResults;

// Чтение строки из файла с динамическим выделением памяти
void read_line(FILE* file, char** line) 
{
    size_t len = 0;
    ssize_t read;
    if ((read = getline(line, &len, file)) == -1) 
    {
        *line = NULL;
        return;
    }
    if ((*line)[read - 1] == '\n')
        (*line)[read - 1] = '\0';
}

/*
 * Поиск тестовых файлов в директории.
 * Формат: для каждого файла .in должен существовать парный .out.
 * Возвращает количество найденных тестов.
 */
int find_tests(const char* test_dir, TestCase* tests) 
{
    DIR *dir;
    struct dirent *ent;
    int count = 0;
    
    if ((dir = opendir(test_dir)) != NULL) 
    {
        while ((ent = readdir(dir)) != NULL && count < MAX_TESTS) 
        {
            char* ext = strrchr(ent->d_name, '.');
            if (ext && !strcmp(ext, ".in")) 
            {
                snprintf(tests[count].input_file, MAX_PATH, 
                       "%s/%s", test_dir, ent->d_name);
                
                char out_name[MAX_PATH];
                strncpy(out_name, ent->d_name, ext - ent->d_name);
                out_name[ext - ent->d_name] = '\0';
                strcat(out_name, ".out");
                
                snprintf(tests[count].output_file, MAX_PATH,
                        "%s/%s", test_dir, out_name);
                count++;
            }
        }
        closedir(dir);
    }
    return count;
}

// Получение текущего времени с микросекундной точностью
double get_time() 
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

/*
 * Основная логика тестирования
 */

// Запуск одного тестового случая:
// 1. Перенаправление ввода/вывода
// 2. Замер времени выполнения
// 3. Сравнение с эталонным выводом
void run_test(const char* program, TestCase test, TestResults* results) 
{
    if (access(test.output_file, F_OK) == -1) 
    {
        printf("Output file missing: %s\n", test.output_file);
        results->failed++;
        return;
    }

    int input_fd = open(test.input_file, O_RDONLY);
    if (input_fd == -1) 
    {
        printf("Error opening input file: %s\n", test.input_file);
        results->failed++;
        return;
    }

    int output_fd = open("tmp_output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (output_fd == -1) 
    {
        printf("Error creating temporary output file\n");
        results->failed++;
        close(input_fd);
        return;
    }

    double start = get_time();
    pid_t pid = fork();
    
    if (pid == 0) 
    {
        dup2(input_fd, STDIN_FILENO);
        dup2(output_fd, STDOUT_FILENO);
        execl(program, program, NULL);
        exit(EXIT_FAILURE);
    }
    else if (pid > 0) 
    {
        int status;
        waitpid(pid, &status, 0);
        double end = get_time();
        double duration = end - start;

        close(input_fd);
        close(output_fd);

        FILE* expected = fopen(test.output_file, "r");
        FILE* actual = fopen("tmp_output.txt", "r");
        
        if (!expected || !actual) 
        {
            printf("Error opening output files\n");
            results->failed++;
            return;
        }

        char* expected_line = NULL;
        char* actual_line = NULL;
        int match = 1;

        while (1) 
        {
            read_line(expected, &expected_line);
            read_line(actual, &actual_line);

            if (!expected_line && !actual_line) break;
            
            if (!expected_line || !actual_line || 
                strcmp(expected_line ? expected_line : "", 
                       actual_line ? actual_line : "") != 0) 
            {
                match = 0;
                break;
            }
        }

        if (match) 
        {
            results->passed++;
            printf("\033[32m[PASS]\033[0m %s (%.3fs)\n", test.input_file, duration);
        } 
        else 
        {
            results->failed++;
            printf("\033[31m[FAIL]\033[0m %s (%.3fs)\n", test.input_file, duration);
        }

        fclose(expected);
        fclose(actual);
        remove("tmp_output.txt");
    }
    else 
    {
        printf("Fork failed\n");
        results->failed++;
    }
}

int main(int argc, char* argv[]) 
{
    if (argc != 3) 
    {
        printf("Usage: %s <program> <test_dir>\n", argv[0]);
        return EXIT_FAILURE;
    }

    TestCase tests[MAX_TESTS];
    int num_tests = find_tests(argv[2], tests);
    
    if (num_tests == 0) 
    {
        printf("No tests found\n");
        return EXIT_FAILURE;
    }

    TestResults results = {0};
    printf("Running %d tests...\n\n", num_tests);

    for (int i = 0; i < num_tests; i++) 
        run_test(argv[1], tests[i], &results);

    printf("\nResults:\n");
    printf("Passed: \033[32m%d\033[0m\n", results.passed);
    printf("Failed: \033[31m%d\033[0m\n", results.failed);

    return results.failed ? EXIT_FAILURE : EXIT_SUCCESS;
}