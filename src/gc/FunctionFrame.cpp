#include <deque>
#include <vector>
#include "HeapReference.hpp"

using namespace std;
using namespace Theta;

namespace Theta {
  struct FunctionFrame {
    int epoch; // The GC epoch at which this frame was last synced

    deque<HeapReference> references;

    FunctionFrame(int currentEpoch) : epoch(currentEpoch) {}

    void updateHeapReferences(vector<pair<int, int>> referenceMappings) {
      for (HeapReference &ref : references) {
        for (pair<int, int> remappedRef : referenceMappings) {
          if (ref.address == remappedRef.first) {
            ref.previousAddress = remappedRef.first;
            ref.address = remappedRef.second;
          } 
        }
      }
    }
  };
}
