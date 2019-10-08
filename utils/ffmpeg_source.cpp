/*
 * Copyright 2012 Michael Chen <omxcodec@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "FFMPEG"
#include <utils/Log.h>

#include <stdlib.h>
#include "ffmpeg_source.h"

#include <media/DataSourceBase.h>

extern "C" {

#include "config.h"
#include "libavformat/url.h"

}

namespace android {

class FFSource
{
public:
    void set(DataSourceBase *s) { mSource = s; }
    int init_check();
    int read(unsigned char *buf, size_t size);
    int64_t seek(int64_t pos);
    off64_t getSize();

protected:
    DataSourceBase *mSource;
    int64_t mOffset;
};

int FFSource::init_check()
{
    if (mSource->initCheck() != OK) {
        ALOGE("FFSource initCheck failed");
        return -1;
    }

    return 0;
}

int FFSource::read(unsigned char *buf, size_t size)
{
    ssize_t n = 0;

    n = mSource->readAt(mOffset, buf, size);
    if (n == UNKNOWN_ERROR) {
        ALOGE("FFSource readAt failed");
        return AVERROR(errno);
    }
    if (n > 0) {
        mOffset += n;
    }

    return n;
}

int64_t FFSource::seek(int64_t pos)
{
    mOffset = pos;
    return 0;
}

off64_t FFSource::getSize()
{
    off64_t sz = -1;

    if (mSource->getSize(&sz) != OK) {
         ALOGE("FFSource getSize failed");
         return AVERROR(errno);
    }

    return sz;
}

/////////////////////////////////////////////////////////////////

static int android_open(URLContext *h, const char *url, int flags __unused)
{
    // the url in form of "android-source:<DataSourceBase Ptr>",
    // the DataSourceBase Pointer passed by the ffmpeg extractor
    DataSourceBase *source = NULL;
    char url_check[PATH_MAX] = {0};

    ALOGV("android source begin open");

    if (!url) {
        ALOGE("android url is null!");
        return -1;
    }

    ALOGV("android open, url: %s", url);
    sscanf(url + strlen("android-source:"), "%p", &source);
    if(source == NULL){
        ALOGE("ffmpeg open data source error! (invalid source)");
        return -1;
    }

    snprintf(url_check, sizeof(url_check), "android-source:%p",
                source);

    if (strcmp(url_check, url) != 0) {

        char uri[PATH_MAX] = {0};
        if (!source->getUri(uri, sizeof(uri))) {
            ALOGE("ffmpeg open data source error! (source uri)");
            return -1;
        }

        snprintf(url_check, sizeof(url_check), "android-source:%p|file:%s",
                    source, uri);

        if (strcmp(url_check, url) != 0) {
            ALOGE("ffmpeg open data source error! (url check)");
            return -1;
        }
    }

    ALOGV("ffmpeg open android data source success, source ptr: %p", source);

    reinterpret_cast<FFSource *>(h->priv_data)->set(source);

    ALOGV("android source open success");

    return 0;
}
static int android_read(URLContext *h, unsigned char *buf, int size)
{
    FFSource* ffs = (FFSource *)h->priv_data;
    return ffs->read(buf, size);
}

static int android_write(URLContext *h __unused, const unsigned char *buf __unused, int size __unused)
{
    return -1;
}

static int64_t android_seek(URLContext *h, int64_t pos, int whence)
{
    FFSource* ffs = (FFSource*)h->priv_data;

    if (whence == AVSEEK_SIZE) {
        return ffs->getSize();
    }

    ffs->seek(pos);
    return 0;
}

static int android_close(URLContext *h)
{
    ALOGV("android source close");
    reinterpret_cast<FFSource *>(h->priv_data)->set(NULL);
    return 0;
}

static int android_get_handle(URLContext *h)
{
    return (intptr_t)h->priv_data;
}

static int android_check(URLContext *h, int mask)
{
    FFSource* ffs = (FFSource*)h->priv_data;

    /* url_check does not guarantee url_open will be called
     * (and actually it is not designed to do so)
     * If url_open is not called before url_check called, ffs
     * will be null, and we will assume everything is ok.
     */
    if (ffs && (ffs->init_check() < 0))
        return AVERROR(EACCES); // FIXME

    return (mask & AVIO_FLAG_READ);
}

extern "C" URLProtocol ff_android_protocol;

void ffmpeg_register_android_source()
{
    if (ff_android_protocol.name) return;

    ff_android_protocol.name                = "android-source";
    ff_android_protocol.url_open            = android_open;
    ff_android_protocol.url_read            = android_read;
    ff_android_protocol.url_write           = android_write;
    ff_android_protocol.url_seek            = android_seek;
    ff_android_protocol.url_close           = android_close;
    ff_android_protocol.url_get_file_handle = android_get_handle;
    ff_android_protocol.url_check           = android_check;
    ff_android_protocol.priv_data_size      = sizeof(FFSource);
}

}  // namespace android
