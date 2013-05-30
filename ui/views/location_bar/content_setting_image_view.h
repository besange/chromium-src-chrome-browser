// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_VIEWS_LOCATION_BAR_CONTENT_SETTING_IMAGE_VIEW_H_
#define CHROME_BROWSER_UI_VIEWS_LOCATION_BAR_CONTENT_SETTING_IMAGE_VIEW_H_

#include "base/memory/scoped_ptr.h"
#include "chrome/browser/ui/views/location_bar/touchable_location_bar_view.h"
#include "chrome/common/content_settings_types.h"
#include "ui/base/animation/animation_delegate.h"
#include "ui/base/animation/slide_animation.h"
#include "ui/gfx/font.h"
#include "ui/views/painter.h"
#include "ui/views/view.h"
#include "ui/views/widget/widget_observer.h"

class ContentSettingImageModel;
class LocationBarView;

namespace content {
class WebContents;
}

namespace views {
class ImageView;
class Label;
}

// The ContentSettingImageView displays an icon and optional text label for
// various content settings affordances in the location bar (i.e. plugin
// blocking, geolocation).
class ContentSettingImageView : public TouchableLocationBarView,
                                public ui::AnimationDelegate,
                                public views::View,
                                public views::WidgetObserver {
 public:
  // |background_images| is the array of images used to draw
  // the label animation background (if any).
  ContentSettingImageView(ContentSettingsType content_type,
                          const int background_images[],
                          LocationBarView* parent,
                          const gfx::Font& font,
                          int font_y_offset,
                          SkColor font_color);
  virtual ~ContentSettingImageView();

  // TouchableLocationBarView:
  virtual int GetBuiltInHorizontalPadding() const OVERRIDE;

  // Update the decoration from the shown WebContents.
  void Update(content::WebContents* web_contents);

 private:
  // Number of milliseconds spent animating open; also the time spent animating
  // closed.
  static const int kOpenTimeMS;

  // The total animation time, including open and close as well as an
  // intervening "stay open" period.
  static const int kAnimationDurationMS;

  // ui::AnimationDelegate:
  virtual void AnimationEnded(const ui::Animation* animation) OVERRIDE;
  virtual void AnimationProgressed(const ui::Animation* animation) OVERRIDE;
  virtual void AnimationCanceled(const ui::Animation* animation) OVERRIDE;

  // views::View:
  virtual gfx::Size GetPreferredSize() OVERRIDE;
  virtual void Layout() OVERRIDE;
  virtual bool OnMousePressed(const ui::MouseEvent& event) OVERRIDE;
  virtual void OnMouseReleased(const ui::MouseEvent& event) OVERRIDE;
  virtual void OnGestureEvent(ui::GestureEvent* event) OVERRIDE;
  virtual void OnPaintBackground(gfx::Canvas* canvas) OVERRIDE;

  // views::WidgetObserver:
  virtual void OnWidgetDestroying(views::Widget* widget) OVERRIDE;

  bool background_showing() const {
    return slide_animator_.is_animating() || pause_animation_;
  }

  int GetTotalSpacingWhileAnimating() const;
  void OnClick();

  LocationBarView* parent_;  // Weak, owns us.
  scoped_ptr<ContentSettingImageModel> content_setting_image_model_;
  scoped_ptr<views::Painter> background_painter_;
  views::ImageView* icon_;
  views::Label* text_label_;
  ui::SlideAnimation slide_animator_;
  bool pause_animation_;
  double pause_animation_state_;
  views::Widget* bubble_widget_;

  DISALLOW_COPY_AND_ASSIGN(ContentSettingImageView);
};

#endif  // CHROME_BROWSER_UI_VIEWS_LOCATION_BAR_CONTENT_SETTING_IMAGE_VIEW_H_
