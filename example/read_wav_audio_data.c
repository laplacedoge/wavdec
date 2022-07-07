#include <stdio.h>
#include <stdlib.h>
#include "wavdec.h"

int main(int argc, char *argv[]) {
    wav_handle_t wav_handle;
    int opterr;
    uint32_t start_time;    // Start time of audio playing.
    uint32_t end_time;      // End time of audio playing.
    uint32_t audio_frames;  // Number of frames during audio playing.
    uint32_t audio_bytes;   // Number of bytes for audio playing data.
    uint8_t *buff;          // Buffer to store audio data.

    if(argc < 2) {
        fprintf(stderr, "Wav file path not found!\n");
        return -1;
    }
    opterr = wavdec_init(argv[1], &wav_handle);
    if(opterr != WAVDEC_ERR_NONE) {
        fprintf(stderr, "Failed to initialize wav handle, opterr: %d.\n", opterr);
        return -1;
    }
    printf("[Frame size]: %d\n", wavdec_get_frame_size(&wav_handle));
    printf("[Total frames]: %d\n", wavdec_get_total_frames(&wav_handle));
    start_time = 2 * 60 * 1000;
    end_time = 3 * 60 * 1000;
    audio_frames = wavdec_conv(&wav_handle, end_time - start_time, WAVDEC_CONV_MS2FRAME);
    audio_bytes = wavdec_conv(&wav_handle, end_time - start_time, WAVDEC_CONV_MS2BYTE);
    printf("[Audio frames]: %d\n", audio_frames);
    printf("[Audio bytes]: %d\n", audio_bytes);
    buff = (uint8_t *)malloc(audio_bytes);
    if(buff == NULL) {
        fprintf(stderr, "Failed to malloc memory for audio data!\n");
        goto err_exit;
    }
    wavdec_seek(&wav_handle, (int)wavdec_conv(&wav_handle, start_time, WAVDEC_CONV_MS2FRAME), WAVDEC_SEEK_SET);
    if(opterr != WAVDEC_ERR_NONE) {
        fprintf(stderr, "Failed to seek audio data, opterr: %d.\n", opterr);
        goto err_exit;
    }
    wavdec_read(&wav_handle, buff, audio_frames);
    if(opterr != WAVDEC_ERR_NONE) {
        fprintf(stderr, "Failed to read audio data, opterr: %d.\n", opterr);
        goto err_exit;
    }
    free(buff);
    wavdec_deinit(&wav_handle);
    if(opterr != WAVDEC_ERR_NONE) {
        fprintf(stderr, "Failed to deinitialize wav handle, opterr: %d.\n", opterr);
        return -1;
    }
    return 0;
err_exit:
    free(buff);
    return -1;
}
