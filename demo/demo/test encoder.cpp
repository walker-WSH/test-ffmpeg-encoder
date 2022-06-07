#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <assert.h>
#include <Windows.h>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
}

//---------------------------------------------------------------------------------

#define SET_POSITION line = __LINE__;
#define ARRAY_COUNT(x) sizeof(x) / sizeof(x[0])

struct size
{
        int x;
        int y;
};

int fpsList[] = { 30,60 };
int keySecondList[] = { 1,3 };
int bitrateList[] = { 2000, 6000 };
int maxbitrateList[] = { 2000, 6000 };
int preanalysisList[] = { 0, 1 };
int filler_dataList[] = { 1, 1 };
int cqp_i_List[] = { 1,25,50 };
int cqp_p_List[] = { 1,25,50 };
enum AVPixelFormat formatList[] = { AV_PIX_FMT_NV12, AV_PIX_FMT_YUV420P };
enum AVColorSpace colorSpaceList[] = { AVCOL_SPC_BT709, AVCOL_SPC_BT470BG };
enum AVColorRange colorRangeList[] = { AVCOL_RANGE_JPEG, AVCOL_RANGE_MPEG };
struct size resolutionList[] = { {1920, 1080}, {1080, 720} };
const char* qualityList[] = { "balanced","speed", "quality", };
const char* rcList[] = { "cbr", "cqp", "vbr_peak", "vbr_latency",};

class CTest
{
        void init_data(AVCodecContext* codec_context, AVFrame* frame);
        void encode(AVCodecContext* enc_ctx, AVFrame* frame, AVPacket* pkt, bool* got);
        void test();

public:
        CTest(bool hevc) : isHEVC(hevc) {}

        void run_test();

public:
        int line = 0;
        bool isHEVC = true;

        int fps = fpsList[0];
        int keySecond = keySecondList[0];
        int bitrate = bitrateList[0];
        int maxbitrate = maxbitrateList[0];
        int preanalysis = preanalysisList[0];
        int filler_data = filler_dataList[0];
        int cqp_i = cqp_i_List[0];
        int cqp_p = cqp_p_List[0];
        enum AVPixelFormat format = formatList[0];
        enum AVColorSpace colorSpace = colorSpaceList[0];
        enum AVColorRange colorRange = colorRangeList[0];
        struct size resolution = resolutionList[0];
        const char* quality = qualityList[0];
        const char* rc = rcList[0];
};

void CTest::init_data(AVCodecContext* codec_context, AVFrame* frame)
{
        if (format == AV_PIX_FMT_YUV420P)
        {
                uint8_t test = GetTickCount() % 50;

                /* Y */
                for (int y = 0; y < codec_context->height; y++)
                {
                        for (int x = 0; x < codec_context->width; x++)
                        {
                                frame->data[0][y * frame->linesize[0] + x] = x + y + test * 3;
                        }
                }
                /* Cb and Cr */
                for (int y = 0; y < codec_context->height / 2; y++)
                {
                        for (int x = 0; x < codec_context->width / 2; x++)
                        {
                                frame->data[1][y * frame->linesize[1] + x] = 128 + y + test * 2;
                                frame->data[2][y * frame->linesize[2] + x] = 64 + x + test * 5;
                        }
                }
        }
        else
        {
                for (size_t i = 0; i < codec_context->width * codec_context->height; i++)
                {
                        frame->data[0][i] =  i % 255;
                }

                for (size_t i = 0; i < codec_context->width * codec_context->height / 2; i++)
                {
                        frame->data[1][i] = i % 255;
                }
        }
}

void CTest::encode(AVCodecContext* enc_ctx, AVFrame* frame, AVPacket* pkt, bool* got)
{
        *got = false;
        int ret;

        SET_POSITION;
        ret = avcodec_send_frame(enc_ctx, frame);
        SET_POSITION;
        if (ret < 0) {
                printf("Error sending a frame for encoding \n");
                return;
        }

        while (ret >= 0) {
                SET_POSITION;
                ret = avcodec_receive_packet(enc_ctx, pkt);
                SET_POSITION;
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                {
                        return;
                }
                else if (ret < 0) {
                        printf("Error during encoding \n");
                        return;
                }

                SET_POSITION;
                printf(".");
                *got = true;
                av_packet_unref(pkt);
                SET_POSITION;
        }
}

