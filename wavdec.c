#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "wavdec.h"

static int __opterr = WAVDEC_ERR_NONE;

void wavdec_set_opterr(int opterr) {
    __opterr = opterr;
}

int wavdec_get_opterr() {
    return __opterr;
}

/**
 * These are file system interface functions,
 * you have to implement it in your source file.
 */

/**
 * @brief   Open file.
 * @param   path    File path.
 * @return  NULL is failure, otherwise success.
 */
__attribute__((weak)) void *__wavdec_fsif_open(const char *path) {
    __opterr = WAVDEC_ERR_FILE_OPEN_FAIL;
    return NULL;
}

/**
 * @brief   Get file size.
 * @param   file    File pointer.
 * @return  -1 is failure, otherwise file size.
 */
__attribute__((weak)) int __wavdec_fsif_size(void *file) {
    __opterr = WAVDEC_ERR_FILE_SIZE_FAIL;
    return -1;
}

/**
 * @brief   Seek file.
 * @param   file    File pointer.
 * @param   offset  Seeking offset in which starts from the beginning of the file.
 * @return  -1 is failure, 0 is success.
 */
__attribute__((weak))
int __wavdec_fsif_seek(void *file, uint32_t offset) {
    __opterr = WAVDEC_ERR_FILE_SEEK_FAIL;
    return -1;
}

/**
 * @brief   Read file.
 * @param   file    File pointer.
 * @param   buff    Data buffer pointer.
 * @param   size    Reading data size.
 * @return  -1 is failure, otherwise actual reading size.
 */
__attribute__((weak))
int __wavdec_fsif_read(void *file, void *buff, uint32_t size) {
    __opterr = WAVDEC_ERR_FILE_READ_FAIL;
    return -1;
}

/**
 * @brief   Close file.
 * @param   file    File pointer.
 * @return  -1 is failure, 0 is success.
 */
__attribute__((weak))
int __wavdec_fsif_close(void *file){
    __opterr = WAVDEC_ERR_FILE_CLOSE_FAIL;
    return -1;
}

/**
 * @brief   Initialize wav handle with default value.
 * 
 * @param   handle  Wav handle pointer.
 */
void __wavdec_init_default_wav_handle(wav_handle_t *handle) {
    handle->file = NULL;
    handle->file_size = 0;
    handle->ch_num = WAVDEC_CH_NONE;
    handle->sample_rate = 0;
    handle->sample_bit = WAVDEC_SAMPLE_BIT_NONE;
    handle->data_size = 0;
    handle->progress = 0;
    handle->offset.fmt_chunk = 0;
    handle->offset.data_chunk = 0;
}

/**
 * @brief   Search specified RIFF sub-chunk in wav file.
 * 
 * @param file      File pointer.
 * @param chunk_id  Sub-chunk ID string.
 * @param offset    Starting position in wav file for searching.
 * @param size      Searching range in wav file.
 * @return  -1 is failure, otherwise the sub-chunk offset based on the passed arg 'offset'.
 */
int __wavdec_search_riff_sub_chunk(void *file, const char *chunk_id, uint32_t offset, uint32_t size) {
    uint8_t buff[sizeof(riff_sub_chunk_t)];
    riff_sub_chunk_t *sub_chunk = (riff_sub_chunk_t *)&buff;
    int __size = (int)size;
    uint32_t __offset = 0;
    uint32_t __inc = 0;
    while(__size > sizeof(riff_sub_chunk_t)) {
        __wavdec_fsif_seek(file, offset + __offset);
        if(__opterr != WAVDEC_ERR_NONE) {
            return -1;
        }
        __wavdec_fsif_read(file, &buff, sizeof(riff_sub_chunk_t));
        if(memcmp(&sub_chunk->chunk_id, chunk_id, 4) == 0) {
            if(__size < sizeof(riff_sub_chunk_t) + sub_chunk->chunk_size) {
                __opterr = WAVDEC_ERR_ILLEGAL_CHUNK_SIZE;
                return -1;
            }
            __opterr = WAVDEC_ERR_NONE;
            return (int)__offset;
        } else {
            __inc = sizeof(riff_sub_chunk_t) + sub_chunk->chunk_size;
            __offset += __inc;
            __size -= __inc;
        }
    }
    if(__size == 0) {
        if(memcmp(chunk_id, "fmt ", 4) == 0) {
            __opterr = WAVDEC_ERR_MISS_FMT_CHUNK;
        } else if(memcmp(chunk_id, "data", 4) == 0) {
            __opterr = WAVDEC_ERR_MISS_DATA_CHUNK;
        }
    } else {
        __opterr = WAVDEC_ERR_INSUFFICIENT_DATA;
    }
    return -1;
}

/**
 * @brief   Validate wav file.
 * 
 * @param file    File pointer.
 * @param handle  Wav handle pointer.
 * @return  0 is success, otherwise failure.
 */
