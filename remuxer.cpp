#include "remuxer.h"
#include <QProgressBar>
#include <QMessageBox>
#include <QStringList>
#include <QDebug>

#define USE_DEBUG 0
ReMuxer::ReMuxer(QObject *parent):
    QThread(parent)
{
}

void ReMuxer::setInputFile(QString filename)
{
    In_File = filename;

}

void ReMuxer::setOutFile(QString filename)
{
    Out_File = filename;
}

int ReMuxer::remux()
{
    if(In_File.isEmpty() || In_File.isNull()
            || Out_File.isEmpty() || Out_File.isNull())
        return -1;

    AVOutputFormat *ofmt = NULL;

    AVFormatContext *ifmt_ctx = NULL, *ofmt_ctx = NULL;
    AVPacket pkt;

    int ret,i;
    int frame_index = 0;

    // --[2]初始化输入文件的格式上下文
    //将文件头信息复制给avformatContext
    if((ret = avformat_open_input(&ifmt_ctx,In_File.toLocal8Bit(),NULL,NULL)) < 0)
    {
        qDebug()<<"could not open input file!";
        goto end;
    }
    //将流的参数信息复制给ifmt_ctx->streams
    if((ret = avformat_find_stream_info(ifmt_ctx,NULL)) < 0)
    {
        qDebug()<<"can not find a stream!\n";
        goto end;
    }

#if USE_DEBUG
    //cout<<"----------------------Input file Information-----------------"<<endl;
    av_dump_format(ifmt_ctx,0,In_File.toLocal8Bit(),0);
    // cout<<"-------------------------------------------------------------"<<endl;
#endif
    //[2]

    //估算进度条最大值
    int frame_num = (((ifmt_ctx->streams[0]->avg_frame_rate.num/ifmt_ctx->streams[0]->avg_frame_rate.den) * double(ifmt_ctx->duration/1000000))/
            ifmt_ctx->streams[0]->codec_info_nb_frames)*(ifmt_ctx->streams[0]->codec_info_nb_frames
            +ifmt_ctx->streams[1]->codec_info_nb_frames);
    emit sendProgressMaxValue(frame_num);


    //[3] --初始化输出文件的格式上下文
    avformat_alloc_output_context2(&ofmt_ctx,NULL,NULL,Out_File.toLocal8Bit());
    if(!ofmt_ctx)
    {
        qDebug()<<"could not creat output context";
        goto end;
    }
    ofmt = ofmt_ctx->oformat;
    //[3]

    //[4] --将输入文件内容复制到输出文件中
    for( i =0;i<ifmt_ctx->nb_streams;++i)
    {
        //获取输入流
        AVStream *in_stream = ifmt_ctx->streams[i];
        //创建输出流
        AVStream *out_stream = avformat_new_stream(ofmt_ctx,in_stream->codec->codec);
        if(!out_stream)
        {
            qDebug()<<"failed allocating output stram";
            ret = AVERROR_UNKNOWN;
            goto end;
        }

        //复制输入流的AVCodeContext到输出流的AVCodeContext
        out_stream->time_base.num = in_stream->time_base.num;
        out_stream->time_base.den = in_stream->time_base.den;
        out_stream->avg_frame_rate = in_stream->avg_frame_rate;
        if(avcodec_copy_context(out_stream->codec,in_stream->codec) < 0)
        {
            qDebug()<<"failed to copy CodecContext!";
            goto end;
        }

        out_stream->codec->codec_tag = 0;
        if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
            out_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;

    }
    //[4]

#if USE_DEBUG
    //cout<<"----------------------Output file Information-----------------"<<endl;
    av_dump_format(ofmt_ctx,0,Out_File.toLocal8Bit(),1);
    //cout<<"-------------------------------------------------------------"<<endl;
#endif

    //打开输出文件
    if(!(ofmt->flags & AVFMT_NOFILE))
    {
        ret = avio_open(&ofmt_ctx->pb,Out_File.toLocal8Bit(),AVIO_FLAG_WRITE);
        if (ret < 0) {
            qDebug()<<"Could not open output file: "<<Out_File.toLocal8Bit();
            goto end;
        }
    }

    //写头文件
    if(avformat_write_header(ofmt_ctx,NULL) < 0)
    {
        qDebug()<<"Error occurred when opening output file";
        goto end;
    }

    AVBitStreamFilterContext * vbsf = av_bitstream_filter_init("h264_mp4toannexb");

    //写入文件内容
    while(1)
    {
        AVStream *in_stream,*out_stream;
        ret = av_read_frame(ifmt_ctx,&pkt);
        if(ret < 0)
            break;
        in_stream  = ifmt_ctx->streams[pkt.stream_index];
        out_stream = ofmt_ctx->streams[pkt.stream_index];

        //qDebug()<<pkt.pts<<" "<<pkt.dts;
        QStringList inExt = QString(ifmt_ctx->iformat->extensions).split(",");
        QStringList outExt = QString(ofmt_ctx->oformat->extensions).split(",");
        //qDebug()<<"inExt:"<<inExt;
        //qDebug()<<"outExt:"<<outExt;
        if(!((inExt.contains("mp4") || inExt.contains("mkv") || inExt.contains("flv")) &&
             (outExt.contains("mp4") || outExt.contains("mkv")|| outExt.contains("flv"))))
        {qDebug()<<"run";
            if (pkt.stream_index == AVMEDIA_TYPE_VIDEO &&
                    in_stream->codecpar->codec_id == AV_CODEC_ID_H264) {
                AVPacket fpkt = pkt;
                int a = av_bitstream_filter_filter(vbsf,
                                                   out_stream->codec, NULL, &fpkt.data, &fpkt.size,
                                                   pkt.data, pkt.size, pkt.flags & AV_PKT_FLAG_KEY);
                pkt.data = fpkt.data;
                pkt.size = fpkt.size;
            }
        }

        //转换PTS/DTS
        /* copy packet */
        pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
        pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
        pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
        pkt.pos = -1;

        //写入
        if(av_interleaved_write_frame(ofmt_ctx,&pkt) < 0)
        {
            qDebug()<<"Error muxing packet";
            break;
        }
#if USE_DEBUG
        qDebug()<<"Write "<<frame_index<<" frames to output file";
#endif
        av_free_packet(&pkt);
        frame_index++;
        emit sendProgress(frame_index);
    }

    //写文件尾
    av_write_trailer(ofmt_ctx);

end:
    avformat_close_input(&ifmt_ctx);
    if (ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE))
        avio_close(ofmt_ctx->pb);
    avformat_free_context(ofmt_ctx);
    av_bitstream_filter_close(vbsf);
    if (ret < 0 && ret != AVERROR_EOF) {
        qDebug()<<"Error occurred.";
        return -1;
    }

    return 1;
}

void ReMuxer::run()
{
    remux();
}
