// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/views/sad_tab_view.h"

#include "base/metrics/histogram.h"
#include "base/utf_string_conversions.h"
#include "chrome/browser/google/google_util.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/webui/bug_report_ui.h"
#include "chrome/browser/userfeedback/proto/extension.pb.h"
#include "chrome/common/url_constants.h"
#include "content/browser/tab_contents/tab_contents.h"
#include "content/browser/tab_contents/tab_contents_delegate.h"
#include "grit/generated_resources.h"
#include "grit/locale_settings.h"
#include "grit/theme_resources.h"
#include "third_party/skia/include/effects/SkGradientShader.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/gfx/canvas.h"
#include "ui/gfx/canvas_skia.h"
#include "ui/gfx/font.h"
#include "ui/gfx/size.h"
#include "ui/gfx/skia_util.h"
#include "views/controls/link.h"

static const int kSadTabOffset = -64;
static const int kIconTitleSpacing = 20;
static const int kTitleMessageSpacing = 15;
static const int kMessageBottomMargin = 20;
static const float kMessageSize = 0.65f;
static const SkColor kTitleColor = SK_ColorWHITE;
static const SkColor kMessageColor = SK_ColorWHITE;
static const SkColor kLinkColor = SK_ColorWHITE;
static const SkColor kCrashBackgroundColor = SkColorSetRGB(35, 48, 64);
static const SkColor kCrashBackgroundEndColor = SkColorSetRGB(35, 48, 64);
// TODO(gspencer): update these colors when the UI team has picked
// official versions.  See http://crosbug.com/10711.
static const SkColor kKillBackgroundColor = SkColorSetRGB(57, 48, 88);
static const SkColor kKillBackgroundEndColor = SkColorSetRGB(57, 48, 88);
static const int kMessageFlags = gfx::Canvas::MULTI_LINE |
    gfx::Canvas::NO_ELLIPSIS | gfx::Canvas::TEXT_ALIGN_CENTER;

// Font size correction.
#if defined(CROS_FONTS_USING_BCI)
static const int kTitleFontSizeDelta = 1;
static const int kMessageFontSizeDelta = 0;
#else
static const int kTitleFontSizeDelta = 2;
static const int kMessageFontSizeDelta = 1;
#endif

SadTabView::SadTabView(TabContents* tab_contents, Kind kind)
    : tab_contents_(tab_contents),
      learn_more_link_(NULL),
      feedback_link_(NULL),
      kind_(kind),
      painted_(false) {
  DCHECK(tab_contents);

  // Sometimes the user will never see this tab, so keep track of the total
  // number of creation events to compare to display events.
  UMA_HISTOGRAM_COUNTS("SadTab.Created", kind);

  ResourceBundle& rb = ResourceBundle::GetSharedInstance();
  title_font_ = new gfx::Font(
      rb.GetFont(ResourceBundle::BaseFont).DeriveFont(kTitleFontSizeDelta,
                                                      gfx::Font::BOLD));
  message_font_ = new gfx::Font(
      rb.GetFont(ResourceBundle::BaseFont).DeriveFont(kMessageFontSizeDelta));
  sad_tab_bitmap_ = rb.GetBitmapNamed(
      kind == CRASHED ? IDR_SAD_TAB : IDR_KILLED_TAB);

  title_ = l10n_util::GetStringUTF16(
      kind == CRASHED ? IDS_SAD_TAB_TITLE : IDS_KILLED_TAB_TITLE);
  title_width_ = title_font_->GetStringWidth(title_);
  message_ = l10n_util::GetStringUTF16(
      kind == CRASHED ? IDS_SAD_TAB_MESSAGE : IDS_KILLED_TAB_MESSAGE);

  if (tab_contents != NULL) {
    learn_more_link_ =
        new views::Link(UTF16ToWide(l10n_util::GetStringUTF16(IDS_LEARN_MORE)));
    learn_more_link_->SetFont(*message_font_);
    learn_more_link_->SetNormalColor(kLinkColor);
    learn_more_link_->set_listener(this);
    AddChildView(learn_more_link_);

    if (kind == KILLED) {
      feedback_link_ = new views::Link(
          UTF16ToWide(l10n_util::GetStringUTF16(IDS_KILLED_TAB_FEEDBACK_LINK)));
      feedback_link_->SetFont(*message_font_);
      feedback_link_->SetNormalColor(kLinkColor);
      feedback_link_->set_listener(this);
      AddChildView(feedback_link_);
    }
  }
}

SadTabView::~SadTabView() {}

