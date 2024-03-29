#include "mppEncoder.hpp"
#include <iostream>

CMppEncoder::CMppEncoder(int argc, char **argv)
{
    MPP_RET ret = MPP_OK;
    info = mpp_calloc(MpiEncMultiCtxInfo, 1);

    info->cmd = mpi_enc_test_cmd_get();

    ret = mpi_enc_test_cmd_update_by_args(info->cmd, argc, argv);
    if (ret)
    {
        std::cout << "get parameter failed!" << std::endl;
    }
    info->name = argv[0];
    info->chn = 0;
    std::cout << info->name << std::endl;
    mpi_enc_test_cmd_show_opt(info->cmd);

    total_rate = 0;
    ret = MPP_NOK;
    pkt = av_packet_alloc();
    av_init_packet(pkt);
}

MPP_RET CMppEncoder::TestMppEncCfgSetup()
{
    MpiEncTestArgs *cmd = info->cmd;
    MpiEncTestData *p = &info->ctx;
    MppApi *mpi = p->mpi;
    MppCtx ctx = p->ctx;
    MppEncCfg cfg = p->cfg;
    RK_U32 quiet = cmd->quiet;
    MPP_RET ret;
    RK_U32 rotation;
    RK_U32 mirroring;
    RK_U32 flip;
    RK_U32 gop_mode = p->gop_mode;
    MppEncRefCfg ref = NULL;

    /* setup default parameter */
    if (p->fps_in_den == 0)
        p->fps_in_den = 1;
    if (p->fps_in_num == 0)
        p->fps_in_num = 60;
    if (p->fps_out_den == 0)
        p->fps_out_den = 1;
    if (p->fps_out_num == 0)
        p->fps_out_num = 60;

    if (!p->bps)
        p->bps = p->width * p->height / 8 * (p->fps_out_num / p->fps_out_den);

    mpp_enc_cfg_set_s32(cfg, "tune:scene_mode", p->scene_mode);

    mpp_enc_cfg_set_s32(cfg, "prep:width", p->width);
    mpp_enc_cfg_set_s32(cfg, "prep:height", p->height);
    mpp_enc_cfg_set_s32(cfg, "prep:hor_stride", p->hor_stride);
    mpp_enc_cfg_set_s32(cfg, "prep:ver_stride", p->ver_stride);
    mpp_enc_cfg_set_s32(cfg, "prep:format", p->fmt);

    mpp_enc_cfg_set_s32(cfg, "rc:mode", p->rc_mode);

    /* fix input / output frame rate */
    mpp_enc_cfg_set_s32(cfg, "rc:fps_in_flex", p->fps_in_flex);
    mpp_enc_cfg_set_s32(cfg, "rc:fps_in_num", p->fps_in_num);
    mpp_enc_cfg_set_s32(cfg, "rc:fps_in_denorm", p->fps_in_den);
    mpp_enc_cfg_set_s32(cfg, "rc:fps_out_flex", p->fps_out_flex);
    mpp_enc_cfg_set_s32(cfg, "rc:fps_out_num", p->fps_out_num);
    mpp_enc_cfg_set_s32(cfg, "rc:fps_out_denorm", p->fps_out_den);

    /* drop frame or not when bitrate overflow */
    mpp_enc_cfg_set_u32(cfg, "rc:drop_mode", MPP_ENC_RC_DROP_FRM_DISABLED);
    mpp_enc_cfg_set_u32(cfg, "rc:drop_thd", 20); /* 20% of max bps */
    mpp_enc_cfg_set_u32(cfg, "rc:drop_gap", 1);  /* Do not continuous drop frame */

    /* setup bitrate for different rc_mode */
    mpp_enc_cfg_set_s32(cfg, "rc:bps_target", p->bps);
    switch (p->rc_mode)
    {
    case MPP_ENC_RC_MODE_FIXQP:
    {
        /* do not setup bitrate on FIXQP mode */
    }
    break;
    case MPP_ENC_RC_MODE_CBR:
    {
        /* CBR mode has narrow bound */
        mpp_enc_cfg_set_s32(cfg, "rc:bps_max", p->bps_max ? p->bps_max : p->bps * 17 / 16);
        mpp_enc_cfg_set_s32(cfg, "rc:bps_min", p->bps_min ? p->bps_min : p->bps * 15 / 16);
    }
    break;
    case MPP_ENC_RC_MODE_VBR:
    case MPP_ENC_RC_MODE_AVBR:
    {
        /* VBR mode has wide bound */
        mpp_enc_cfg_set_s32(cfg, "rc:bps_max", p->bps_max ? p->bps_max : p->bps * 17 / 16);
        mpp_enc_cfg_set_s32(cfg, "rc:bps_min", p->bps_min ? p->bps_min : p->bps * 1 / 16);
    }
    break;
    default:
    {
        /* default use CBR mode */
        mpp_enc_cfg_set_s32(cfg, "rc:bps_max", p->bps_max ? p->bps_max : p->bps * 17 / 16);
        mpp_enc_cfg_set_s32(cfg, "rc:bps_min", p->bps_min ? p->bps_min : p->bps * 15 / 16);
    }
    break;
    }

    /* setup qp for different codec and rc_mode */
    switch (p->type)
    {
    case MPP_VIDEO_CodingAVC:
    case MPP_VIDEO_CodingHEVC:
    {
        switch (p->rc_mode)
        {
        case MPP_ENC_RC_MODE_FIXQP:
        {
            RK_S32 fix_qp = cmd->qp_init;

            mpp_enc_cfg_set_s32(cfg, "rc:qp_init", fix_qp);
            mpp_enc_cfg_set_s32(cfg, "rc:qp_max", fix_qp);
            mpp_enc_cfg_set_s32(cfg, "rc:qp_min", fix_qp);
            mpp_enc_cfg_set_s32(cfg, "rc:qp_max_i", fix_qp);
            mpp_enc_cfg_set_s32(cfg, "rc:qp_min_i", fix_qp);
            mpp_enc_cfg_set_s32(cfg, "rc:qp_ip", 0);
            mpp_enc_cfg_set_s32(cfg, "rc:fqp_min_i", fix_qp);
            mpp_enc_cfg_set_s32(cfg, "rc:fqp_max_i", fix_qp);
            mpp_enc_cfg_set_s32(cfg, "rc:fqp_min_p", fix_qp);
            mpp_enc_cfg_set_s32(cfg, "rc:fqp_max_p", fix_qp);
        }
        break;
        case MPP_ENC_RC_MODE_CBR:
        case MPP_ENC_RC_MODE_VBR:
        case MPP_ENC_RC_MODE_AVBR:
        {
            mpp_enc_cfg_set_s32(cfg, "rc:qp_init", cmd->qp_init ? cmd->qp_init : -1);
            mpp_enc_cfg_set_s32(cfg, "rc:qp_max", cmd->qp_max ? cmd->qp_max : 51);
            mpp_enc_cfg_set_s32(cfg, "rc:qp_min", cmd->qp_min ? cmd->qp_min : 10);
            mpp_enc_cfg_set_s32(cfg, "rc:qp_max_i", cmd->qp_max_i ? cmd->qp_max_i : 51);
            mpp_enc_cfg_set_s32(cfg, "rc:qp_min_i", cmd->qp_min_i ? cmd->qp_min_i : 10);
            mpp_enc_cfg_set_s32(cfg, "rc:qp_ip", 2);
            mpp_enc_cfg_set_s32(cfg, "rc:fqp_min_i", cmd->fqp_min_i ? cmd->fqp_min_i : 10);
            mpp_enc_cfg_set_s32(cfg, "rc:fqp_max_i", cmd->fqp_max_i ? cmd->fqp_max_i : 51);
            mpp_enc_cfg_set_s32(cfg, "rc:fqp_min_p", cmd->fqp_min_p ? cmd->fqp_min_p : 10);
            mpp_enc_cfg_set_s32(cfg, "rc:fqp_max_p", cmd->fqp_max_p ? cmd->fqp_max_p : 51);
        }
        break;
        default:
        {
            mpp_err_f("unsupport encoder rc mode %d\n", p->rc_mode);
        }
        break;
        }
    }
    break;
    case MPP_VIDEO_CodingVP8:
    {
        /* vp8 only setup base qp range */
        mpp_enc_cfg_set_s32(cfg, "rc:qp_init", cmd->qp_init ? cmd->qp_init : 40);
        mpp_enc_cfg_set_s32(cfg, "rc:qp_max", cmd->qp_max ? cmd->qp_max : 127);
        mpp_enc_cfg_set_s32(cfg, "rc:qp_min", cmd->qp_min ? cmd->qp_min : 0);
        mpp_enc_cfg_set_s32(cfg, "rc:qp_max_i", cmd->qp_max_i ? cmd->qp_max_i : 127);
        mpp_enc_cfg_set_s32(cfg, "rc:qp_min_i", cmd->qp_min_i ? cmd->qp_min_i : 0);
        mpp_enc_cfg_set_s32(cfg, "rc:qp_ip", 6);
    }
    break;
    case MPP_VIDEO_CodingMJPEG:
    {
        /* jpeg use special codec config to control qtable */
        mpp_enc_cfg_set_s32(cfg, "jpeg:q_factor", cmd->qp_init ? cmd->qp_init : 80);
        mpp_enc_cfg_set_s32(cfg, "jpeg:qf_max", cmd->qp_max ? cmd->qp_max : 99);
        mpp_enc_cfg_set_s32(cfg, "jpeg:qf_min", cmd->qp_min ? cmd->qp_min : 1);
    }
    break;
    default:
    {
    }
    break;
    }

    /* setup codec  */
    mpp_enc_cfg_set_s32(cfg, "codec:type", p->type);
    switch (p->type)
    {
    case MPP_VIDEO_CodingAVC:
    {
        RK_U32 constraint_set;

        /*
         * H.264 profile_idc parameter
         * 66  - Baseline profile
         * 77  - Main profile
         * 100 - High profile
         */
        mpp_enc_cfg_set_s32(cfg, "h264:profile", 100);
        /*
         * H.264 level_idc parameter
         * 10 / 11 / 12 / 13    - qcif@15fps / cif@7.5fps / cif@15fps / cif@30fps
         * 20 / 21 / 22         - cif@30fps / half-D1@@25fps / D1@12.5fps
         * 30 / 31 / 32         - D1@25fps / 720p@30fps / 720p@60fps
         * 40 / 41 / 42         - 1080p@30fps / 1080p@30fps / 1080p@60fps
         * 50 / 51 / 52         - 4K@30fps
         */
        mpp_enc_cfg_set_s32(cfg, "h264:level", 42);
        mpp_enc_cfg_set_s32(cfg, "h264:cabac_en", 1);
        mpp_enc_cfg_set_s32(cfg, "h264:cabac_idc", 0);
        mpp_enc_cfg_set_s32(cfg, "h264:trans8x8", 1);

        mpp_env_get_u32("constraint_set", &constraint_set, 0);
        if (constraint_set & 0x3f0000)
            mpp_enc_cfg_set_s32(cfg, "h264:constraint_set", constraint_set);
    }
    break;
    case MPP_VIDEO_CodingHEVC:
    case MPP_VIDEO_CodingMJPEG:
    case MPP_VIDEO_CodingVP8:
    {
    }
    break;
    default:
    {
        mpp_err_f("unsupport encoder coding type %d\n", p->type);
    }
    break;
    }

    p->split_mode = 0;
    p->split_arg = 0;
    p->split_out = 0;

    mpp_env_get_u32("split_mode", &p->split_mode, MPP_ENC_SPLIT_NONE);
    mpp_env_get_u32("split_arg", &p->split_arg, 0);
    mpp_env_get_u32("split_out", &p->split_out, 0);

    if (p->split_mode)
    {
        mpp_log_q(quiet, "%p split mode %d arg %d out %d\n", ctx,
                  p->split_mode, p->split_arg, p->split_out);
        mpp_enc_cfg_set_s32(cfg, "split:mode", p->split_mode);
        mpp_enc_cfg_set_s32(cfg, "split:arg", p->split_arg);
        mpp_enc_cfg_set_s32(cfg, "split:out", p->split_out);
    }

    mpp_env_get_u32("mirroring", &mirroring, 0);
    mpp_env_get_u32("rotation", &rotation, 0);
    mpp_env_get_u32("flip", &flip, 0);

    mpp_enc_cfg_set_s32(cfg, "prep:mirroring", mirroring);
    mpp_enc_cfg_set_s32(cfg, "prep:rotation", rotation);
    mpp_enc_cfg_set_s32(cfg, "prep:flip", flip);

    // config gop_len and ref cfg
    mpp_enc_cfg_set_s32(cfg, "rc:gop", p->gop_len ? p->gop_len : p->fps_out_num * 2);

    mpp_env_get_u32("gop_mode", &gop_mode, gop_mode);

    if (gop_mode)
    {
        mpp_enc_ref_cfg_init(&ref);

        if (p->gop_mode < 4)
            mpi_enc_gen_ref_cfg(ref, gop_mode);
        else
            mpi_enc_gen_smart_gop_ref_cfg(ref, p->gop_len, p->vi_len);

        mpp_enc_cfg_set_ptr(cfg, "rc:ref_cfg", ref);
    }

    ret = mpi->control(ctx, MPP_ENC_SET_CFG, cfg);
    if (ret)
    {
        mpp_err("mpi control enc set cfg failed ret %d\n", ret);
        goto RET;
    }

    if (ref)
        mpp_enc_ref_cfg_deinit(&ref);

    /* optional */
    {
        RK_U32 sei_mode;

        mpp_env_get_u32("sei_mode", &sei_mode, MPP_ENC_SEI_MODE_ONE_FRAME);
        // p->sei_mode = sei_mode;
        p->sei_mode = MPP_ENC_SEI_MODE_ONE_FRAME;
        ret = mpi->control(ctx, MPP_ENC_SET_SEI_CFG, &p->sei_mode);
        if (ret)
        {
            mpp_err("mpi control enc set sei cfg failed ret %d\n", ret);
            goto RET;
        }
    }

    if (p->type == MPP_VIDEO_CodingAVC || p->type == MPP_VIDEO_CodingHEVC)
    {
        p->header_mode = MPP_ENC_HEADER_MODE_EACH_IDR;
        ret = mpi->control(ctx, MPP_ENC_SET_HEADER_MODE, &p->header_mode);
        if (ret)
        {
            mpp_err("mpi control enc set header mode failed ret %d\n", ret);
            goto RET;
        }
    }

    /* setup test mode by env */
    mpp_env_get_u32("osd_enable", &p->osd_enable, 0);
    mpp_env_get_u32("osd_mode", &p->osd_mode, MPP_ENC_OSD_PLT_TYPE_DEFAULT);
    mpp_env_get_u32("roi_enable", &p->roi_enable, 0);
    mpp_env_get_u32("user_data_enable", &p->user_data_enable, 0);

RET:
    return ret;
}

