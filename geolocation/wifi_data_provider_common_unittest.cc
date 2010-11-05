// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/geolocation/wifi_data_provider_common.h"

#include <vector>

#include "base/scoped_ptr.h"
#include "base/string_util.h"
#include "base/third_party/dynamic_annotations/dynamic_annotations.h"
#include "base/utf_string_conversions.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

using testing::_;
using testing::AtLeast;
using testing::DoDefault;
using testing::Invoke;
using testing::Return;

class MockWlanApi : public WifiDataProviderCommon::WlanApiInterface {
 public:
  MockWlanApi() : calls_(0), bool_return_(true) {
    ANNOTATE_BENIGN_RACE(&calls_, "This is a test-only data race on a counter");
    ON_CALL(*this, GetAccessPointData(_))
        .WillByDefault(Invoke(this, &MockWlanApi::GetAccessPointDataInternal));
  }

  MOCK_METHOD1(GetAccessPointData, bool(WifiData::AccessPointDataSet* data));

  int calls_;
  bool bool_return_;
  WifiData::AccessPointDataSet data_out_;

 private:
  bool GetAccessPointDataInternal(WifiData::AccessPointDataSet* data) {
    ++calls_;
    *data = data_out_;
    return bool_return_;
  }
};

class MockPollingPolicy :public PollingPolicyInterface {
 public:
  MockPollingPolicy() {
    ON_CALL(*this,PollingInterval())
        .WillByDefault(Return(1));
    ON_CALL(*this,NoWifiInterval())
        .WillByDefault(Return(1));
  }

  MOCK_METHOD0(PollingInterval, int());
  MOCK_METHOD0(NoWifiInterval, int());

  virtual void UpdatePollingInterval(bool) {}
};

// Stops the specified (nested) message loop when the listener is called back.
class MessageLoopQuitListener
    : public WifiDataProviderCommon::ListenerInterface {
 public:
  explicit MessageLoopQuitListener(MessageLoop* message_loop)
      : message_loop_to_quit_(message_loop) {
    CHECK(message_loop_to_quit_ != NULL);
  }
  // ListenerInterface
  virtual void DeviceDataUpdateAvailable(
      DeviceDataProvider<WifiData>* provider) {
    // Provider should call back on client's thread.
    EXPECT_EQ(MessageLoop::current(), message_loop_to_quit_);
    provider_ = provider;
    message_loop_to_quit_->QuitNow();
  }
  MessageLoop* message_loop_to_quit_;
  DeviceDataProvider<WifiData>* provider_;
};


class WifiDataProviderCommonWithMock : public WifiDataProviderCommon {
 public:
  WifiDataProviderCommonWithMock()
      : new_wlan_api_(new MockWlanApi),
        new_polling_policy_(new MockPollingPolicy) {}

  // WifiDataProviderCommon
  virtual WlanApiInterface* NewWlanApi() {
    CHECK(new_wlan_api_ != NULL);
    return new_wlan_api_.release();
  }
  virtual PollingPolicyInterface* NewPollingPolicy() {
    CHECK(new_polling_policy_ != NULL);
    return new_polling_policy_.release();
  }

  scoped_ptr<MockWlanApi> new_wlan_api_;
  scoped_ptr<MockPollingPolicy> new_polling_policy_;

  DISALLOW_COPY_AND_ASSIGN(WifiDataProviderCommonWithMock);
};

WifiDataProviderImplBase* CreateWifiDataProviderCommonWithMock() {
  return new WifiDataProviderCommonWithMock;
}

// Main test fixture
class GeolocationWifiDataProviderCommonTest : public testing::Test {
 public:
  GeolocationWifiDataProviderCommonTest()
      : quit_listener_(&main_message_loop_) {
  }

  virtual void SetUp() {
    provider_ = new WifiDataProviderCommonWithMock;
    wlan_api_ = provider_->new_wlan_api_.get();
    polling_policy_ = provider_->new_polling_policy_.get();
    provider_->AddListener(&quit_listener_);
  }
  virtual void TearDown() {
    provider_->RemoveListener(&quit_listener_);
    provider_->StopDataProvider();
    provider_ = NULL;
  }

 protected:
  MessageLoop main_message_loop_;
  MessageLoopQuitListener quit_listener_;
  scoped_refptr<WifiDataProviderCommonWithMock> provider_;
  MockWlanApi* wlan_api_;
  MockPollingPolicy* polling_policy_;
};

