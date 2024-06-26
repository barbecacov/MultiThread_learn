// my_gpu_add.cu
#include <iostream>
#include <cuda_runtime.h>
#include <cstdlib>

// 核函数
__global__
void vectorAddKernel(float * A_d, float * B_d, float * C_d, int n){
    int i = threadIdx.x + blockDim.x * blockIdx.x;
    if(i <= n) C_d[i] = A_d[i] + B_d[i];
}


int main(int argc, char * argv[]){
    int n = atoi(argv[1]);
    std::cout << "执行加法" << n << "次" << std::endl;
    
    // 要申请的内存大小
    size_t size = n * sizeof(float);

    // 先在host上申请内存，生成数据
    // 在CPU上使用 free - malloc 申请内存方式
    float *a = (float*)malloc(size);
    float *b = (float*)malloc(size);
    float *c = (float*)malloc(size);

    // 生成随机数组
    for(int i = 0; i < n; i++){
        float af = rand() / double(RAND_MAX);
        float bf = rand() / double(RAND_MAX);
        a[i] = af;
        b[i] = bf;
    }

    // 定义device指针
    float * da = NULL;
    float * db = NULL;
    float * dc = NULL;

    // 显存申请
    cudaMalloc((void **) &da, size);
    cudaMalloc((void **) &db, size);
    cudaMalloc((void **) &dc, size);

    // 转移拷贝 host --> device
    cudaMemcpy(da, a, size, cudaMemcpyHostToDevice);
    cudaMemcpy(db, b, size, cudaMemcpyHostToDevice);
    cudaMemcpy(dc, c, size, cudaMemcpyHostToDevice);

    // 调用核函数
    vectorAddKernel<<< n/256+1, 256>>> (da, db, dc, n);
    std::cout<<"nblockPerGrid:"<<n/256 +1 <<"\n";

    // 转移拷贝 device --> host
    cudaMemcpy(c, dc, size, cudaMemcpyDeviceToHost);

    // 释放显存
    cudaFree(da);
    cudaFree(db);
    cudaFree(dc);

    
    // std::cout<<"a:"<<*a<<"\n";
    // std::cout<<"b:"<<*b<<"\n";
    // std::cout<<"c:"<<*c<<"\n";
    /*
    $ ./my_gpu_add 1
    a:0.840188
    b:0.394383
    c:1.23457
    通过这个输出可以看出：计算结果是正确的
    */

    // 释放空间
    free(a);
    free(b);
    free(c);
    return 0;
}

