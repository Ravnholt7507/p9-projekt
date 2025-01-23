#include "DFO.h"
#include <vector>
#include <stdexcept>

// Shift a DFO "forward" by shiftHours: prepend shiftHours idle slices.
DFO shiftDFOForward(const DFO &original, int shiftHours)
{
    if (shiftHours <= 0) return original;

    // The new DFO has (shiftHours + original.polygons.size()) slices.
    DFO shifted(original.dfo_id, {}, {}, original.polygons[0].numsamples);
    shifted.polygons.clear();

    // 1) Prepend 'shiftHours' idle slices
    for (int i = 0; i < shiftHours; i++) {
        // each idle slice: min_prev=0, max_prev=0
        DependencyPolygon idle(0.0, 0.0, original.polygons[0].numsamples);
        // Optionally fill 'idle.points' with something minimal
        // e.g. two trivial points
        idle.points.push_back({0,0});
        idle.points.push_back({0,0});

        shifted.polygons.push_back(idle);
    }

    // 2) Append the original polygons
    for (auto &pg : original.polygons) {
        shifted.polygons.push_back(pg);
    }
    return shifted;
}

// Shift a DFO "backward" by shiftHours: append shiftHours idle slices.
// (Often less relevant in aggregator logic, but for completeness:)
DFO shiftDFOBackward(const DFO &original, int shiftHours)
{
    if (shiftHours <= 0) return original;

    DFO shifted(original.dfo_id, {}, {}, original.polygons[0].numsamples);
    shifted.polygons = original.polygons; // copy existing

    // 1) Append 'shiftHours' idle slices
    for (int i = 0; i < shiftHours; i++) {
        // This idle might have min=..., max=... = the final polygon's max?
        // Or simply zero. Depends on how you want to model “ending early”
        DependencyPolygon idle(shifted.polygons.back().max_prev_energy,
                               shifted.polygons.back().max_prev_energy,
                               original.polygons[0].numsamples);

        // minimal set of points
        idle.points.push_back({idle.min_prev_energy, 0.0});
        idle.points.push_back({idle.min_prev_energy, 0.0});

        shifted.polygons.push_back(idle);
    }
    return shifted;
}