void CMppEncoder::Init()
{
    MpiEncTestArgs *cmd = info->cmd;
    MpiEncTestData *p = &info->ctx;
    MppCtx ctx = p->ctx;
    MpiEncMultiCtxRet *enc_ret = &info->ret;
    MppPollType timeout = MPP_POLL_BLOCK;
    RK_U32 quiet = cmd->quiet;
    RK_S64 t_s = 0;
    RK_S64 t_e = 0;
    MPP_RET ret = MPP_OK;
    mpp_log_q(quiet, "%s start\n", info->name);
    ret = TestCtxInit();
    if (ret)
    {
        mpp_err_f("test data init failed ret %d\n", ret);
        TestCtxDeinit();
    }

    ret = mpp_buffer_group_get_internal(&p->buf_grp, MPP_BUFFER_TYPE_DRM | MPP_BUFFER_FLAGS_CACHABLE);
    if (ret)
    {
        mpp_err_f("failed to get mpp buffer group ret %d\n", ret);
        TestCtxDeinit();
    }

    ret = mpp_buffer_get(p->buf_grp, &p->frm_buf, p->frame_size + p->header_size);
    if (ret)
    {
        mpp_err_f("failed to get buffer for input frame ret %d\n", ret);
        TestCtxDeinit();
    }

    ret = mpp_buffer_get(p->buf_grp, &p->pkt_buf, p->frame_size);
    if (ret)
    {
        mpp_err_f("failed to get buffer for output packet ret %d\n", ret);
        TestCtxDeinit();
    }

    ret = mpp_buffer_get(p->buf_grp, &p->md_info, p->mdinfo_size);
    if (ret)
    {
        mpp_err_f("failed to get buffer for motion info output packet ret %d\n", ret);
        TestCtxDeinit();
    }

    // encoder demo
    ret = mpp_create(&p->ctx, &p->mpi);
    if (ret)
    {
        mpp_err("mpp_create failed ret %d\n", ret);
        TestCtxDeinit();
    }

    mpp_log_q(quiet, "%p encoder test start w %d h %d type %d\n",
              p->ctx, p->width, p->height, p->type);

    ret = p->mpi->control(p->ctx, MPP_SET_OUTPUT_TIMEOUT, &timeout);
    if (MPP_OK != ret)
    {
        mpp_err("mpi control set output timeout %d ret %d\n", timeout, ret);
        TestCtxDeinit();
    }

    ret = mpp_init(p->ctx, MPP_CTX_ENC, p->type);
    if (ret)
    {
        mpp_err("mpp_init failed ret %d\n", ret);
        TestCtxDeinit();
    }

    ret = mpp_enc_cfg_init(&p->cfg);
    if (ret)
    {
        mpp_err_f("mpp_enc_cfg_init failed ret %d\n", ret);
        TestCtxDeinit();
    }

    ret = p->mpi->control(p->ctx, MPP_ENC_GET_CFG, p->cfg);
    if (ret)
    {
        mpp_err_f("get enc cfg failed ret %d\n", ret);
        TestCtxDeinit();
    }

    ret = TestMppEncCfgSetup();
    if (ret)
    {
        mpp_err_f("test mpp setup failed ret %d\n", ret);
        TestCtxDeinit();
    }

    memset(&checkcrc, 0, sizeof(checkcrc));
    checkcrc.sum = mpp_malloc(RK_ULONG, 512);

    if (p->type == MPP_VIDEO_CodingAVC || p->type == MPP_VIDEO_CodingHEVC)
    {
        MppPacket packet = NULL;

        /*
         * Can use packet with normal malloc buffer as input not pkt_buf.
         * Please refer to vpu_api_legacy.cpp for normal buffer case.
         * Using pkt_buf buffer here is just for simplifing demo.
         */
        mpp_packet_init_with_buffer(&packet, p->pkt_buf);
        /* NOTE: It is important to clear output packet length!! */
        mpp_packet_set_length(packet, 0);

        ret = p->mpi->control(info->ctx.ctx, MPP_ENC_GET_HDR_SYNC, packet);
        if (ret)
        {
            mpp_err("mpi control enc get extra info failed\n");
        }
        else
        {
            /* get and write sps/pps for H.264 */

            void *ptr = mpp_packet_get_pos(packet);
            size_t len = mpp_packet_get_length(packet);

            sps_header.data = (uint8_t *)malloc(len);
            sps_header.size = len;
            memcpy(sps_header.data, ptr, len);
            std::cout << len << std::endl;

            if (p->fp_output)
                fwrite(ptr, 1, len, p->fp_output);
        }

        mpp_packet_deinit(&packet);
    }

    init_rtmp_streamer("rtmp://localhost:1935/stream", sps_header.data, sps_header.size);
}