void CTest::test()
{
        SET_POSITION;
        if (isHEVC) {
                printf("================ start test HEVC ============== \n");
        }
        else {
                printf("================ start test H264 ============== \n");
        }

        printf("%d x %d \n", resolution.x, resolution.y);
        printf("fps : %d \n", fps);
        printf("keySecond : %d \n", keySecond);
        printf("bitrate : %d \n", bitrate);
        printf("maxbitrate : %d \n", maxbitrate);
        printf("format : %d \n", format);
        printf("colorSpace : %d \n", colorSpace);
        printf("colorRange : %d \n", colorRange);
        printf("preanalysis : %d \n", preanalysis);
        printf("filler_data : %d \n", filler_data);
        printf("cqp_i : %d \n", cqp_i);
        printf("cqp_p : %d \n", cqp_p);
        printf("quality : %s \n", quality);
        printf("rc : %s \n", rc);
        printf("-------------------------\n\n");
          
        SET_POSITION;

        AVCodec* nvenc = nullptr;
        AVCodecContext* context = nullptr;
        AVFrame* vframe = nullptr;
        AVPacket* av_pkt = av_packet_alloc();
        int ret;
        int count = 0;
        bool got_packet = false;

        SET_POSITION;
        if (isHEVC)
        {
                SET_POSITION;
                nvenc = avcodec_find_encoder_by_name("hevc_amf");
                SET_POSITION;
        }
        else {
                SET_POSITION;
                nvenc = avcodec_find_encoder_by_name("h264_amf");
                SET_POSITION;
        }

        if (!nvenc) {
                printf("Couldn't find encoder \n");
                goto fail;
        }

        SET_POSITION;
        context = avcodec_alloc_context3(nvenc);
        if (!context) {
                printf("Failed to create codec context \n");
                goto fail;
        }

        SET_POSITION;

        av_opt_set(context->priv_data, "quality", quality, 0);
        av_opt_set(context->priv_data, "rc", rc, 0);

        if (strcmp(rc, "cqp") == 0)
        {
                bitrate = 0;
                if (isHEVC)
                {
                        av_opt_set_int(context->priv_data, "min_qp_i", cqp_i, 0);
                        av_opt_set_int(context->priv_data, "max_qp_i", cqp_i, 0);
                        av_opt_set_int(context->priv_data, "min_qp_p", cqp_p, 0);
                        av_opt_set_int(context->priv_data, "max_qp_p", cqp_p, 0);
                }
                else {
                        av_opt_set_int(context->priv_data, "qp_i", cqp_i, 0);
                        av_opt_set_int(context->priv_data, "qp_p", cqp_p, 0);
                }
        }
        else if (strcmp(rc, "cbr") == 0)
        {
                av_opt_set_int(context->priv_data, "preanalysis", preanalysis, 0);
                av_opt_set_int(context->priv_data, "filler_data", filler_data, 0);
                context->bit_rate = bitrate * 1000;
                context->rc_buffer_size = bitrate * 1000;
        }
        else {
                if (maxbitrate < bitrate)
                        maxbitrate = bitrate;

                av_opt_set_int(context->priv_data, "preanalysis", preanalysis, 0);
                context->bit_rate = bitrate * 1000;
                context->rc_buffer_size = bitrate * 1000;
                context->rc_max_rate = maxbitrate * 1000;
        }

        SET_POSITION;
        context->gop_size = keySecond * fps;
        context->width = resolution.x;
        context->height = resolution.y;
        context->time_base.num = 1;
        context->time_base.den = fps;
        context->framerate.num = fps;
        context->framerate.den = 1;
        context->pix_fmt = format;
        context->colorspace = colorSpace;
        context->color_range = colorRange;
        context->max_b_frames = 0;

        SET_POSITION;
        ret = avcodec_open2(context, nvenc, NULL);
        if (ret < 0) {
                printf("Failed to open \n");
                goto fail;
        }

        SET_POSITION;
        vframe = av_frame_alloc();
        vframe->format = context->pix_fmt;
        vframe->width = context->width;
        vframe->height = context->height;
        vframe->colorspace = context->colorspace;
        vframe->color_range = context->color_range;

        SET_POSITION;
        ret = av_frame_get_buffer(vframe, 1);
        if (ret < 0) {
                printf("Failed to alloc frame \n");
                goto fail;
        }

        //------------------------------------------------------------
        SET_POSITION;
        for (size_t i = 0; i < 50; i++)
        {
                SET_POSITION;
                init_data(context, vframe);
                vframe->pts = i;
                SET_POSITION;

                encode(context, vframe, av_pkt, &got_packet);
                SET_POSITION;

                if (got_packet)
                {
                        SET_POSITION;
                        ++count;
                }
        }

        SET_POSITION;
        encode(context, vframe, av_pkt, &got_packet);
        SET_POSITION;
        
        if (got_packet)
        {
                SET_POSITION;
                ++count;
        }

        SET_POSITION;
        printf("\n %d packets (%s) \n", count, isHEVC ? "HEVC" : "H264");
        printf("---------- test end-------------- \n\n\n");

        //------------------------------------------------------------
fail:
        if (av_pkt)
        {
                SET_POSITION;
                av_packet_free(&av_pkt);
                SET_POSITION;
                av_pkt = nullptr;
        }

        if (context)
        {
/* @note Do not use this function. Use avcodec_free_context() to destroy a
 * codec context (either open or closed). Opening and closing a codec context
 * multiple times is not supported anymore -- use multiple codec contexts
 * instead.
 */
                //avcodec_close(context); 

                SET_POSITION;
                avcodec_free_context(&context);
                SET_POSITION;
                context = nullptr;
        }
        
        if (vframe)
        {
                SET_POSITION;
                av_frame_unref(vframe);
                SET_POSITION;
                av_frame_free(&vframe);
                SET_POSITION;
                vframe = nullptr;
        }
}

