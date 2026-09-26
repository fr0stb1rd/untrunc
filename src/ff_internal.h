
extern "C" {
#include <libavcodec/avcodec.h>
}

struct FFCodecDefault;

/*
Untrunc accesses the internal decode function pointer of an AVCodec struct.
Starting with the following commit, this pointer is no longer public:
https://github.com/FFmpeg/FFmpeg/commit/20f9727018 (59, 25, 100)

Internally, every AVCodec is an FFCodec. We cast it to access the callback,
but we must declare the FFCodec struct ourselves so the compiler can resolve member offsets.

If FFCodec changes in future FFmpeg versions, our definition will drift out of sync,
causing unexpected runtime behavior (e.g. as seen in https://github.com/anthwlock/untrunc/pull/283).

To identify and apply necessary adjustments:
1. Look for changes to codec_internal.h after commit d5fc732 (Apr 14, 2026):
    https://github.com/FFmpeg/FFmpeg/commits/master/libavcodec/codec_internal.h
2. If FFCodec changed (e.g., https://github.com/FFmpeg/FFmpeg/commit/4524d527bf):
    - Find the LIBAVCODEC_VERSION_INT at that commit:
        Major:       https://github.com/FFmpeg/FFmpeg/blob/4524d527bf/libavcodec/version_major.h
        Minor/Patch: https://github.com/FFmpeg/FFmpeg/blob/4524d527bf/libavcodec/version.h
    - Update our FFCodec struct using version-based preprocessor checks (see 9086e3d for reference).

Reference FFmpeg FFCodec definitions:
- (59, 25, 100) https://github.com/FFmpeg/FFmpeg/blob/4243da4ff4/libavcodec/codec_internal.h#L112
- (61, 13, 100) https://github.com/FFmpeg/FFmpeg/blob/4524d527bf/libavcodec/codec_internal.h#L127
- Master:       https://github.com/FFmpeg/FFmpeg/blob/master/libavcodec/codec_internal.h#L127
*/

typedef struct FFCodec {
    AVCodec p;
    unsigned caps_internal : 29;
    unsigned cb_type : 3;
    int priv_data_size;
    int (*update_thread_context)(struct AVCodecContext *dst,
                                const struct AVCodecContext *src);
    int (*update_thread_context_for_user)(struct AVCodecContext *dst,
                                            const struct AVCodecContext *src);
    const FFCodecDefault *defaults;
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(61, 13, 100)
    void (*init_static_data)(struct FFCodec *codec);
#endif
    int (*init)(struct AVCodecContext *);
    union {
        int (*decode)(struct AVCodecContext *avctx, struct AVFrame *frame,
                    int *got_frame_ptr, struct AVPacket *avpkt);
        int (*decode_sub)(struct AVCodecContext *avctx, struct AVSubtitle *sub,
                        int *got_frame_ptr, struct AVPacket *avpkt);
        int (*receive_frame)(struct AVCodecContext *avctx, struct AVFrame *frame);
        int (*encode)(struct AVCodecContext *avctx, struct AVPacket *avpkt,
                    const struct AVFrame *frame, int *got_packet_ptr);
        int (*encode_sub)(struct AVCodecContext *avctx, uint8_t *buf, int buf_size,
                        const struct AVSubtitle *sub);
        int (*receive_packet)(struct AVCodecContext *avctx, struct AVPacket *avpkt);
    } cb;

    // ..

} FFCodec ;

static av_always_inline const FFCodec *ffcodec(const AVCodec *codec)
{
    return (const FFCodec*)codec;
}
