#include "service.h"

#include <grpcpp/grpcpp.h>

namespace talos {
namespace icd {
namespace server {

TalosOperatorService::TalosOperatorService(std::ostream& out) : out_(out) {}

TalosOperatorService::~TalosOperatorService() = default;

::grpc::Status TalosOperatorService::Handshake(::grpc::ServerContext* /*context*/, const ::icd::v1::HandshakeRequest* /*request*/, ::icd::v1::HandshakeResponse* response) {
	response->mutable_status()->set_return_code(0);
	response->set_major_version(1);
	response->set_minor_version(0);
	return ::grpc::Status::OK;
}

::grpc::Status TalosOperatorService::PolarPanDiscrete(::grpc::ServerContext* /*context*/, const ::icd::v1::PolarPanDiscreteRequest* /*request*/, ::google::protobuf::UInt32Value* response) {
	response->set_value(0);
	return ::grpc::Status::OK;
}

::grpc::Status TalosOperatorService::Home(::grpc::ServerContext* /*context*/, const ::icd::v1::HomeRequest* /*request*/, ::google::protobuf::UInt32Value* response) {
	response->set_value(0);
	return ::grpc::Status::OK;
}

::grpc::Status TalosOperatorService::PolarPanContinuousStart(::grpc::ServerContext* /*context*/, const ::icd::v1::PolarPanContinuousStartRequest* /*request*/, ::google::protobuf::UInt32Value* response) {
	response->set_value(0);
	return ::grpc::Status::OK;
}

::grpc::Status TalosOperatorService::PolarPanContinuousStop(::grpc::ServerContext* /*context*/, const ::google::protobuf::Empty* /*request*/, ::google::protobuf::UInt32Value* response) {
	response->set_value(0);
	return ::grpc::Status::OK;
}

::grpc::Status TalosOperatorService::CartesianMoveDiscrete(::grpc::ServerContext* /*context*/, const ::icd::v1::CartesianMoveDiscreteRequest* /*request*/, ::google::protobuf::UInt32Value* response) {
	response->set_value(0);
	return ::grpc::Status::OK;
}

::grpc::Status TalosOperatorService::CartesianMoveContinuousStart(::grpc::ServerContext* /*context*/, const ::icd::v1::CartesianMoveContinuousStartRequest* /*request*/, ::google::protobuf::UInt32Value* response) {
	response->set_value(0);
	return ::grpc::Status::OK;
}

::grpc::Status TalosOperatorService::CartesianMoveContinuousStop(::grpc::ServerContext* /*context*/, const ::google::protobuf::Empty* /*request*/, ::google::protobuf::UInt32Value* response) {
	response->set_value(0);
	return ::grpc::Status::OK;
}

::grpc::Status TalosOperatorService::ExecuteHardwareOperation(::grpc::ServerContext* /*context*/, const ::icd::v1::ExecuteHardwareOperationRequest* request, ::icd::v1::ExecuteHardwareOperationResponse* response) {
	response->mutable_status()->set_return_code(0);
	response->set_subcommand_id(request->subcommand_id());
	response->clear_payload();
	return ::grpc::Status::OK;
}

::grpc::Status TalosOperatorService::GetSpeed(::grpc::ServerContext* /*context*/, const ::google::protobuf::Empty* /*request*/, ::icd::v1::GetSpeedResponse* response) {
	response->mutable_status()->set_return_code(0);
	response->set_speed(0);
	return ::grpc::Status::OK;
}

::grpc::Status TalosOperatorService::SetSpeed(::grpc::ServerContext* /*context*/, const ::icd::v1::SetSpeedRequest* /*request*/, ::google::protobuf::UInt32Value* response) {
	response->set_value(0);
	return ::grpc::Status::OK;
}

::grpc::Status TalosOperatorService::SavePosition(::grpc::ServerContext* /*context*/, const ::icd::v1::SavePositionRequest* /*request*/, ::google::protobuf::UInt32Value* response) {
	response->set_value(0);
	return ::grpc::Status::OK;
}

::grpc::Status TalosOperatorService::DeletePosition(::grpc::ServerContext* /*context*/, const ::icd::v1::DeletePositionRequest* /*request*/, ::google::protobuf::UInt32Value* response) {
	response->set_value(0);
	return ::grpc::Status::OK;
}

::grpc::Status TalosOperatorService::GoToPosition(::grpc::ServerContext* /*context*/, const ::icd::v1::GoToPositionRequest* /*request*/, ::google::protobuf::UInt32Value* response) {
	response->set_value(0);
	return ::grpc::Status::OK;
}

::grpc::Status TalosOperatorService::SetPolarPosition(::grpc::ServerContext* /*context*/, const ::icd::v1::SetPolarPositionRequest* /*request*/, ::google::protobuf::UInt32Value* response) {
	response->set_value(0);
	return ::grpc::Status::OK;
}

::grpc::Status TalosOperatorService::GetPolarPosition(::grpc::ServerContext* /*context*/, const ::icd::v1::GetPolarPositionRequest* /*request*/, ::icd::v1::GetPolarPositionResponse* response) {
	response->mutable_status()->set_return_code(0);
	response->set_delta_tenths_deg(0);
	response->set_azimuth_tenths_deg(0);
	response->set_radius_tenths(0);
	return ::grpc::Status::OK;
}

::grpc::Status TalosOperatorService::SetCartesianPosition(::grpc::ServerContext* /*context*/, const ::icd::v1::SetCartesianPositionRequest* /*request*/, ::google::protobuf::UInt32Value* response) {
	response->set_value(0);
	return ::grpc::Status::OK;
}

::grpc::Status TalosOperatorService::GetCartesianPosition(::grpc::ServerContext* /*context*/, const ::icd::v1::GetCartesianPositionRequest* /*request*/, ::icd::v1::GetCartesianPositionResponse* response) {
	response->mutable_status()->set_return_code(0);
	response->set_x_tenths_mm(0);
	response->set_y_tenths_mm(0);
	response->set_z_tenths_mm(0);
	return ::grpc::Status::OK;
}

} // namespace server
} // namespace icd
} // namespace talos