void CTest::run_test()
{
        printf("=============================================================================================== \n");
        DWORD st = GetTickCount();
        for (size_t i1 = 0; i1 < ARRAY_COUNT(fpsList); i1++)
        {
                fps = fpsList[i1];
                for (size_t i2 = 0; i2 < ARRAY_COUNT(keySecondList); i2++)
                {
                        keySecond = keySecondList[i2];
                        for (size_t i3 = 0; i3 < ARRAY_COUNT(bitrateList); i3++)
                        {
                                bitrate = bitrateList[i3];
                                for (size_t i4 = 0; i4 < ARRAY_COUNT(maxbitrateList); i4++)
                                {
                                        maxbitrate = maxbitrateList[i4];
                                        for (size_t i5 = 0; i5 < ARRAY_COUNT(preanalysisList); i5++)
                                        {
                                                preanalysis = preanalysisList[i5];
                                                for (size_t i6 = 0; i6 < ARRAY_COUNT(filler_dataList); i6++)
                                                {
                                                        filler_data = filler_dataList[i6];
                                                        for (size_t i7 = 0; i7 < ARRAY_COUNT(cqp_i_List); i7++)
                                                        {
                                                                cqp_i = cqp_i_List[i7];
                                                                for (size_t i8 = 0; i8 < ARRAY_COUNT(cqp_p_List); i8++)
                                                                {
                                                                        cqp_p = cqp_p_List[i8];
                                                                        for (size_t i9 = 0; i9 < ARRAY_COUNT(formatList); i9++)
                                                                        {
                                                                                format = formatList[i9];
                                                                                for (size_t i0 = 0; i0 < ARRAY_COUNT(colorSpaceList); i0++)
                                                                                {
                                                                                        colorSpace = colorSpaceList[i0];
                                                                                        for (size_t ix = 0; ix < ARRAY_COUNT(colorRangeList); ix++)
                                                                                        {
                                                                                                colorRange = colorRangeList[ix];
                                                                                                for (size_t iy = 0; iy < ARRAY_COUNT(resolutionList); iy++)
                                                                                                {
                                                                                                        resolution = resolutionList[iy];
                                                                                                        for (size_t iz = 0; iz < 3; iz++) // qualityList
                                                                                                        {
                                                                                                                quality = qualityList[iz];
                                                                                                                for (size_t iw = 0; iw < 4; iw++) // rcList
                                                                                                                {
                                                                                                                        rc = rcList[iw];
                                                                                                                        test();
                                                                                                                }
                                                                                                        }
                                                                                                }

                                                                                        }
                                                                                }
                                                                        }
                                                                }
                                                        }
                                                }
                                        }
                                }
                        }
                }
        }


        printf("take %u seconds to test full loop \n", (GetTickCount() - st) / 1000);
}

