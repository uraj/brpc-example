#include <gflags/gflags.h>
#include <butil/logging.h>
#include <butil/time.h>
#include <brpc/channel.h>
#include <fstream>
#include <algorithm>
#include <iterator>
#include "compserv.pb.h"

DEFINE_string(ir, "", "LLVM IR file to be processed by remote server");
DEFINE_string(server, "0.0.0.0:8010", "IP Address of server");
DEFINE_string(load_balancer, "", "The algorithm for load balancing");
DEFINE_int32(timeout_ms, 60000, "RPC timeout in milliseconds");
DEFINE_int32(max_retry, 3, "Max retries(not including the first RPC)"); 

DEFINE_bool(subinst, false, "");
DEFINE_bool(bcf, false, "");
DEFINE_bool(fla, false, "");
DEFINE_bool(gi, false, "");
DEFINE_bool(zs, false, "");
DEFINE_bool(fepi, false, "");

int main(int argc, char* argv[]) {
    google::ParseCommandLineFlags(&argc, &argv, true);
    
    brpc::Channel channel;
    
    brpc::ChannelOptions options;
    options.timeout_ms = FLAGS_timeout_ms;
    options.max_retry = FLAGS_max_retry;
    if (channel.Init(FLAGS_server.c_str(), FLAGS_load_balancer.c_str(), &options) != 0) {
        LOG(ERROR) << "Fail to initialize channel";
        return -1;
    }

    CompileService_Stub stub(&channel);

    CompileRequest request;
    CompileResponse response;
    brpc::Controller cntl;

    std::ifstream input(FLAGS_ir, std::ios::binary);
    if (!input.is_open()) {
        LOG(ERROR) << "Fail to open input ir file";
        return -1;
    }

    std::ostringstream content_stream;
    content_stream << input.rdbuf();

    request.set_allocated_ir(new std::string(content_stream.str()));

    if (FLAGS_subinst) request.add_trans(CompileRequest::SUBINST);
    if (FLAGS_bcf)     request.add_trans(CompileRequest::BCF);
    if (FLAGS_fla)     request.add_trans(CompileRequest::FLA);
    if (FLAGS_gi)      request.add_trans(CompileRequest::GI);
    if (FLAGS_zs)      request.add_trans(CompileRequest::ZS);
    if (FLAGS_fepi)    request.add_trans(CompileRequest::FEPI);

    stub.compile(&cntl, &request, &response, NULL);
    if (!cntl.Failed()) {
        LOG(INFO) << "Received response from " << cntl.remote_side()
            << " to " << cntl.local_side()
            << " latency=" << cntl.latency_us() << "us";
    } else {
        LOG(ERROR) << cntl.ErrorText();
        return -1;
    }

    if (!response.has_ir()) {
        LOG(ERROR) << CompileResponse::ErrorCode_Name(response.code());
        return -1;
    }

    std::ofstream output(FLAGS_ir + ".trans", std::ios::binary);
    if (!output.is_open()) {
        LOG(ERROR) << "Cannot open output ir file";
        return -1;
    }

    auto &ir = response.ir();
    output.write(ir.data(), ir.size());

    LOG(INFO) << "Client is going to quit";
    return 0;
}
