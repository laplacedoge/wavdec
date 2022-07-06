#include <stdio.h>
#include "wavdec.h"

int main(int argc, char *argv[]) {
    wav_handle_t wav_handle;
    int opterr;
    int data_rate;

    if(argc < 2) {
        fprintf(stderr, "Wav file path not found!\n");
        return -1;
    }
    opterr = wavdec_init(argv[1], &wav_handle);
    if(opterr != WAVDEC_ERR_NONE) {
        fprintf(stderr, "Failed to initialize wav handle, opterr: %d\n", opterr);
        return -1;
    }
    wavdec_deinit(&wav_handle);
    if(opterr != WAVDEC_ERR_NONE) {
        fprintf(stderr, "Failed to deinitialize wav handle, opterr: %d\n", opterr);
        return -1;
    }
    data_rate = wav_handle.sample_rate * wav_handle.ch_num * (wav_handle.sample_bit / 8);
    printf("          [File size]: %d\n", wav_handle.file_size);
    printf("    [Num of channels]: %d\n", wav_handle.ch_num);
    printf("        [Sample rate]: %d\n", wav_handle.sample_rate);
    printf("    [Bits per sample]: %d\n", wav_handle.sample_bit);
    printf("    [Audio data rate]: %d\n", data_rate);
    printf("    [Audio data size]: %d\n", wav_handle.data_size);
    printf("[\"fmt \" chunk offset]: %d\n", wav_handle.offset.fmt_chunk);
    printf("[\"data\" chunk offset]: %d\n", wav_handle.offset.data_chunk);
    return 0;
}
