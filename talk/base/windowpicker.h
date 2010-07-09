// Copyright 2010 Google Inc. All Rights Reserved
// Author: oja@google.com (Fredrik Oja)

#ifndef TALK_BASE_WINDOWPICKER_H_
#define TALK_BASE_WINDOWPICKER_H_

#include <list>
#include <string>

namespace talk_base {

typedef unsigned int WindowId;

class WindowDescription {
 public:
  WindowDescription(WindowId id, const std::string& title)
      : id_(id), title_(title) {
  }
  WindowId id() const {
    return id_;
  }
  const std::string& title() const {
    return title_;
  }

 private:
  const WindowId id_;
  const std::string title_;
};

typedef std::list<WindowDescription> WindowDescriptionList;

class WindowPicker {
 public:
  virtual ~WindowPicker() {}
  virtual bool Init() = 0;
  virtual bool GetWindowList(WindowDescriptionList* descriptions) = 0;
};

}  // namespace talk_base

#endif  // TALK_BASE_WINDOWPICKER_H_
