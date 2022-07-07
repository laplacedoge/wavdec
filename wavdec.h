#ifndef __WAVDEC_H__
#define __WAVDEC_H__

#include <stdint.h>

enum {
    WAVDEC_ERR_NONE = 0,
    WAVDEC_ERR_FSIF,
        WAVDEC_ERR_FILE_OPEN_FAIL,
        WAVDEC_ERR_FILE_SIZE_FAIL,
        WAVDEC_ERR_FILE_SEEK_FAIL,
        WAVDEC_ERR_FILE_READ_FAIL,
        WAVDEC_ERR_FILE_CLOSE_FAIL,
    WAVDEC_ERR_VALIDATE,
        WAVDEC_ERR_INSUFFICIENT_DATA,
        WAVDEC_ERR_NOT_WAV_FILE,
        WAVDEC_ERR_ILLEGAL_WAV_FILE,
            WAVDEC_ERR_ILLEGAL_CHUNK_SIZE,
            WAVDEC_ERR_MISS_RIFF_CHUNK,
            WAVDEC_ERR_ILLEGAL_RIFF_CHUNK,
                WAVDEC_ERR_ILLEGAL_FORM_TYPE,
            WAVDEC_ERR_MISS_FMT_CHUNK,
            WAVDEC_ERR_ILLEGAL_FMT_CHUNK,
                WAVDEC_ERR_ILLEGAL_AUDIO_FMT,
                WAVDEC_ERR_ILLEGAL_CH_NUM,
                WAVDEC_ERR_ILLEGAL_SAMPLE_RATE,
                WAVDEC_ERR_ILLEGAL_SAMPLE_BIT,
            WAVDEC_ERR_MISS_DATA_CHUNK,
            WAVDEC_ERR_ILLEGAL_DATA_CHUNK,
    WAVDEC_ERR_ILLEGAL_OPT,
        WAVDEC_ERR_ILLEGAL_ARG,
        WAVDEC_ERR_FRAME_OVERFLOW,
};

enum {
    WAVDEC_CH_NONE = 0, // Channel not selected.
    WAVDEC_CH_MONO,     // Single channel.
    WAVDEC_CH_STEREO,   // Dual channel.
};

/**
 * Audio data format for single channel(mono):
 * +-----------------------+-----------------------+-----------------------+
 * |       sample 1        |       sample 2        |       sample 2        |
 * +-----------+-----------+-----------+-----------+-----------+-----------+
 * | low byte  | high byte | low byte  | high byte | low byte  | high byte |
 * +-----------+-----------+-----------+-----------+-----------+-----------+
 * 
 * Audio data format for dual channel(stereo):
 * +-----------------------------------------------+-----------------------------------------------+
 * |                   sample 1                    |                   sample 2                    |
 * +-----------------------+-----------------------+-----------------------+-----------------------+
 * |     left channel      |     right channel     |     left channel      |     right channel     |
 * +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
 * | low byte  | high byte | low byte  | high byte | low byte  | high byte | low byte  | high byte |
 * +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
 */

enum {
    WAVDEC_SAMPLE_BIT_NONE = 0, // Sample bit not selected.
    WAVDEC_SAMPLE_BIT_8,        // 8-bit sample.
    WAVDEC_SAMPLE_BIT_16,       // 16-bit sample.
    WAVDEC_SAMPLE_BIT_24,       // 24-bit sample.
    WAVDEC_SAMPLE_BIT_32,       // 32-bit sample.
};

enum {
    WAVDEC_SEEK_SET = 0,        // From beginning frame.
    WAVDEC_SEEK_CURT,           // From current frame.
    WAVDEC_SEEK_END,            // From ending frame.
};

enum {
    WAVDEC_CONV_MS2FRAME = 0,   // Convert milliseconds to frames.
    WAVDEC_CONV_MS2BYTE,        // Convert milliseconds to bytes.
    WAVDEC_CONV_FRAME2MS,       // Convert frames to milliseconds.
    WAVDEC_CONV_FRAME2BYTE,     // Convert frames to bytes.
};

typedef struct wav_handle {
    void *file;                 // Wav file pointer.
    uint32_t file_size;         // Wav file size.
    uint16_t ch_num;            // Number of audio channels.
    uint16_t sample_rate;       // Sample rate.
    uint16_t sample_bit;        // Bits per sample.
    uint32_t data_size;         // Size of audio data portion.
    uint32_t progress;          // Audio playing progress(in frames).
    struct wav_handle_offset {
        uint32_t fmt_chunk;     // "fmt " sub-chunk offset in wav file.
        uint32_t data_chunk;    // "data" sub-chunk offset in wav file.
    } offset;
} wav_handle_t;

typedef struct wav_riff_chunk {
    char chunk_id[4];           // String "RIFF"
    uint32_t chunk_size;        // Data size of this chunk, also include 'form_type'
    char form_type[4];          // String "WAVE"
} wav_riff_chunk_t;

typedef struct riff_sub_chunk {
    char chunk_id[4];
    uint32_t chunk_size;
} riff_sub_chunk_t;

typedef struct wav_fmt_chunk {
    char chunk_id[4];           // String "fmt "
    uint32_t chunk_size;        // Data size of this chunk
    uint16_t audio_type;        // 1 is for PCM, doesn't support other types
    uint16_t ch_num;            // Number of channels
    uint32_t sample_rate;       // Audio sample rate in HZ
    uint32_t data_rate;         // Audio data rate in byte per second
    uint16_t block_align;
    uint16_t sample_bit;        // Length of bits for per audio sample
} wav_fmt_chunk_t;

typedef struct wav_data_chunk {
    char chunk_id[4];           // String "data"
    uint32_t chunk_size;        // Data size of this chunk
    uint8_t data[];             // Audio data
} wav_data_chunk_t;

void wavdec_set_opterr(int opterr);

int wavdec_get_opterr();

int wavdec_init(const char *path, wav_handle_t *handle);

int wavdec_deinit(wav_handle_t *handle);

uint32_t wavdec_get_frame_size(wav_handle_t *handle);

uint32_t wavdec_get_total_frames(wav_handle_t *handle);

uint32_t wavdec_conv(wav_handle_t *handle, uint32_t value, int code);

int wavdec_seek(wav_handle_t *handle, int offset, int whence);

int wavdec_read(wav_handle_t *handle, void *buff, uint32_t size);

#endif