int CMppEncoder::init_rtmp_streamer(char *stream, uint8_t *data, uint32_t size)
{
    int ret;
    AVCodecParameters *in_codecpar;
    if ((ret = avformat_network_init()) < 0)
    {
        fprintf(stderr, "avformat_network_init failed!");
        return -1;
    }

    avformat_alloc_output_context2(&ofmt_ctx, NULL, "flv", stream);
    if (!ofmt_ctx)
    {
        fprintf(stderr, "Could not create output context\n");
        return -1;
    }

    out_stream = avformat_new_stream(ofmt_ctx, NULL);
    if (!out_stream)
    {
        printf("Failed allocating output stream!\n");
        goto end;
    }

    in_codecpar = out_stream->codecpar;
    in_codecpar->codec_id = AV_CODEC_ID_H264;
    in_codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
    in_codecpar->codec_tag = 0;
    in_codecpar->format = AV_PIX_FMT_BGR24;
    in_codecpar->width = 1920;
    in_codecpar->height = 1080;
    in_codecpar->extradata = data;
    in_codecpar->extradata_size = size;

    av_dump_format(ofmt_ctx, 0, stream, 1);

    // 打开输出URL（Open output URL）
    if (!(ofmt_ctx->oformat->flags & AVFMT_NOFILE))
    {
        ret = avio_open(&ofmt_ctx->pb, stream, AVIO_FLAG_WRITE);
        if (ret < 0)
        {
            printf("Could not open output URL '%s'", stream);
            goto end;
        }
    }
    // 写文件头（Write file header）
    ret = avformat_write_header(ofmt_ctx, NULL);
    if (ret < 0)
    {
        printf("Error occurred when opening output URL\n");
        goto end;
    }
    // free(data);
    return 0;
end:
    if (ofmt_ctx && !(ofmt_ctx->oformat->flags & AVFMT_NOFILE))
        avio_close(ofmt_ctx->pb);
    avformat_free_context(ofmt_ctx);
    if (ret < 0 && ret != AVERROR_EOF)
    {
        printf("Error occurred.\n");
        return -1;
    }
}

