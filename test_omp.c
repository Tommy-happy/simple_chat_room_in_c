#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <omp.h>
#define BUFFER_SIZE 10

int main(){
    omp_set_num_threads(5);
    #pragma omp parallel sections
    {
        #pragma omp section{
            printf("in section 1\n");
        }
        #pragma omp section
        {
            printf("in section 2\n");
        }
        // printf("out of section\n");
    }
    
    // printf("hello,openmp\n");
}