void SadTabView::OnPaint(gfx::Canvas* canvas) {
  if (!painted_) {
    // User actually saw the error, keep track for user experience stats.
    UMA_HISTOGRAM_COUNTS("SadTab.Displayed", kind_);
    painted_ = true;
  }
  SkPaint paint;
  SkSafeUnref(paint.setShader(
      gfx::CreateGradientShader(
          0,
          height(),
          kind_ == CRASHED ? kCrashBackgroundColor : kKillBackgroundColor,
          kind_ == CRASHED ?
            kCrashBackgroundEndColor : kKillBackgroundEndColor)));
  paint.setStyle(SkPaint::kFill_Style);
  canvas->AsCanvasSkia()->drawRectCoords(
      0, 0, SkIntToScalar(width()), SkIntToScalar(height()), paint);

  canvas->DrawBitmapInt(*sad_tab_bitmap_, icon_bounds_.x(), icon_bounds_.y());

  canvas->DrawStringInt(title_, *title_font_, kTitleColor,
                        title_bounds_.x(), title_bounds_.y(),
                        title_bounds_.width(), title_bounds_.height(),
                        gfx::Canvas::TEXT_ALIGN_CENTER);

  canvas->DrawStringInt(message_, *message_font_,
                        kMessageColor, message_bounds_.x(), message_bounds_.y(),
                        message_bounds_.width(), message_bounds_.height(),
                        kMessageFlags);

  if (learn_more_link_ != NULL) {
    learn_more_link_->SetBounds(
        learn_more_bounds_.x(), learn_more_bounds_.y(),
        learn_more_bounds_.width(), learn_more_bounds_.height());
  }
  if (feedback_link_ != NULL) {
    feedback_link_->SetBounds(
        feedback_bounds_.x(), feedback_bounds_.y(),
        feedback_bounds_.width(), feedback_bounds_.height());
  }
}

void SadTabView::Layout() {
  int icon_width = sad_tab_bitmap_->width();
  int icon_height = sad_tab_bitmap_->height();
  int icon_x = (width() - icon_width) / 2;
  int icon_y = ((height() - icon_height) / 2) + kSadTabOffset;
  icon_bounds_.SetRect(icon_x, icon_y, icon_width, icon_height);

  int title_x = (width() - title_width_) / 2;
  int title_y = icon_bounds_.bottom() + kIconTitleSpacing;
  int title_height = title_font_->GetHeight();
  title_bounds_.SetRect(title_x, title_y, title_width_, title_height);

  int message_width = static_cast<int>(width() * kMessageSize);
  int message_height = 0;
  gfx::CanvasSkia::SizeStringInt(message_,
                                 *message_font_, &message_width,
                                 &message_height, kMessageFlags);
  int message_x = (width() - message_width) / 2;
  int message_y = title_bounds_.bottom() + kTitleMessageSpacing;
  message_bounds_.SetRect(message_x, message_y, message_width, message_height);
  int bottom = message_bounds_.bottom();

  if (learn_more_link_ != NULL) {
    gfx::Size sz = learn_more_link_->GetPreferredSize();
    gfx::Insets insets = learn_more_link_->GetInsets();
    learn_more_bounds_.SetRect((width() - sz.width()) / 2,
                               bottom + kTitleMessageSpacing - insets.top(),
                               sz.width(),
                               sz.height());
    bottom = learn_more_bounds_.bottom();
  }

  if (feedback_link_ != NULL) {
    gfx::Size sz = feedback_link_->GetPreferredSize();
    gfx::Insets insets = feedback_link_->GetInsets();
    feedback_bounds_.SetRect((width() - sz.width()) / 2,
                             bottom + kTitleMessageSpacing - insets.top(),
                             sz.width(),
                             sz.height());
  }
}

void SadTabView::LinkClicked(views::Link* source, int event_flags) {
  if (tab_contents_ != NULL && source == learn_more_link_) {
    GURL help_url =
        google_util::AppendGoogleLocaleParam(GURL(kind_ == CRASHED ?
                                                  chrome::kCrashReasonURL :
                                                  chrome::kKillReasonURL));
    tab_contents_->OpenURL(help_url, GURL(), CURRENT_TAB, PageTransition::LINK);
  } else if (tab_contents_ != NULL && source == feedback_link_) {
    browser::ShowHtmlBugReportView(
        Browser::GetBrowserForController(&tab_contents_->controller(), NULL),
        l10n_util::GetStringUTF8(IDS_KILLED_TAB_FEEDBACK_MESSAGE),
        userfeedback::ChromeOsData_ChromeOsCategory_CRASH);
  }
}