MPP_RET CMppEncoder::TestMppRun(cv::Mat img)
{
    MpiEncTestArgs *cmd = info->cmd;
    MpiEncTestData *p = &info->ctx;
    MppApi *mpi = p->mpi;
    MppCtx ctx = p->ctx;
    RK_U32 quiet = cmd->quiet;
    RK_S32 chn = info->chn;
    RK_U32 cap_num = 0;
    MPP_RET ret = MPP_OK;

    MppMeta meta = NULL;
    MppFrame frame = NULL;
    MppPacket packet = NULL;

    void *buf = mpp_buffer_get_ptr(p->frm_buf);
    RK_S32 cam_frm_idx = -1;
    MppBuffer cam_buf = NULL;
    RK_U32 eoi = 1;

    mpp_buffer_sync_begin(p->frm_buf);
    memcpy(buf, img.data, img.total() * img.elemSize());
    // p->frm_eos = 1;
    // mpp_log_q(quiet, "chn %d found last frame. feof %d\n", chn, feof(p->fp_input));

    mpp_buffer_sync_end(p->frm_buf);

    ret = mpp_frame_init(&frame);
    if (ret)
    {
        mpp_err_f("mpp_frame_init failed\n");
        goto RET;
    }

    mpp_frame_set_width(frame, p->width);
    mpp_frame_set_height(frame, p->height);
    mpp_frame_set_hor_stride(frame, p->hor_stride);
    mpp_frame_set_ver_stride(frame, p->ver_stride);
    mpp_frame_set_fmt(frame, p->fmt);
    mpp_frame_set_eos(frame, p->frm_eos);

    mpp_frame_set_buffer(frame, p->frm_buf);

    meta = mpp_frame_get_meta(frame);
    mpp_packet_init_with_buffer(&packet, p->pkt_buf);
    /* NOTE: It is important to clear output packet length!! */
    mpp_packet_set_length(packet, 0);

    /*
     * NOTE: in non-block mode the frame can be resent.
     * The default input timeout mode is block.
     *
     * User should release the input frame to meet the requirements of
     * resource creator must be the resource destroyer.
     */
    ret = mpi->encode_put_frame(ctx, frame);
    if (ret)
    {
        mpp_err("chn %d encode put frame failed\n", chn);
        mpp_frame_deinit(&frame);
        goto RET;
    }

    mpp_frame_deinit(&frame);

    do
    {
        ret = mpi->encode_get_packet(ctx, &packet);
        if (ret)
        {
            mpp_err("chn %d encode get packet failed\n", chn);
            goto RET;
        }

        mpp_assert(packet);

        if (packet)
        {
            // write packet to file here
            void *ptr = mpp_packet_get_pos(packet);
            size_t len = mpp_packet_get_length(packet);
            char log_buf[256];
            RK_S32 log_size = sizeof(log_buf) - 1;
            RK_S32 log_len = 0;

            p->pkt_eos = mpp_packet_get_eos(packet);

            write_frame((uint8_t *)ptr, len, (MppPacketImpl *)packet);

            mpp_log_q(quiet, "chn %d %s\n", chn, log_buf);

            mpp_packet_deinit(&packet);
            // fps_calc_inc(cmd->fps);

            p->stream_size += len;
            p->frame_count += eoi;

            if (p->pkt_eos)
            {
                mpp_log_q(quiet, "chn %d found last packet\n", chn);
                mpp_assert(p->frm_eos);
            }
        }
    } while (!eoi);
RET:

    return ret;
}