CTest testHEVC(true);
CTest testH264(false);
DWORD h264Thread = 0;
DWORD h265Thread = 0;

LONG WINAPI ExceptionFilter(struct _EXCEPTION_POINTERS* pExceptionPointers)
{
        printf("crashed! \n");

        SYSTEMTIME st;
        GetLocalTime(&st);

        char file[256];
        if (GetCurrentThreadId() == h265Thread)
        {
                sprintf_s(file, "crash-HEVC-%u-%u-%u.txt", st.wHour, st.wMinute, st.wMinute);
        }
        else
        {
                sprintf_s(file, "crash-H264-%u-%u-%u.txt", st.wHour, st.wMinute, st.wMinute);
        }

        FILE* fp = nullptr;
        fopen_s(&fp, file, "wb+");

        if (fp)
        {
                CTest* test = nullptr;

                if (GetCurrentThreadId() == h265Thread)
                {
                        test = &testHEVC;
                        fprintf(fp, "crashed at HEVC: %d \n", testHEVC.line);
                }
                else
                {
                        test = &testH264;
                        fprintf(fp, "crashed at H264: %d \n", testH264.line);
                }
                    
                fprintf(fp, "%d x %d \n", test->resolution.x, test->resolution.y);
                fprintf(fp, "fps : %d \n", test->fps);
                fprintf(fp, "keySecond : %d \n", test->keySecond);
                fprintf(fp, "bitrate : %d \n", test->bitrate);
                fprintf(fp, "maxbitrate : %d \n", test->maxbitrate);
                fprintf(fp, "format : %d \n", test->format);
                fprintf(fp, "colorSpace : %d \n", test->colorSpace);
                fprintf(fp, "colorRange : %d \n", test->colorRange);
                fprintf(fp, "preanalysis : %d \n", test->preanalysis);
                fprintf(fp, "filler_data : %d \n", test->filler_data);
                fprintf(fp, "cqp_i : %d \n", test->cqp_i);
                fprintf(fp, "cqp_p : %d \n", test->cqp_p);
                fprintf(fp, "quality : %s \n", test->quality);
                fprintf(fp, "rc : %s \n", test->rc);

                fclose(fp);
        }

        return EXCEPTION_EXECUTE_HANDLER;
}

DWORD threadFunc(LPVOID lpThreadParameter)
{
        if (!!lpThreadParameter)
                h265Thread = GetCurrentThreadId();
        else
                h264Thread = GetCurrentThreadId();

        while (1)
        {
                if (lpThreadParameter)
                        testHEVC.run_test();
                else
                        testH264.run_test();
                
                Sleep(3000);
        }

        return 0;
}

int main(int argc, char** argv)
{
        SetUnhandledExceptionFilter(ExceptionFilter);

        CreateThread(0, 0, threadFunc, (LPVOID)true, 0, 0);
        CreateThread(0, 0, threadFunc, (LPVOID)false, 0, 0);

        getchar();
        return 0;
}