// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/bind.h"
#include "base/message_loop/message_loop.h"
#include "chrome/browser/ui/views/app_list/win/app_list_shower.h"

AppListShower::AppListShower(scoped_ptr<AppListViewFactory> factory,
                             scoped_ptr<KeepAliveService> keep_alive)
    : factory_(factory.Pass()),
      keep_alive_service_(keep_alive.Pass()),
      profile_(NULL),
      can_close_app_list_(true) {
}

AppListShower::~AppListShower() {
}

void AppListShower::ShowAndReacquireFocus(Profile* requested_profile) {
  ShowForProfile(requested_profile);
  view_->RegainNextLostFocus();
}

void AppListShower::ShowForProfile(Profile* requested_profile) {
  // If the app list is already displaying |profile| just activate it (in case
  // we have lost focus).
  if (IsAppListVisible() && (requested_profile == profile_)) {
    view_->Show();
    return;
  }

  if (!view_) {
    CreateViewForProfile(requested_profile);
  } else if (requested_profile != profile_) {
    profile_ = requested_profile;
    view_->SetProfile(requested_profile);
  }

  keep_alive_service_->EnsureKeepAlive();
  if (!IsAppListVisible())
    view_->MoveNearCursor();
  view_->Show();
}

gfx::NativeWindow AppListShower::GetWindow() {
  if (!IsAppListVisible())
    return NULL;
  return view_->GetWindow();
}

void AppListShower::CreateViewForProfile(Profile* requested_profile) {
  // Aura has problems with layered windows and bubble delegates. The app
  // launcher has a trick where it only hides the window when it is dismissed,
  // reshowing it again later. This does not work with win aura for some
  // reason. This change temporarily makes it always get recreated, only on
  // win aura. See http://crbug.com/176186.
#if !defined(USE_AURA)
  if (requested_profile == profile_)
    return;
#endif

  profile_ = requested_profile;
  view_.reset(factory_->CreateAppListView(
      profile_, &pagination_model_,
      base::Bind(&AppListShower::DismissAppList, base::Unretained(this))));
}

void AppListShower::DismissAppList() {
  if (view_ && can_close_app_list_) {
    view_->Hide();
    keep_alive_service_->FreeKeepAlive();
  }
}

void AppListShower::CloseAppList() {
  view_.reset();
  profile_ = NULL;

  // We may end up here as the result of the OS deleting the AppList's
  // widget (WidgetObserver::OnWidgetDestroyed). If this happens and there
  // are no browsers around then deleting the keep alive will result in
  // deleting the Widget again (by way of CloseAllSecondaryWidgets). When
  // the stack unravels we end up back in the Widget that was deleted and
  // crash. By delaying deletion of the keep alive we ensure the Widget has
  // correctly been destroyed before ending the keep alive so that
  // CloseAllSecondaryWidgets() won't attempt to delete the AppList's Widget
  // again.
  if (base::MessageLoop::current()) {  // NULL in tests.
    base::MessageLoop::current()->PostTask(FROM_HERE,
        base::Bind(&KeepAliveService::FreeKeepAlive,
                   base::Unretained(keep_alive_service_.get())));
    return;
  }
  keep_alive_service_->FreeKeepAlive();
}

bool AppListShower::IsAppListVisible() const {
  return view_ && view_->IsVisible();
}

void AppListShower::WarmupForProfile(Profile* profile) {
  DCHECK(!profile_);
  CreateViewForProfile(profile);
  view_->Prerender();
}

bool AppListShower::HasView() const {
  return !!view_;
}
