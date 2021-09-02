#include "prefix_sum.h"
#include "helpers.h"
#include<cmath>

extern pthread_barrier_t barrier;

void* compute_prefix_sum(void *a)
{   
    prefix_sum_args_t *args = (prefix_sum_args_t *)a;
    
    int number_vals; int max_val; int min_val;
    if ((args->t_id == args->n_threads - 1) && ((args->n_vals % args->n_threads) != 0)){
        min_val = (args->n_vals / args->n_threads) * (args->n_threads - 1);
        max_val = args->n_vals;
        number_vals = max_val - min_val;    
        
    } else {
        
        number_vals = (args->n_vals / args->n_threads);
        max_val = (number_vals * (args->t_id + 1));
        min_val = max_val - number_vals;

    }

    int test_pow_2 = int(log2(number_vals));
    int recent_pow_2 = int(pow(2, test_pow_2));
    int dist = number_vals - recent_pow_2;

    int max_pow_2 = min_val + recent_pow_2;

    int* x = (int*)malloc(dist*sizeof(int));
    if (dist > 0){
        for (int j = 0; j<dist; j++){
            x[j] = args->input_vals[max_pow_2 + j];
        }
    }
    int y = args->input_vals[max_pow_2-1];

    for (int d = 0; d < log2(recent_pow_2); d++){
        for (int k = min_val; k < max_pow_2; k = k + int((pow(2, d+1)))){
            args->input_vals[k + int(pow(2, d +1)) - 1 ] = args->op(args->input_vals[k + int(pow(2, d +1)) - 1 ], args->input_vals[k + int(pow(2, d)) - 1], args->n_loops);
        }
    }

    pthread_barrier_wait(&barrier);
    
    args->input_vals[max_pow_2-1] = 0; 
    for (int d = int(log2(recent_pow_2) - 1); d >=0; d--){
        for (int k = min_val; k < max_pow_2; k = k + int((pow(2, d+1)))){
            int t = args->input_vals[k + int(pow(2, d)) - 1];
            args->input_vals[k + int(pow(2, d)) - 1] = args->input_vals[k + int(pow(2, d+1)) - 1];
            args->input_vals[k + int(pow(2, d+1)) - 1] = args->op(t, args->input_vals[k + int(pow(2, d+1)) - 1], args->n_loops);
        }
    }
    

    int val = 0;
    for (int idx = 0; idx < args->n_threads; idx++){
        if (idx == args->t_id){
            if (idx != 0){
               val = args->output_vals[min_val - 1];
            }
            for (int j = 0; j<recent_pow_2; j++){
                args->output_vals[min_val + j] = args->input_vals[min_val + j + 1] + val;
            }
            args->output_vals[max_pow_2-1] = args->output_vals[max_pow_2-2] + y;
            int fin = max_pow_2;
            int cur = 0;
            while (dist > 0){
                fin += 1;
                dist -= 1;
                args->output_vals[fin-1] = args->output_vals[fin - 2] + x[cur];
                cur += 1;
            }
            free(x);
        }

        pthread_barrier_wait(&barrier);
    }
    
    return 0;
}