bool CMppEncoder::writeHeader(MpiEncTestData *mpp_enc_data, SpsHeader *sps_header)
{
    int ret;
    if (mpp_enc_data->type == MPP_VIDEO_CodingAVC)
    {
        MppPacket packet = NULL;
        ret = mpp_enc_data->mpi->control(mpp_enc_data->ctx, MPP_ENC_GET_EXTRA_INFO, &packet);
        if (ret)
        {
            printf("mpi control enc get extra info failed\n");
            return 1;
        }

        /* get and write sps/pps for H.264 */
        if (packet)
        {
            void *ptr = mpp_packet_get_pos(packet);
            size_t len = mpp_packet_get_length(packet);
            sps_header->data = (uint8_t *)malloc(len);
            sps_header->size = len;
            memcpy(sps_header->data, ptr, len);
            packet = NULL;
        }
    }
    return 1;
}

int CMppEncoder::write_frame(uint8_t *data, int size, MppPacketImpl *mpp_pkt)
{
    pkt->size = size;
    pkt->data = data;
    pkt->flags = 0x01;
    pkt->stream_index = out_stream->index;
    // std::cout << size << std::endl;
    pkt->duration = AV_TIME_BASE * 1 / 60000;
    pkt->dts = frame_index * pkt->duration;
    pkt->pts = pkt->dts;
    frame_index++;
    printf("%d\n", frame_index);
    // pkt->dts = mpp_pkt->dts;
    // pkt->pts = mpp_pkt->pts;

    if (av_write_frame(ofmt_ctx, pkt) < 0)
    {
        printf("Error muxing packet\n");
        return -1;
    }
    // if (av_interleaved_write_frame(ofmt_ctx, pkt) < 0)
    // {
    //     printf("Error muxing packet\n");
    //     return -1;
    // }
    return 0;
}

