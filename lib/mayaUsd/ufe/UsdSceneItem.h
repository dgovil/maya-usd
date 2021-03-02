//
// Copyright 2019 Autodesk
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
#pragma once

#include <mayaUsd/base/api.h>

#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usdGeom/pointInstancer.h>
#include <pxr/usdImaging/usdImaging/delegate.h>

#include <ufe/path.h>
#include <ufe/sceneItem.h>

namespace MAYAUSD_NS_DEF {
namespace ufe {

//! \brief USD run-time scene item interface
//
// UsdSceneItem implements the Ufe::SceneItem interface for UsdPrims.
// Typically there will be a direct mapping between a location in the UFE scene
// item hierarchy identified by a Ufe::Path and a location in the USD namespace
// identified by an SdfPath and represented by a UsdPrim. UFE and USD are
// consistent in that way such that parent scene items/prims can be found by
// popping components off of the path and child scene items/prims can be found
// by appending components to the path.
//
// The above is also true of USD PointInstancer schema prims
// (UsdGeomPointInstancer). PointInstancer prims are treated the same as any
// other UsdPrim and their definitions of parent and child prims are the same
// as well. PointInstancer prims will often have a single prototypes scope as a
// child under which all of the PointInstancer's prototypes are defined.
//
// The point instances generated by PointInstancer prims, however, are handled
// a bit differently, and there are subtle differences in semantics between USD
// and UFE.
//
// Point instances are generated based on data members of a PointInstancer prim
// and a "prototype" prim (or a hierarchy of prims below a prototype prim) that
// is being instanced. As a result, point instances themselves do not occupy a
// location in USD namespace and are instead uniquely identified by the
// combination of the PointInstancer that generated the point instance and an
// instance index used to access per-instance values in the PointInstancer's
// array attributes. See additional detail on PointInstancer prims here:
//
// https://graphics.pixar.com/usd/docs/api/class_usd_geom_point_instancer.html
//
// This means that in a USD namespace sense, a point instance does not really
// have either a parent or children. Similarly, the point instances generated
// by a PointInstancer prim would not be considered the PointInstancer's
// children, nor would the PointInstancer prim be considered the parent of any
// point instances that it generates.
//
// In UFE, however, there is utility in treating a PointInstancer as the parent
// of the point instances it generates. For example, this is used to enable
// pick-walking from a point instance up to the PointInstancer that generated
// it, and then further up the scene hierarchy if desired. As in USD though,
// point instances in UFE still do not have any children. This handling in UFE
// ensures that individual point instances can be selected and have their
// transformation manipulated (which is authored as edits to the positions,
// orientations, scales, etc. attributes on the PointInstancer prim), but no
// other per-instance data access or manipulation is allowed, since no such
// authoring is possible in USD. Any authoring intended to affect point
// instances must be done either to the PointInstancer prim that generates that
// point instance, or to the prim (or hierarchy of prims) representing the
// prototype being instanced.
//
class MAYAUSD_CORE_PUBLIC UsdSceneItem : public Ufe::SceneItem
{
public:
    typedef std::shared_ptr<UsdSceneItem> Ptr;

    UsdSceneItem(
        const Ufe::Path&       path,
        const PXR_NS::UsdPrim& prim,
        int                    instanceIndex = PXR_NS::UsdImagingDelegate::ALL_INSTANCES);
    ~UsdSceneItem() override = default;

    // Delete the copy/move constructors assignment operators.
    UsdSceneItem(const UsdSceneItem&) = delete;
    UsdSceneItem& operator=(const UsdSceneItem&) = delete;
    UsdSceneItem(UsdSceneItem&&) = delete;
    UsdSceneItem& operator=(UsdSceneItem&&) = delete;

    //! Create a UsdSceneItem from a UFE path and a USD prim.
    //
    // A non-negative instanceIndex should be provided if the scene item is
    // intended to represent an individual instance of a PointInstancer.
    static UsdSceneItem::Ptr create(
        const Ufe::Path&       path,
        const PXR_NS::UsdPrim& prim,
        int                    instanceIndex = PXR_NS::UsdImagingDelegate::ALL_INSTANCES);

    const PXR_NS::UsdPrim& prim() const { return fPrim; }

    int instanceIndex() const { return _instanceIndex; }

    //! Returns true if the UsdSceneItem represents a point instance.
    //
    // The scene item represents a point instance if its prim is a
    // PointInstancer and its instanceIndex is non-negative.
    bool isPointInstance() const
    {
        return (fPrim && fPrim.IsA<PXR_NS::UsdGeomPointInstancer>() && _instanceIndex >= 0);
    }

    // Ufe::SceneItem overrides
    std::string nodeType() const override;
#ifdef UFE_V2_FEATURES_AVAILABLE
    std::vector<std::string> ancestorNodeTypes() const override;
#endif

private:
    PXR_NS::UsdPrim fPrim;
    const int _instanceIndex;
}; // UsdSceneItem

} // namespace ufe
} // namespace MAYAUSD_NS_DEF
