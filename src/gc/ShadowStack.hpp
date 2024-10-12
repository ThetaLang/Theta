#pragma once

#include <deque>
#include "FunctionFrame.cpp"
#include "HeapReference.hpp"


namespace Theta {
  class ShadowStack {
  public:

    static ShadowStack& getInstance() {
      static ShadowStack instance;
      return instance;
    }

    void pushFrame(int epoch) {
      frames.push_front(FunctionFrame(epoch));
    }

    void popFrame() {
      if (!frames.empty()) frames.pop_front();
    }

    void pushReference(int address) {
      if (frames.empty()) return;

      frames.front().references.push_front(HeapReference(address));
    }

    void popReference() {
      if (frames.empty()) return;

      return frames.front().references.pop_front();
    }

    const FunctionFrame currentFrame() {
      return frames.front();
    }

    bool isCurrentFrameOutdated(int currentEpoch) {
      if (frames.empty()) return false;

      return frames.front().epoch < currentEpoch;
    }

    void updateFrameReferences(vector<pair<int, int>> referenceMappings) {
      for (FunctionFrame &frame : frames) {
        frame.updateHeapReferences(referenceMappings);
      }
    }

  private:
    ShadowStack() {}

    ShadowStack(const ShadowStack&) = delete;
    ShadowStack& operator=(const ShadowStack) = delete;

    deque<FunctionFrame> frames;
  };
}