int __wavdec_validate_file(void *file, wav_handle_t *handle) {
    wav_handle_t __handle;
    __wavdec_init_default_wav_handle(&__handle);
    __handle.file = file;
    int file_size;
    file_size = __wavdec_fsif_size(file);
    if(__opterr != WAVDEC_ERR_NONE) {
        goto exit;
    }
    if(file_size < (sizeof(wav_riff_chunk_t) + sizeof(wav_fmt_chunk_t) + sizeof(wav_data_chunk_t))) {
        __opterr = WAVDEC_ERR_INSUFFICIENT_DATA;
        goto exit;
    }
    __handle.file_size = file_size;
    uint8_t buff[32];
    __wavdec_fsif_seek(file, 0);
    if(__opterr != WAVDEC_ERR_NONE) {
        goto exit;
    }
    __wavdec_fsif_read(file, &buff, sizeof(wav_riff_chunk_t) + 4 + 4);
    if(__opterr != WAVDEC_ERR_NONE) {
        goto exit;
    }
    wav_riff_chunk_t *wav_riff_chunk = (wav_riff_chunk_t *)&buff;
    if((memcmp(&wav_riff_chunk->chunk_id, "RIFF", 4)) != 0) {
        __opterr = WAVDEC_ERR_MISS_RIFF_CHUNK;
        goto exit;
    }
    if(sizeof(wav_riff_chunk_t) - 4 + wav_riff_chunk->chunk_size != file_size) {
        __opterr = WAVDEC_ERR_ILLEGAL_CHUNK_SIZE;
        goto exit;
    }
    if((memcmp(&wav_riff_chunk->form_type, "WAVE", 4)) != 0) {
        __opterr = WAVDEC_ERR_ILLEGAL_FORM_TYPE;
        goto exit;
    }
    int offset;
    offset = __wavdec_search_riff_sub_chunk(file, "fmt ", sizeof(wav_riff_chunk_t), wav_riff_chunk->chunk_size);
    __handle.offset.fmt_chunk = sizeof(wav_riff_chunk_t) + offset;
    __wavdec_fsif_seek(file, __handle.offset.fmt_chunk);
    if(__opterr != WAVDEC_ERR_NONE) {
        goto exit;
    }
    __wavdec_fsif_read(file, &buff, sizeof(wav_fmt_chunk_t));
    if(__opterr != WAVDEC_ERR_NONE) {
        goto exit;
    }
    wav_fmt_chunk_t *wav_fmt_chunk = (wav_fmt_chunk_t *)buff;
    if(wav_fmt_chunk->audio_type != 1) {
        __opterr = WAVDEC_ERR_ILLEGAL_AUDIO_FMT;
        goto exit;
    }
    if(!(wav_fmt_chunk->ch_num == 1 || wav_fmt_chunk->ch_num == 2)) {
        __opterr = WAVDEC_ERR_ILLEGAL_CH_NUM;
        goto exit;
    }
    __handle.ch_num = wav_fmt_chunk->ch_num;
    __handle.sample_rate = wav_fmt_chunk->sample_rate;
    if(!(wav_fmt_chunk->sample_bit == 8 ||
         wav_fmt_chunk->sample_bit == 16 ||
         wav_fmt_chunk->sample_bit == 24 ||
         wav_fmt_chunk->sample_bit == 32)) {
        __opterr = WAVDEC_ERR_ILLEGAL_SAMPLE_BIT;
        goto exit;
    }
    __handle.sample_bit = wav_fmt_chunk->sample_bit;
    offset = __wavdec_search_riff_sub_chunk(file, "data", sizeof(wav_riff_chunk_t), wav_riff_chunk->chunk_size);
    if(__opterr != WAVDEC_ERR_NONE) {
        goto exit;
    }
    __handle.offset.data_chunk = sizeof(wav_riff_chunk_t) + offset;
    __wavdec_fsif_seek(file, __handle.offset.data_chunk);
    if(__opterr != WAVDEC_ERR_NONE) {
        goto exit;
    }
    __wavdec_fsif_read(file, &buff, sizeof(wav_data_chunk_t));
    wav_data_chunk_t *wav_data_chunk = (wav_data_chunk_t *)buff;
    __handle.data_size = wav_data_chunk->chunk_size;
    if(__opterr != WAVDEC_ERR_NONE) {
        goto exit;
    }
    memcpy(handle, &__handle, sizeof(wav_handle_t));
    __opterr = WAVDEC_ERR_NONE;
exit:
    return __opterr;
}

/**
 * @brief   Initialize wav handle by wav file path.
 * 
 * @param path    File path string.
 * @param handle  Wav handle pointer.
 * @return  0 is success, otherwise failure.
 */
int wavdec_init(const char *path, wav_handle_t *handle) {
    void *file;
    file = __wavdec_fsif_open(path);
    if(__opterr != WAVDEC_ERR_NONE) {
        return __opterr;
    }
    __wavdec_init_default_wav_handle(handle);
    __wavdec_validate_file(file, handle);
    if(__opterr != WAVDEC_ERR_NONE) {
        goto exit;
    }
    __opterr = WAVDEC_ERR_NONE;
exit:
    return __opterr;
}

/**
 * @brief   Deinitialize wav handle.
 * 
 * @param handle  Wav handle pointer.
 * @return  0 is success, otherwise failure.
 */
int wavdec_deinit(wav_handle_t *handle) {
    __wavdec_fsif_close(handle->file);
    if(__opterr != WAVDEC_ERR_NONE) {
        return __opterr;
    }
    return WAVDEC_ERR_NONE;
}