MPP_RET CMppEncoder::TestCtxInit()
{
    MpiEncTestArgs *cmd = info->cmd;
    MpiEncTestData *p = &info->ctx;
    MPP_RET ret = MPP_OK;

    // get paramter from cmd
    p->width = cmd->width;
    p->height = cmd->height;
    p->hor_stride = (cmd->hor_stride) ? (cmd->hor_stride) : (MPP_ALIGN(cmd->width, 16));
    p->ver_stride = (cmd->ver_stride) ? (cmd->ver_stride) : (MPP_ALIGN(cmd->height, 16));
    p->fmt = cmd->format;
    p->type = cmd->type;
    p->bps = cmd->bps_target;
    p->bps_min = cmd->bps_min;
    p->bps_max = cmd->bps_max;
    p->rc_mode = cmd->rc_mode;
    p->frame_num = cmd->frame_num;
    if (cmd->type == MPP_VIDEO_CodingMJPEG && p->frame_num == 0)
    {
        mpp_log("jpege default encode only one frame. Use -n [num] for rc case\n");
        p->frame_num = 1;
    }
    p->gop_mode = cmd->gop_mode;
    p->gop_len = cmd->gop_len;
    p->vi_len = cmd->vi_len;

    p->fps_in_flex = cmd->fps_in_flex;
    p->fps_in_den = cmd->fps_in_den;
    p->fps_in_num = cmd->fps_in_num;
    p->fps_out_flex = cmd->fps_out_flex;
    p->fps_out_den = cmd->fps_out_den;
    p->fps_out_num = cmd->fps_out_num;
    p->scene_mode = cmd->scene_mode;
    p->mdinfo_size = (MPP_VIDEO_CodingHEVC == cmd->type) ? (MPP_ALIGN(p->hor_stride, 32) >> 5) *
                                                               (MPP_ALIGN(p->ver_stride, 32) >> 5) * 16
                                                         : (MPP_ALIGN(p->hor_stride, 64) >> 6) *
                                                               (MPP_ALIGN(p->ver_stride, 16) >> 4) * 16;

    p->fp_input = fopen(cmd->file_input, "rb");
    if (NULL == p->fp_input)
    {
        mpp_err("failed to open input file %s\n", cmd->file_input);
        mpp_err("create default yuv image for test\n");
    }

    if (cmd->file_output)
    {
        p->fp_output = fopen(cmd->file_output, "w+b");
        if (NULL == p->fp_output)
        {
            mpp_err("failed to open output file %s\n", cmd->file_output);
            ret = MPP_ERR_OPEN_FILE;
        }
    }

    if (cmd->file_slt)
    {
        p->fp_verify = fopen(cmd->file_slt, "wt");
        if (!p->fp_verify)
            mpp_err("failed to open verify file %s\n", cmd->file_slt);
    }

    // update resource parameter
    switch (p->fmt & MPP_FRAME_FMT_MASK)
    {
    case MPP_FMT_YUV420SP:
    case MPP_FMT_YUV420P:
    {
        p->frame_size = MPP_ALIGN(p->hor_stride, 64) * MPP_ALIGN(p->ver_stride, 64) * 3 / 2;
    }
    break;

    case MPP_FMT_YUV422_YUYV:
    case MPP_FMT_YUV422_YVYU:
    case MPP_FMT_YUV422_UYVY:
    case MPP_FMT_YUV422_VYUY:
    case MPP_FMT_YUV422P:
    case MPP_FMT_YUV422SP:
    {
        p->frame_size = MPP_ALIGN(p->hor_stride, 64) * MPP_ALIGN(p->ver_stride, 64) * 2;
    }
    break;
    case MPP_FMT_YUV400:
    case MPP_FMT_RGB444:
    case MPP_FMT_BGR444:
    case MPP_FMT_RGB555:
    case MPP_FMT_BGR555:
    case MPP_FMT_RGB565:
    case MPP_FMT_BGR565:
    case MPP_FMT_RGB888:
    case MPP_FMT_BGR888:
    case MPP_FMT_RGB101010:
    case MPP_FMT_BGR101010:
    case MPP_FMT_ARGB8888:
    case MPP_FMT_ABGR8888:
    case MPP_FMT_BGRA8888:
    case MPP_FMT_RGBA8888:
    {
        p->frame_size = MPP_ALIGN(p->hor_stride, 64) * MPP_ALIGN(p->ver_stride, 64);
    }
    break;

    default:
    {
        p->frame_size = MPP_ALIGN(p->hor_stride, 64) * MPP_ALIGN(p->ver_stride, 64) * 4;
    }
    break;
    }

    if (MPP_FRAME_FMT_IS_FBC(p->fmt))
    {
        if ((p->fmt & MPP_FRAME_FBC_MASK) == MPP_FRAME_FBC_AFBC_V1)
            p->header_size = MPP_ALIGN(MPP_ALIGN(p->width, 16) * MPP_ALIGN(p->height, 16) / 16, SZ_4K);
        else
            p->header_size = MPP_ALIGN(p->width, 16) * MPP_ALIGN(p->height, 16) / 16;
    }
    else
    {
        p->header_size = 0;
    }

    return ret;
}

