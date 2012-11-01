// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_PREFS_PREF_NOTIFIER_IMPL_H_
#define CHROME_BROWSER_PREFS_PREF_NOTIFIER_IMPL_H_

#include <list>
#include <string>

#include "base/callback.h"
#include "base/hash_tables.h"
#include "base/observer_list.h"
#include "base/prefs/pref_notifier.h"
#include "base/prefs/public/pref_observer.h"
#include "base/threading/non_thread_safe.h"

class PrefService;

// The PrefNotifier implementation used by the PrefService.
class PrefNotifierImpl : public PrefNotifier,
                         public base::NonThreadSafe {
 public:
  PrefNotifierImpl();
  explicit PrefNotifierImpl(PrefService* pref_service);
  virtual ~PrefNotifierImpl();

  // If the pref at the given path changes, we call the observer's
  // OnPreferenceChanged method.
  void AddPrefObserver(const char* path, PrefObserver* observer);
  void RemovePrefObserver(const char* path, PrefObserver* observer);

  // We run the callback once, when initialization completes. The bool
  // parameter will be set to true for successful initialization,
  // false for unsuccessful.
  void AddInitObserver(base::Callback<void(bool)> observer);

  void SetPrefService(PrefService* pref_service);

 protected:
  // PrefNotifier overrides.
  virtual void OnPreferenceChanged(const std::string& pref_name) OVERRIDE;
  virtual void OnInitializationCompleted(bool succeeded) OVERRIDE;

  // A map from pref names to a list of observers. Observers get fired in the
  // order they are added. These should only be accessed externally for unit
  // testing.
  typedef ObserverList<PrefObserver> PrefObserverList;
  typedef base::hash_map<std::string, PrefObserverList*> PrefObserverMap;

  typedef std::list<base::Callback<void(bool)> > PrefInitObserverList;

  const PrefObserverMap* pref_observers() const { return &pref_observers_; }

 private:
  // For the given pref_name, fire any observer of the pref. Virtual so it can
  // be mocked for unit testing.
  virtual void FireObservers(const std::string& path);

  // Weak reference; the notifier is owned by the PrefService.
  PrefService* pref_service_;

  PrefObserverMap pref_observers_;
  PrefInitObserverList init_observers_;

  DISALLOW_COPY_AND_ASSIGN(PrefNotifierImpl);
};

#endif  // CHROME_BROWSER_PREFS_PREF_NOTIFIER_IMPL_H_
