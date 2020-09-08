#include <glog/logging.h>
#include <stdio.h>

//编译指令：g++ test_glog.cpp -lglog -o test_glog
int main(int argc, char** argv)
{
    //缓存的最大时长，超时会写入文件
    FLAGS_logbufsecs = 60;
    //单个日志文件最大，单位M
    FLAGS_max_log_size = 10;
    //设置为true，就不会写日志文件了
    FLAGS_logtostderr = false;
    // 设置日志保存的路径,out_log目录需手动创建
    FLAGS_log_dir = "./out_log";

    google::InitGoogleLogging("test_glog");

    for(int i = 0; i < 1000; ++i)
    {
        LOG(INFO) << i;
    }

    google::ShutdownGoogleLogging();
}