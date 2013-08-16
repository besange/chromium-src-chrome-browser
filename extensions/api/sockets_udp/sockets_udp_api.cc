// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/extensions/api/sockets_udp/sockets_udp_api.h"

#include "chrome/browser/extensions/api/socket/udp_socket.h"
#include "chrome/browser/extensions/api/sockets_udp/udp_socket_event_dispatcher.h"
#include "chrome/common/extensions/permissions/permissions_data.h"
#include "chrome/common/extensions/permissions/socket_permission.h"
#include "content/public/common/socket_permission_request.h"
#include "net/base/net_errors.h"

namespace extensions {
namespace api {

using content::SocketPermissionRequest;

const char kSocketNotFoundError[] = "Socket not found";
const char kPermissionError[] = "App does not have permission";
const char kWildcardAddress[] = "*";
const int kWildcardPort = 0;

UDPSocketAsyncApiFunction::~UDPSocketAsyncApiFunction() {}

scoped_ptr<SocketResourceManagerInterface>
    UDPSocketAsyncApiFunction::CreateSocketResourceManager() {
  return scoped_ptr<SocketResourceManagerInterface>(
      new SocketResourceManager<ResumableUDPSocket>()).Pass();
}

ResumableUDPSocket* UDPSocketAsyncApiFunction::GetUdpSocket(int socket_id) {
  return static_cast<ResumableUDPSocket*>(GetSocket(socket_id));
}

UDPSocketExtensionWithDnsLookupFunction::
    ~UDPSocketExtensionWithDnsLookupFunction() {}

scoped_ptr<SocketResourceManagerInterface>
    UDPSocketExtensionWithDnsLookupFunction::CreateSocketResourceManager() {
  return scoped_ptr<SocketResourceManagerInterface>(
      new SocketResourceManager<ResumableUDPSocket>()).Pass();
}

ResumableUDPSocket* UDPSocketExtensionWithDnsLookupFunction::GetUdpSocket(
    int socket_id) {
  return static_cast<ResumableUDPSocket*>(GetSocket(socket_id));
}

linked_ptr<sockets_udp::SocketInfo> CreateSocketInfo(
    int socket_id,
    ResumableUDPSocket* socket) {
  linked_ptr<sockets_udp::SocketInfo> socket_info(
      new sockets_udp::SocketInfo());
  // This represents what we know about the socket, and does not call through
  // to the system.
  socket_info->socket_id = socket_id;
  if (!socket->name().empty()) {
    socket_info->name.reset(new std::string(socket->name()));
  }
  socket_info->persistent = socket->persistent();
  if (socket->buffer_size() > 0) {
    socket_info->buffer_size.reset(new int(socket->buffer_size()));
  }

  // Grab the local address as known by the OS.
  net::IPEndPoint localAddress;
  if (socket->GetLocalAddress(&localAddress)) {
    socket_info->local_address.reset(
        new std::string(localAddress.ToStringWithoutPort()));
    socket_info->local_port.reset(new int(localAddress.port()));
  }

  return socket_info;
}

void SetSocketProperties(ResumableUDPSocket* socket,
                         sockets_udp::SocketProperties* properties) {
  if (properties->name.get()) {
    socket->set_name(*properties->name.get());
  }
  if (properties->persistent.get()) {
    socket->set_persistent(*properties->persistent.get());
  }
  if (properties->buffer_size.get()) {
    socket->set_buffer_size(*properties->buffer_size.get());
  }
}

SocketsUdpCreateFunction::SocketsUdpCreateFunction() {}

SocketsUdpCreateFunction::~SocketsUdpCreateFunction() {}

bool SocketsUdpCreateFunction::Prepare() {
  params_ = sockets_udp::Create::Params::Create(*args_);
  EXTENSION_FUNCTION_VALIDATE(params_.get());
  return true;
}

void SocketsUdpCreateFunction::Work() {
  ResumableUDPSocket* socket = new ResumableUDPSocket(extension_->id());

  sockets_udp::SocketProperties* properties = params_.get()->properties.get();
  if (properties) {
    SetSocketProperties(socket, properties);
  }

  sockets_udp::CreateInfo create_info;
  create_info.socket_id = AddSocket(socket);
  results_ = sockets_udp::Create::Results::Create(create_info);
}

SocketsUdpUpdateFunction::SocketsUdpUpdateFunction() {}

SocketsUdpUpdateFunction::~SocketsUdpUpdateFunction() {}

bool SocketsUdpUpdateFunction::Prepare() {
  params_ = sockets_udp::Update::Params::Create(*args_);
  EXTENSION_FUNCTION_VALIDATE(params_.get());
  return true;
}

void SocketsUdpUpdateFunction::Work() {
  ResumableUDPSocket* socket = GetUdpSocket(params_->socket_id);
  if (!socket) {
    error_ = kSocketNotFoundError;
    return;
  }

  SetSocketProperties(socket, &params_.get()->properties);
  results_ = sockets_udp::Update::Results::Create();
}

SocketsUdpBindFunction::SocketsUdpBindFunction() {}

SocketsUdpBindFunction::~SocketsUdpBindFunction() {}

bool SocketsUdpBindFunction::Prepare() {
  params_ = sockets_udp::Bind::Params::Create(*args_);
  EXTENSION_FUNCTION_VALIDATE(params_.get());
  return true;
}

void SocketsUdpBindFunction::Work() {
  ResumableUDPSocket* socket = GetUdpSocket(params_->socket_id);
  if (!socket) {
    error_ = kSocketNotFoundError;
    return;
  }

  SocketPermission::CheckParam param(
      SocketPermissionRequest::UDP_BIND, params_->address, params_->port);
  if (!PermissionsData::CheckAPIPermissionWithParam(
          GetExtension(),
          APIPermission::kSocket,
          &param)) {
    error_ = kPermissionError;
    return;
  }

  int net_result = socket->Bind(params_->address, params_->port);
  if (net_result == net::OK) {
    UDPSocketEventDispatcher::Get(profile())->OnSocketBind(extension_->id(),
                                                           params_->socket_id);
  }

  if (net_result != net::OK)
    error_ = net::ErrorToString(net_result);
  results_ = sockets_udp::Bind::Results::Create(net_result);
}

SocketsUdpSendFunction::SocketsUdpSendFunction()
    : io_buffer_size_(0) {}

SocketsUdpSendFunction::~SocketsUdpSendFunction() {}

bool SocketsUdpSendFunction::Prepare() {
  params_ = sockets_udp::Send::Params::Create(*args_);
  EXTENSION_FUNCTION_VALIDATE(params_.get());
  io_buffer_size_ = params_->data.size();
  io_buffer_ = new net::WrappedIOBuffer(params_->data.data());
  return true;
}

void SocketsUdpSendFunction::AsyncWorkStart() {
  ResumableUDPSocket* socket = GetUdpSocket(params_->socket_id);
  if (!socket) {
    error_ = kSocketNotFoundError;
    AsyncWorkCompleted();
    return;
  }

  SocketPermission::CheckParam param(
      SocketPermissionRequest::UDP_SEND_TO,
      params_->address,
      params_->port);
  if (!PermissionsData::CheckAPIPermissionWithParam(
          GetExtension(),
          APIPermission::kSocket,
          &param)) {
    error_ = kPermissionError;
    AsyncWorkCompleted();
    return;
  }

  StartDnsLookup(params_->address);
}

void SocketsUdpSendFunction::AfterDnsLookup(int lookup_result) {
  if (lookup_result == net::OK) {
    StartSendTo();
  } else {
    SetSendResult(lookup_result, -1);
  }
}

void SocketsUdpSendFunction::StartSendTo() {
  ResumableUDPSocket* socket = GetUdpSocket(params_->socket_id);
  if (!socket) {
    error_ = kSocketNotFoundError;
    AsyncWorkCompleted();
    return;
  }

  socket->SendTo(io_buffer_, io_buffer_size_, resolved_address_, params_->port,
                  base::Bind(&SocketsUdpSendFunction::OnCompleted, this));
}

void SocketsUdpSendFunction::OnCompleted(int net_result) {
  if (net_result >= net::OK) {
    SetSendResult(net::OK, net_result);
  } else {
    SetSendResult(net_result, -1);
  }
}

void SocketsUdpSendFunction::SetSendResult(int net_result, int bytes_written) {
  CHECK(net_result <= net::OK) << "Network status code must be < 0";

  sockets_udp::SendInfo send_info;
  send_info.result = net_result;
  if (net_result == net::OK) {
    send_info.bytes_written.reset(new int(bytes_written));
  }

  if (net_result != net::OK)
    error_ = net::ErrorToString(net_result);
  results_ = sockets_udp::Send::Results::Create(send_info);
  AsyncWorkCompleted();
}

SocketsUdpCloseFunction::SocketsUdpCloseFunction() {}

SocketsUdpCloseFunction::~SocketsUdpCloseFunction() {}

bool SocketsUdpCloseFunction::Prepare() {
  params_ = sockets_udp::Close::Params::Create(*args_);
  EXTENSION_FUNCTION_VALIDATE(params_.get());
  return true;
}

void SocketsUdpCloseFunction::Work() {
  ResumableUDPSocket* socket = GetUdpSocket(params_->socket_id);
  if (!socket) {
    error_ = kSocketNotFoundError;
    return;
  }

  RemoveSocket(params_->socket_id);
  results_ = sockets_udp::Close::Results::Create();
}

SocketsUdpGetInfoFunction::SocketsUdpGetInfoFunction() {}

SocketsUdpGetInfoFunction::~SocketsUdpGetInfoFunction() {}

bool SocketsUdpGetInfoFunction::Prepare() {
  params_ = sockets_udp::GetInfo::Params::Create(*args_);
  EXTENSION_FUNCTION_VALIDATE(params_.get());
  return true;
}

void SocketsUdpGetInfoFunction::Work() {
  ResumableUDPSocket* socket = GetUdpSocket(params_->socket_id);
  if (!socket) {
    error_ = kSocketNotFoundError;
    return;
  }

  linked_ptr<sockets_udp::SocketInfo> socket_info =
      CreateSocketInfo(params_->socket_id, socket);
  results_ = sockets_udp::GetInfo::Results::Create(*socket_info);
}

SocketsUdpGetSocketsFunction::SocketsUdpGetSocketsFunction() {}

SocketsUdpGetSocketsFunction::~SocketsUdpGetSocketsFunction() {}

bool SocketsUdpGetSocketsFunction::Prepare() {
  return true;
}

void SocketsUdpGetSocketsFunction::Work() {
  std::vector<linked_ptr<sockets_udp::SocketInfo> > socket_infos;
  base::hash_set<int>* resource_ids = GetSocketIds();
  if (resource_ids != NULL) {
    for (base::hash_set<int>::iterator it = resource_ids->begin();
             it != resource_ids->end(); ++it) {
      int socket_id = *it;
      ResumableUDPSocket* socket = GetUdpSocket(socket_id);
      if (socket) {
        socket_infos.push_back(CreateSocketInfo(socket_id, socket));
      }
    }
  }
  results_ = sockets_udp::GetSockets::Results::Create(socket_infos);
}

SocketsUdpJoinGroupFunction::SocketsUdpJoinGroupFunction() {}

SocketsUdpJoinGroupFunction::~SocketsUdpJoinGroupFunction() {}

bool SocketsUdpJoinGroupFunction::Prepare() {
  params_ = sockets_udp::JoinGroup::Params::Create(*args_);
  EXTENSION_FUNCTION_VALIDATE(params_.get());
  return true;
}

void SocketsUdpJoinGroupFunction::Work() {
  ResumableUDPSocket* socket = GetUdpSocket(params_->socket_id);
  if (!socket) {
    error_ = kSocketNotFoundError;
    return;
  }

  SocketPermission::CheckParam param(
      SocketPermissionRequest::UDP_MULTICAST_MEMBERSHIP,
      kWildcardAddress,
      kWildcardPort);

  if (!PermissionsData::CheckAPIPermissionWithParam(
          GetExtension(), APIPermission::kSocket, &param)) {
    error_ = kPermissionError;
    return;
  }

  int net_result = socket->JoinGroup(params_->address);
  if (net_result != net::OK)
    error_ = net::ErrorToString(net_result);
  results_ = sockets_udp::JoinGroup::Results::Create(net_result);
}

SocketsUdpLeaveGroupFunction::SocketsUdpLeaveGroupFunction() {}

SocketsUdpLeaveGroupFunction::~SocketsUdpLeaveGroupFunction() {}

bool SocketsUdpLeaveGroupFunction::Prepare() {
  params_ = api::sockets_udp::LeaveGroup::Params::Create(*args_);
  EXTENSION_FUNCTION_VALIDATE(params_.get());
  return true;
}

void SocketsUdpLeaveGroupFunction::Work() {
  ResumableUDPSocket* socket = GetUdpSocket(params_->socket_id);
  if (!socket) {
    error_ = kSocketNotFoundError;
    return;
  }

  SocketPermission::CheckParam param(
      SocketPermissionRequest::UDP_MULTICAST_MEMBERSHIP,
      kWildcardAddress,
      kWildcardPort);
  if (!PermissionsData::CheckAPIPermissionWithParam(GetExtension(),
                                                    APIPermission::kSocket,
                                                    &param)) {
    error_ = kPermissionError;
    return;
  }

  int net_result = socket->LeaveGroup(params_->address);
  if (net_result != net::OK)
    error_ = net::ErrorToString(net_result);
  results_ = sockets_udp::LeaveGroup::Results::Create(net_result);
}

SocketsUdpSetMulticastTimeToLiveFunction::
    SocketsUdpSetMulticastTimeToLiveFunction() {}

SocketsUdpSetMulticastTimeToLiveFunction::
    ~SocketsUdpSetMulticastTimeToLiveFunction() {}

bool SocketsUdpSetMulticastTimeToLiveFunction::Prepare() {
  params_ = api::sockets_udp::SetMulticastTimeToLive::Params::Create(*args_);
  EXTENSION_FUNCTION_VALIDATE(params_.get());
  return true;
}

void SocketsUdpSetMulticastTimeToLiveFunction::Work() {
  ResumableUDPSocket* socket = GetUdpSocket(params_->socket_id);
  if (!socket) {
    error_ = kSocketNotFoundError;
    return;
  }

  int net_result = socket->SetMulticastTimeToLive(params_->ttl);
  if (net_result != net::OK)
    error_ = net::ErrorToString(net_result);
  results_ = sockets_udp::SetMulticastTimeToLive::Results::Create(net_result);
}

SocketsUdpSetMulticastLoopbackModeFunction::
    SocketsUdpSetMulticastLoopbackModeFunction() {}

SocketsUdpSetMulticastLoopbackModeFunction::
  ~SocketsUdpSetMulticastLoopbackModeFunction() {}

bool SocketsUdpSetMulticastLoopbackModeFunction::Prepare() {
  params_ = api::sockets_udp::SetMulticastLoopbackMode::Params::Create(*args_);
  EXTENSION_FUNCTION_VALIDATE(params_.get());
  return true;
}

void SocketsUdpSetMulticastLoopbackModeFunction::Work() {
  ResumableUDPSocket* socket = GetUdpSocket(params_->socket_id);
  if (!socket) {
    error_ = kSocketNotFoundError;
    return;
  }

  int net_result = socket->SetMulticastLoopbackMode(params_->enabled);
  if (net_result != net::OK)
    error_ = net::ErrorToString(net_result);
  results_ = sockets_udp::SetMulticastLoopbackMode::Results::Create(net_result);
}

SocketsUdpGetJoinedGroupsFunction::SocketsUdpGetJoinedGroupsFunction() {}

SocketsUdpGetJoinedGroupsFunction::~SocketsUdpGetJoinedGroupsFunction() {}

bool SocketsUdpGetJoinedGroupsFunction::Prepare() {
  params_ = api::sockets_udp::GetJoinedGroups::Params::Create(*args_);
  EXTENSION_FUNCTION_VALIDATE(params_.get());
  return true;
}

void SocketsUdpGetJoinedGroupsFunction::Work() {
  ResumableUDPSocket* socket = GetUdpSocket(params_->socket_id);
  if (!socket) {
    error_ = kSocketNotFoundError;
    return;
  }

  SocketPermission::CheckParam param(
      SocketPermissionRequest::UDP_MULTICAST_MEMBERSHIP,
      kWildcardAddress,
      kWildcardPort);
  if (!PermissionsData::CheckAPIPermissionWithParam(
          GetExtension(),
          APIPermission::kSocket,
          &param)) {
    error_ = kPermissionError;
    return;
  }

  const std::vector<std::string>& groups = socket->GetJoinedGroups();
  results_ = sockets_udp::GetJoinedGroups::Results::Create(groups);
}

}  // namespace api
}  // namespace extensions
