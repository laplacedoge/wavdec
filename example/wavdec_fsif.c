#include <stddef.h>
#include <stdio.h>
#include "wavdec.h"

void *__wavdec_fsif_open(const char *path) {
    void *p = (void *)fopen(path, "r");
    if(p == NULL) {
        wavdec_set_opterr(WAVDEC_ERR_FILE_OPEN_FAIL);
        return NULL;
    }
    wavdec_set_opterr(WAVDEC_ERR_NONE);
    return p;
}

int __wavdec_fsif_size(void *file) {
    int seek_ret = fseek((FILE *)file, 0L, SEEK_END);
    if(seek_ret < 0) {
        goto err_exit;
    }
    long tell_ret = ftell((FILE *)file);
    if(tell_ret < 0) {
        goto err_exit;
    }
    seek_ret = fseek((FILE *)file, 0L, SEEK_SET);
    if(seek_ret < 0) {
        goto err_exit;
    }
    wavdec_set_opterr(WAVDEC_ERR_NONE);
    return (int)tell_ret;
err_exit:
    wavdec_set_opterr(WAVDEC_ERR_FILE_SIZE_FAIL);
    return -1;
}

int __wavdec_fsif_seek(void *file, uint32_t offset) {
    off_t off = fseek((FILE *)file, offset, SEEK_SET);
    if(off < 0) {
        wavdec_set_opterr(WAVDEC_ERR_FILE_SEEK_FAIL);
        return -1;
    }
    wavdec_set_opterr(WAVDEC_ERR_NONE);
    return 0;
}

int __wavdec_fsif_read(void *file, void *buff, uint32_t size) {
    size_t rsize = fread(buff, size, 1, (FILE *)file);
    if(rsize < 0) {
        wavdec_set_opterr(WAVDEC_ERR_FILE_READ_FAIL);
        return -1;
    }
    wavdec_set_opterr(WAVDEC_ERR_NONE);
    return (int)rsize;
}

int __wavdec_fsif_close(void *file){
    int ret = fclose((FILE *)file);
    if(ret < 0) {
        wavdec_set_opterr(WAVDEC_ERR_FILE_CLOSE_FAIL);
        return -1;
    }
    wavdec_set_opterr(WAVDEC_ERR_NONE);
    return 0;
}
