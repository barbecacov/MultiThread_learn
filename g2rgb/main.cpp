#include <cstring>
#include "arm_neon.h"
#include <iostream>
using namespace std;

static long long get_microseconds(void){
    // 微秒单位
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return now.tv_sec * 1000000 + now.tv_nsec / 1000;
}



void normal_process(unsigned char* input_data, unsigned char* output_data)
{
    memset(output_data, 0, 640*512*3);
    for (int j = 0; j < 512; ++j) {
        for (int i = 0; i < 640; ++i) {
            output_data[3*(i+j* 640)] = input_data[i + j * 640];
            output_data[3*(i+j* 640) + 1] = input_data[i + j * 640];
            output_data[3*(i+j* 640) + 2] = input_data[i + j * 640];
        }
    }

    cout << "index 199 : " << *(output_data+199) << endl;
    cout << "index 10000 : " << *(output_data+10000) << endl;
}


void memset_process(unsigned char* input_data, unsigned char* output_data)
{
    memset(output_data, 0, 640*512*3);
    for (int i = 0; i < 640*512; ++i) {
        memset(output_data+i*3, *(input_data+i), 3);
    }
    cout << "index 199 : " << *(output_data+199) << endl;
    cout << "index 10000 : " << *(output_data+10000) << endl;
}


void neon_process(unsigned char* input_data, unsigned char* output_data)
{
    memset(output_data, 0, 640*512*3);
    uint8x8_t v_input;
    uint8x8x3_t v_output;
    for (int i = 0; i < 640 * 512; i += 8) {
        // Load 8 sets of 3 bytes with explicit alignment

        v_input = vld1_u8(input_data + i);
        v_output.val[0] = v_input;
        v_output.val[1] = v_input;
        v_output.val[2] = v_input;
        vst3_u8(output_data + i * 3, v_output);  // Store 8 sets of 3 bytes
    }
    cout << "index 199 : " << *(output_data+199) << endl;
    cout << "index 10000 : " << *(output_data+10000) << endl;
}


int main(){

    int width = 640;
    int height = 512;

    auto* input = (unsigned char*)malloc(width * height);
    auto* output = (unsigned char*)malloc(width * height * 3);

    for (int i = 0; i < width * height; i++) {
        input[i] = i % 255;
    }


    auto t0 = get_microseconds();

    normal_process(input, output);

    auto t1 = get_microseconds();

    cout << "normal_process time : " << (t1 - t0) * 0.001 << endl;

    memset_process(input, output);

    auto t2 = get_microseconds();

    cout << "memset_process time : " << (t2 - t1) * 0.001 << endl;

    neon_process(input, output);

    auto t3 = get_microseconds();

    cout << "neon_process time : " << (t3 - t2) * 0.001 << endl;

    return 0;


}