void CMppEncoder::TestCtxDeinit()
{
    MPP_FREE(checkcrc.sum);
    if (&(info->ctx))
    {
        if (info->ctx.ctx)
        {
            mpp_destroy(info->ctx.ctx);
            info->ctx.ctx = NULL;
        }

        if (info->ctx.cfg)
        {
            mpp_enc_cfg_deinit(info->ctx.cfg);
            info->ctx.cfg = NULL;
        }

        if (info->ctx.frm_buf)
        {
            mpp_buffer_put(info->ctx.frm_buf);
            info->ctx.frm_buf = NULL;
        }

        if (info->ctx.pkt_buf)
        {
            mpp_buffer_put(info->ctx.pkt_buf);
            info->ctx.pkt_buf = NULL;
        }

        if (info->ctx.md_info)
        {
            mpp_buffer_put(info->ctx.md_info);
            info->ctx.md_info = NULL;
        }

        if (info->ctx.osd_data.buf)
        {
            mpp_buffer_put(info->ctx.osd_data.buf);
            info->ctx.osd_data.buf = NULL;
        }

        if (info->ctx.buf_grp)
        {
            mpp_buffer_group_put(info->ctx.buf_grp);
            info->ctx.buf_grp = NULL;
        }

        if (info->ctx.roi_ctx)
        {
            mpp_enc_roi_deinit(info->ctx.roi_ctx);
            info->ctx.roi_ctx = NULL;
        }
        if (info->ctx.cam_ctx)
        {
            camera_source_deinit(info->ctx.cam_ctx);
            info->ctx.cam_ctx = NULL;
        }
        if (info->ctx.fp_input)
        {
            fclose(info->ctx.fp_input);
            info->ctx.fp_input = NULL;
        }
        if (info->ctx.fp_output)
        {
            fclose(info->ctx.fp_output);
            info->ctx.fp_output = NULL;
        }
        if (info->ctx.fp_verify)
        {
            fclose(info->ctx.fp_verify);
            info->ctx.fp_verify = NULL;
        }
    }
}

CMppEncoder::~CMppEncoder()
{
    MPP_RET ret = MPP_OK;
    ret = info->ctx.mpi->reset(info->ctx.ctx);
    if (ret)
    {
        mpp_err("mpi->reset failed\n");
        TestCtxDeinit();
    }

    // info->ret.elapsed_time = t_e - t_s;
    // info->ret.frame_count = info->ctx.frame_count;
    // info->ret.stream_size = info->ctx.stream_size;
    // info->ret.frame_rate = (float)info->ctx.frame_count * 1000000 / info->ret.elapsed_time;
    // info->ret.bit_rate = (info->ctx.stream_size * 8 * (info->ctx.fps_out_num / info->ctx.fps_out_den)) / info->ctx.frame_count;
    // info->ret.delay = info->ctx.first_pkt - info->ctx.first_frm;
    TestCtxDeinit();
    mpi_enc_test_cmd_put(info->cmd);
    MPP_FREE(info);
}
