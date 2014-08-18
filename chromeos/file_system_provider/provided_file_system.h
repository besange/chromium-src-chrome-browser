// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_CHROMEOS_FILE_SYSTEM_PROVIDER_PROVIDED_FILE_SYSTEM_H_
#define CHROME_BROWSER_CHROMEOS_FILE_SYSTEM_PROVIDER_PROVIDED_FILE_SYSTEM_H_

#include "base/memory/weak_ptr.h"
#include "chrome/browser/chromeos/file_system_provider/provided_file_system_info.h"
#include "chrome/browser/chromeos/file_system_provider/provided_file_system_interface.h"
#include "chrome/browser/chromeos/file_system_provider/request_manager.h"
#include "webkit/browser/fileapi/async_file_util.h"

class Profile;

namespace net {
class IOBuffer;
}  // namespace net

namespace base {
class FilePath;
}  // namespace base

namespace extensions {
class EventRouter;
}  // namespace extensions

namespace chromeos {
namespace file_system_provider {

class NotificationManagerInterface;

// Provided file system implementation. Forwards requests between providers and
// clients.
class ProvidedFileSystem : public ProvidedFileSystemInterface {
 public:
  ProvidedFileSystem(Profile* profile,
                     const ProvidedFileSystemInfo& file_system_info);
  virtual ~ProvidedFileSystem();

  // ProvidedFileSystemInterface overrides.
  virtual AbortCallback RequestUnmount(
      const fileapi::AsyncFileUtil::StatusCallback& callback) OVERRIDE;
  virtual AbortCallback GetMetadata(
      const base::FilePath& entry_path,
      const GetMetadataCallback& callback) OVERRIDE;
  virtual AbortCallback ReadDirectory(
      const base::FilePath& directory_path,
      const fileapi::AsyncFileUtil::ReadDirectoryCallback& callback) OVERRIDE;
  virtual AbortCallback OpenFile(const base::FilePath& file_path,
                                 OpenFileMode mode,
                                 const OpenFileCallback& callback) OVERRIDE;
  virtual AbortCallback CloseFile(
      int file_handle,
      const fileapi::AsyncFileUtil::StatusCallback& callback) OVERRIDE;
  virtual AbortCallback ReadFile(
      int file_handle,
      net::IOBuffer* buffer,
      int64 offset,
      int length,
      const ReadChunkReceivedCallback& callback) OVERRIDE;
  virtual AbortCallback CreateDirectory(
      const base::FilePath& directory_path,
      bool exclusive,
      bool recursive,
      const fileapi::AsyncFileUtil::StatusCallback& callback) OVERRIDE;
  virtual AbortCallback DeleteEntry(
      const base::FilePath& entry_path,
      bool recursive,
      const fileapi::AsyncFileUtil::StatusCallback& callback) OVERRIDE;
  virtual AbortCallback CreateFile(
      const base::FilePath& file_path,
      const fileapi::AsyncFileUtil::StatusCallback& callback) OVERRIDE;
  virtual AbortCallback CopyEntry(
      const base::FilePath& source_path,
      const base::FilePath& target_path,
      const fileapi::AsyncFileUtil::StatusCallback& callback) OVERRIDE;
  virtual AbortCallback MoveEntry(
      const base::FilePath& source_path,
      const base::FilePath& target_path,
      const fileapi::AsyncFileUtil::StatusCallback& callback) OVERRIDE;
  virtual AbortCallback Truncate(
      const base::FilePath& file_path,
      int64 length,
      const fileapi::AsyncFileUtil::StatusCallback& callback) OVERRIDE;
  virtual AbortCallback WriteFile(
      int file_handle,
      net::IOBuffer* buffer,
      int64 offset,
      int length,
      const fileapi::AsyncFileUtil::StatusCallback& callback) OVERRIDE;
  virtual const ProvidedFileSystemInfo& GetFileSystemInfo() const OVERRIDE;
  virtual RequestManager* GetRequestManager() OVERRIDE;
  virtual base::WeakPtr<ProvidedFileSystemInterface> GetWeakPtr() OVERRIDE;

 private:
  // Aborts an operation executed with a request id equal to
  // |operation_request_id|. The request is removed immediately on the C++ side
  // despite being handled by the providing extension or not.
  void Abort(int operation_request_id,
             const fileapi::AsyncFileUtil::StatusCallback& callback);

  Profile* profile_;                       // Not owned.
  extensions::EventRouter* event_router_;  // Not owned. May be NULL.
  ProvidedFileSystemInfo file_system_info_;
  scoped_ptr<NotificationManagerInterface> notification_manager_;
  RequestManager request_manager_;

  base::WeakPtrFactory<ProvidedFileSystem> weak_ptr_factory_;
  DISALLOW_COPY_AND_ASSIGN(ProvidedFileSystem);
};

}  // namespace file_system_provider
}  // namespace chromeos

#endif  // CHROME_BROWSER_CHROMEOS_FILE_SYSTEM_PROVIDER_PROVIDED_FILE_SYSTEM_H_
