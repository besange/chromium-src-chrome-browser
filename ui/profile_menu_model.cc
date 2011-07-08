// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/profile_menu_model.h"

#include "chrome/browser/browser_process.h"
#include "chrome/browser/profiles/profile_manager.h"
#include "chrome/common/url_constants.h"
#include "grit/chromium_strings.h"
#include "grit/generated_resources.h"
#include "ui/base/l10n/l10n_util.h"

ProfileMenuModel::ProfileMenuModel(Browser* browser)
    : ALLOW_THIS_IN_INITIALIZER_LIST(ui::SimpleMenuModel(this)),
      browser_(browser) {
  AddItemWithStringId(COMMAND_CUSTOMIZE_PROFILE,
                      IDS_PROFILES_CUSTOMIZE_PROFILE);
  AddSeparator();

  const string16 short_product_name =
      l10n_util::GetStringUTF16(IDS_SHORT_PRODUCT_NAME);
  AddItem(COMMAND_CREATE_NEW_PROFILE, l10n_util::GetStringFUTF16(
      IDS_PROFILES_CREATE_NEW_PROFILE_OPTION, short_product_name));
  AddItemWithStringId(COMMAND_DELETE_PROFILE,
                      IDS_PROFILES_DELETE_PROFILE);
}

ProfileMenuModel::~ProfileMenuModel() {
}

// ui::SimpleMenuModel::Delegate implementation
bool ProfileMenuModel::IsCommandIdChecked(int command_id) const {
  return false;
}

bool ProfileMenuModel::IsCommandIdEnabled(int command_id) const {
  return true;
}

bool ProfileMenuModel::GetAcceleratorForCommandId(int command_id,
    ui::Accelerator* accelerator) {
  return false;
}

void ProfileMenuModel::ExecuteCommand(int command_id) {
  switch (command_id) {
     case COMMAND_CUSTOMIZE_PROFILE:
       browser_->ShowOptionsTab(chrome::kPersonalOptionsSubPage);
       break;
    case COMMAND_CREATE_NEW_PROFILE:
      ProfileManager::CreateMultiProfileAsync();
      break;
    case COMMAND_DELETE_PROFILE:
      g_browser_process->profile_manager()->ScheduleProfileForDeletion(
          browser_->profile()->GetPath());
      break;
    default:
      NOTREACHED();
      break;
  }
}
