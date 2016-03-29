//
// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.md file in the project root for full license information.
//

#pragma once

#include "Matrix.h"
#include "TensorShape.h" // for ImageLayoutKind
#include "ConvolveGeometry.h"

namespace Microsoft { namespace MSR { namespace CNTK {

//-------------------------------------------------------------
// Convolution and pooling engine interface.
//-------------------------------------------------------------
enum class ConvolutionEngineKind
{
    None      = 0,
    Reference = 1,
    CuDnn     = 1 << 1,
    Legacy    = 1 << 2,

    All     = Reference | CuDnn | Legacy
};

enum class PoolKind
{
    None,
    Max,
    Average
};

#pragma warning(push)
#pragma warning(disable : 4251)

template <class ElemType>
class MATH_API ConvolutionEngine
{
public:
    using Mat = Matrix<ElemType>;

public:
    virtual ~ConvolutionEngine() = default;

    void Forward(const Mat& in, const Mat& filter, Mat& out, Mat& workspace);

    void BackwardData(const Mat& srcGrad, const Mat& filter, Mat& grad, Mat& workspace);

    void BackwardFilter(const Mat& srcGrad, const Mat& in, Mat& filterGrad, bool allowReuse, Mat& workspace);

    void ForwardPooling(const Mat& in, Mat& out);

    void BackwardPooling(const Mat& out, const Mat& srcGrad, const Mat& in, Mat& grad);

    static std::unique_ptr<ConvolutionEngine<ElemType>> Create(ConvolveGeometryPtr geometry, DEVICEID_TYPE deviceId, ImageLayoutKind imageLayout,
                                                               size_t maxTempMemSizeInSamples, PoolKind poolKind = PoolKind::None, ConvolutionEngineKind enabledEngines = ConvolutionEngineKind::All);

    DISABLE_COPY_AND_MOVE(ConvolutionEngine);

protected:
    ConvolutionEngine(ConvolveGeometryPtr geometry, DEVICEID_TYPE deviceId, ImageLayoutKind imageLayout, size_t maxTempMemSizeInSamples, PoolKind poolKind)
        : m_geometry(geometry), m_deviceId(deviceId), m_imageLayout(imageLayout), m_maxTempMemSizeInSamples(maxTempMemSizeInSamples), m_poolKind(poolKind)
    {
        assert(m_geometry != nullptr);
    }

    virtual void EnsureCompatible() = 0;

    virtual void EnsureConvolutionInitialized() = 0;

    virtual void ForwardCore(const Mat& in, const Mat& filter, Mat& out, Mat& workspace) = 0;

    virtual void BackwardDataCore(const Mat& srcGrad, const Mat& filter, Mat& grad, Mat& workspace) = 0;

    virtual void BackwardFilterCore(const Mat& srcGrad, const Mat& in, Mat& filterGrad, bool allowReuse, Mat& workspace) = 0;

    virtual void EnsurePoolingInitialized() = 0;

    virtual void ForwardPoolingCore(const Mat& in, Mat& out) = 0;

    virtual void BackwardPoolingCore(const Mat& out, const Mat& srcGrad, const Mat& in, Mat& grad) = 0;

protected:
    ConvolveGeometryPtr m_geometry;
    DEVICEID_TYPE m_deviceId;
    ImageLayoutKind m_imageLayout;
    size_t m_maxTempMemSizeInSamples;
    PoolKind m_poolKind;
};

#pragma warning(pop)

} } }