TEST_F(GeolocationWifiDataProviderCommonTest, CreateDestroy) {
  // Test fixture members were SetUp correctly.
  EXPECT_EQ(&main_message_loop_, MessageLoop::current());
  EXPECT_TRUE(NULL != provider_.get());
  EXPECT_TRUE(NULL != wlan_api_);
}

TEST_F(GeolocationWifiDataProviderCommonTest, StartThread) {
  EXPECT_CALL(*wlan_api_, GetAccessPointData(_))
      .Times(AtLeast(1));
  EXPECT_CALL(*polling_policy_, PollingInterval())
      .Times(AtLeast(1));
  EXPECT_TRUE(provider_->StartDataProvider());
  main_message_loop_.Run();
  SUCCEED();
}

TEST_F(GeolocationWifiDataProviderCommonTest, NoWifi){
  EXPECT_CALL(*polling_policy_, NoWifiInterval())
      .Times(AtLeast(1));
  EXPECT_CALL(*wlan_api_, GetAccessPointData(_))
      .WillRepeatedly(Return(false));
  provider_->StartDataProvider();
  main_message_loop_.Run();
}

TEST_F(GeolocationWifiDataProviderCommonTest, IntermittentWifi){
  EXPECT_CALL(*polling_policy_, PollingInterval())
      .Times(AtLeast(1));
  EXPECT_CALL(*polling_policy_, NoWifiInterval())
      .Times(1);
  EXPECT_CALL(*wlan_api_, GetAccessPointData(_))
      .WillOnce(Return(true))
      .WillOnce(Return(false))
      .WillRepeatedly(DoDefault());

  AccessPointData single_access_point;
  single_access_point.channel = 2;
  single_access_point.mac_address = 3;
  single_access_point.radio_signal_strength = 4;
  single_access_point.signal_to_noise = 5;
  single_access_point.ssid = ASCIIToUTF16("foossid");
  wlan_api_->data_out_.insert(single_access_point);

  provider_->StartDataProvider();
  main_message_loop_.Run();
  main_message_loop_.Run();
}

TEST_F(GeolocationWifiDataProviderCommonTest, DoAnEmptyScan) {
  EXPECT_CALL(*wlan_api_, GetAccessPointData(_))
      .Times(AtLeast(1));
  EXPECT_CALL(*polling_policy_, PollingInterval())
      .Times(AtLeast(1));
  EXPECT_TRUE(provider_->StartDataProvider());
  main_message_loop_.Run();
  // Check we had at least one call. The worker thread may have raced ahead
  // and made multiple calls.
  EXPECT_GT(wlan_api_->calls_, 0);
  WifiData data;
  EXPECT_TRUE(provider_->GetData(&data));
  EXPECT_EQ(0, static_cast<int>(data.access_point_data.size()));
}

TEST_F(GeolocationWifiDataProviderCommonTest, DoScanWithResults) {
  EXPECT_CALL(*wlan_api_, GetAccessPointData(_))
      .Times(AtLeast(1));
  EXPECT_CALL(*polling_policy_, PollingInterval())
      .Times(AtLeast(1));
  AccessPointData single_access_point;
  single_access_point.channel = 2;
  single_access_point.mac_address = 3;
  single_access_point.radio_signal_strength = 4;
  single_access_point.signal_to_noise = 5;
  single_access_point.ssid = ASCIIToUTF16("foossid");
  wlan_api_->data_out_.insert(single_access_point);

  EXPECT_TRUE(provider_->StartDataProvider());
  main_message_loop_.Run();
  EXPECT_GT(wlan_api_->calls_, 0);
  WifiData data;
  EXPECT_TRUE(provider_->GetData(&data));
  EXPECT_EQ(1, static_cast<int>(data.access_point_data.size()));
  EXPECT_EQ(single_access_point.ssid, data.access_point_data.begin()->ssid);
}

TEST_F(GeolocationWifiDataProviderCommonTest,
       StartThreadViaDeviceDataProvider) {
  MessageLoopQuitListener quit_listener(&main_message_loop_);
  WifiDataProvider::SetFactory(CreateWifiDataProviderCommonWithMock);
  DeviceDataProvider<WifiData>::Register(&quit_listener);
  main_message_loop_.Run();
  DeviceDataProvider<WifiData>::Unregister(&quit_listener);
  DeviceDataProvider<WifiData>::ResetFactory();
}
