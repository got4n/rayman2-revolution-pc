#include <libavcodec/avcodec.h>
#include <string.h>

#include "PC.h"
#include "pc_host.h"

static AVCodecContext *decoder;
static AVCodecParserContext *parser;
static AVPacket *packet;
static AVFrame *next;
static int have_next;
static const uint8_t *stream;
static uint32_t stream_size, read_at;
static int flushed;
static uint32_t width, height;
static const uint8_t *last_logged;

void pc_movie_close(void)
{
    av_parser_close(parser);
    avcodec_free_context(&decoder);
    av_packet_free(&packet);
    av_frame_free(&next);
    parser = NULL;
    have_next = 0;
    width = height = 0;
}

static int decode_ahead(void)
{
    for (;;) {
        const int r = avcodec_receive_frame(decoder, next);

        if (r == 0) {
            return 1;
        }
        if (r != AVERROR(EAGAIN)) {
            return 0;
        }
        if (read_at < stream_size) {
            const int used = av_parser_parse2(parser, decoder, &packet->data, &packet->size, stream + read_at,
                                              (int) (stream_size - read_at), AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);

            if (used < 0) {
                return 0;
            }
            read_at += (uint32_t) used;
        } else if (!flushed) {
            av_parser_parse2(parser, decoder, &packet->data, &packet->size, NULL, 0, AV_NOPTS_VALUE,
                             AV_NOPTS_VALUE, 0);
            if (packet->size == 0) {
                flushed = 1;
                avcodec_send_packet(decoder, NULL);
                continue;
            }
        } else {
            return 0;
        }
        if (packet->size != 0 && avcodec_send_packet(decoder, packet) < 0) {
            return 0;
        }
    }
}

static void read_size(void)
{
    for (uint32_t k = 0; k + 7 <= stream_size && k < 4096; k++) {
        if (stream[k] == 0 && stream[k + 1] == 0 && stream[k + 2] == 1 && stream[k + 3] == 0xB3) {
            width = (uint32_t) stream[k + 4] << 4 | stream[k + 5] >> 4;
            height = (uint32_t) (stream[k + 5] & 15) << 8 | stream[k + 6];
            return;
        }
    }
}

int32_t pc_movie_open(const void *bytes, uint32_t size)
{
    const AVCodec *mpeg2 = avcodec_find_decoder(AV_CODEC_ID_MPEG2VIDEO);

    pc_movie_close();
    if (mpeg2 == NULL) {
        return 0;
    }
    parser = av_parser_init(AV_CODEC_ID_MPEG2VIDEO);
    decoder = avcodec_alloc_context3(mpeg2);
    packet = av_packet_alloc();
    next = av_frame_alloc();
    if (parser == NULL || decoder == NULL || packet == NULL || next == NULL ||
        avcodec_open2(decoder, mpeg2, NULL) < 0) {
        revo_log("[movie] no MPEG-2 decoder");
        pc_movie_close();
        return 0;
    }
    stream = bytes;
    stream_size = size;
    read_at = 0;
    flushed = 0;
    read_size();
    have_next = decode_ahead();
    if (stream != last_logged) {
        last_logged = stream;
        revo_log("[movie] %ux%u, %u bytes%s", width, height, size, have_next ? "" : ": no picture decoded");
    }
    return 1;
}

static uint8_t clamp(int v)
{
    return (uint8_t) (v < 0 ? 0 : v > 255 ? 255 : v);
}

int32_t pc_movie_picture(void *pixels, uint32_t w, uint32_t h)
{
    uint8_t *out = pixels;

    if (!have_next) {
        return 0;
    }
    if (next->format == AV_PIX_FMT_YUV420P || next->format == AV_PIX_FMT_YUV422P ||
        next->format == AV_PIX_FMT_YUV444P) {
        const int sx = next->format == AV_PIX_FMT_YUV444P ? 0 : 1;
        const int sy = next->format == AV_PIX_FMT_YUV420P ? 1 : 0;

        for (uint32_t y = 0; y < h; y++) {
            for (uint32_t x = 0; x < w; x++, out += 4) {
                if ((int) x >= next->width || (int) y >= next->height) {
                    memset(out, 0, 4);
                    continue;
                }
                const int c = 298 * (next->data[0][y * next->linesize[0] + x] - 16);
                const int d = next->data[1][(y >> sy) * next->linesize[1] + (x >> sx)] - 128;
                const int e = next->data[2][(y >> sy) * next->linesize[2] + (x >> sx)] - 128;

                out[0] = clamp((c + 409 * e + 128) >> 8);
                out[1] = clamp((c - 100 * d - 208 * e + 128) >> 8);
                out[2] = clamp((c + 516 * d + 128) >> 8);
                out[3] = 0x80;
            }
        }
    } else {
        memset(pixels, 0, (size_t) w * h * 4);
    }
    have_next = decode_ahead();
    return 1;
}

int32_t pc_movie_ended(void)
{
    return parser == NULL || !have_next;
}

int32_t pc_movie_size(uint32_t *w, uint32_t *h)
{
    if (parser == NULL) {
        return 0;
    }
    *w = width;
    *h = height;
    return 1;
}
