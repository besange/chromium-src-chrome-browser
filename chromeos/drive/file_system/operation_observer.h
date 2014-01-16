// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_CHROMEOS_DRIVE_FILE_SYSTEM_OPERATION_OBSERVER_H_
#define CHROME_BROWSER_CHROMEOS_DRIVE_FILE_SYSTEM_OPERATION_OBSERVER_H_

namespace base {
class FilePath;
}

namespace drive {
namespace file_system {

// Error type of sync client.
enum DriveSyncErrorType {
  // Request to delete a file without permission.
  DELETE_WITHOUT_PERMISSION
};

// Passes notifications from Drive operations back to the file system.
class OperationObserver {
 public:
  // Sent when a content of a directory has been changed.
  // |directory_path| is a virtual directory path representing the
  // changed directory.
  virtual void OnDirectoryChangedByOperation(
      const base::FilePath& directory_path) = 0;

  // Sent when a cache file is modified and upload is needed.
  virtual void OnCacheFileUploadNeededByOperation(
      const std::string& local_id) = 0;

  // Sent when metadata entry is updated and sync is needed.
  virtual void OnEntryUpdatedByOperation(const std::string& local_id) {}

  // Sent when a specific drive sync error occurred.
  virtual void OnDriveSyncError(DriveSyncErrorType type) {}
};

}  // namespace file_system
}  // namespace drive

#endif  // CHROME_BROWSER_CHROMEOS_DRIVE_FILE_SYSTEM_OPERATION_OBSERVER_H_
