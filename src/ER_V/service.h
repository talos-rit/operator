#pragma once

#include <ostream>

#include "icd/v1/icd.grpc.pb.h"

namespace talos {
namespace icd {
namespace server {

class TalosOperatorService : public ::icd::v1::TalosOperatorService::Service {
public:
    explicit TalosOperatorService(std::ostream& out);
    ~TalosOperatorService() override;

    ::grpc::Status Handshake(::grpc::ServerContext* context, const ::icd::v1::HandshakeRequest* request, ::icd::v1::HandshakeResponse* response) override;
    ::grpc::Status PolarPanDiscrete(::grpc::ServerContext* context, const ::icd::v1::PolarPanDiscreteRequest* request, ::google::protobuf::UInt32Value* response) override;
    ::grpc::Status Home(::grpc::ServerContext* context, const ::icd::v1::HomeRequest* request, ::google::protobuf::UInt32Value* response) override;
    ::grpc::Status PolarPanContinuousStart(::grpc::ServerContext* context, const ::icd::v1::PolarPanContinuousStartRequest* request, ::google::protobuf::UInt32Value* response) override;
    ::grpc::Status PolarPanContinuousStop(::grpc::ServerContext* context, const ::google::protobuf::Empty* request, ::google::protobuf::UInt32Value* response) override;
    ::grpc::Status CartesianMoveDiscrete(::grpc::ServerContext* context, const ::icd::v1::CartesianMoveDiscreteRequest* request, ::google::protobuf::UInt32Value* response) override;
    ::grpc::Status CartesianMoveContinuousStart(::grpc::ServerContext* context, const ::icd::v1::CartesianMoveContinuousStartRequest* request, ::google::protobuf::UInt32Value* response) override;
    ::grpc::Status CartesianMoveContinuousStop(::grpc::ServerContext* context, const ::google::protobuf::Empty* request, ::google::protobuf::UInt32Value* response) override;
    ::grpc::Status ExecuteHardwareOperation(::grpc::ServerContext* context, const ::icd::v1::ExecuteHardwareOperationRequest* request, ::icd::v1::ExecuteHardwareOperationResponse* response) override;
    ::grpc::Status GetSpeed(::grpc::ServerContext* context, const ::google::protobuf::Empty* request, ::icd::v1::GetSpeedResponse* response) override;
    ::grpc::Status SetSpeed(::grpc::ServerContext* context, const ::icd::v1::SetSpeedRequest* request, ::google::protobuf::UInt32Value* response) override;
    ::grpc::Status SavePosition(::grpc::ServerContext* context, const ::icd::v1::SavePositionRequest* request, ::google::protobuf::UInt32Value* response) override;
    ::grpc::Status DeletePosition(::grpc::ServerContext* context, const ::icd::v1::DeletePositionRequest* request, ::google::protobuf::UInt32Value* response) override;
    ::grpc::Status GoToPosition(::grpc::ServerContext* context, const ::icd::v1::GoToPositionRequest* request, ::google::protobuf::UInt32Value* response) override;
    ::grpc::Status SetPolarPosition(::grpc::ServerContext* context, const ::icd::v1::SetPolarPositionRequest* request, ::google::protobuf::UInt32Value* response) override;
    ::grpc::Status GetPolarPosition(::grpc::ServerContext* context, const ::icd::v1::GetPolarPositionRequest* request, ::icd::v1::GetPolarPositionResponse* response) override;
    ::grpc::Status SetCartesianPosition(::grpc::ServerContext* context, const ::icd::v1::SetCartesianPositionRequest* request, ::google::protobuf::UInt32Value* response) override;
    ::grpc::Status GetCartesianPosition(::grpc::ServerContext* context, const ::icd::v1::GetCartesianPositionRequest* request, ::icd::v1::GetCartesianPositionResponse* response) override;

private:
    std::ostream& out_;
};

} // namespace server
} // namespace icd
} // namespace talos
