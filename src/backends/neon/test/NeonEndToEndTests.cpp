//
// Copyright © 2017 Arm Ltd. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include <backendsCommon/test/EndToEndTestImpl.hpp>

#include <backendsCommon/test/ArgMinMaxEndToEndTestImpl.hpp>
#include <backendsCommon/test/ComparisonEndToEndTestImpl.hpp>
#include <backendsCommon/test/ConcatEndToEndTestImpl.hpp>
#include <backendsCommon/test/DepthToSpaceEndToEndTestImpl.hpp>
#include <backendsCommon/test/DequantizeEndToEndTestImpl.hpp>
#include <backendsCommon/test/DetectionPostProcessEndToEndTestImpl.hpp>
#include <backendsCommon/test/ElementwiseUnaryEndToEndTestImpl.hpp>
#include <backendsCommon/test/InstanceNormalizationEndToEndTestImpl.hpp>
#include <backendsCommon/test/PreluEndToEndTestImpl.hpp>
#include <backendsCommon/test/QuantizedLstmEndToEndTestImpl.hpp>
#include <backendsCommon/test/SpaceToDepthEndToEndTestImpl.hpp>
#include <backendsCommon/test/SplitterEndToEndTestImpl.hpp>
#include <backendsCommon/test/TransposeConvolution2dEndToEndTestImpl.hpp>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(NeonEndToEnd)

std::vector<armnn::BackendId> defaultBackends = {armnn::Compute::CpuAcc};

// Abs
BOOST_AUTO_TEST_CASE(NeonAbsEndToEndTestFloat32)
{
    std::vector<float> expectedOutput =
    {
        1.f, 1.f, 1.f, 1.f, 5.f, 5.f, 5.f, 5.f,
        3.f, 3.f, 3.f, 3.f, 4.f, 4.f, 4.f, 4.f
    };

    ElementwiseUnarySimpleEndToEnd<armnn::DataType::Float32>(defaultBackends,
                                                             UnaryOperation::Abs,
                                                             expectedOutput);
}

// Constant
BOOST_AUTO_TEST_CASE(ConstantUsage_Neon_Float32)
{
    BOOST_TEST(ConstantUsageFloat32Test(defaultBackends));
}

#if defined(ARMNNREF_ENABLED)

// This test unit needs the reference backend, it's not available if the reference backend is not built

BOOST_AUTO_TEST_CASE(FallbackToCpuRef)
{
    using namespace armnn;

    // Create runtime in which test will run and allow fallback to CpuRef.
    IRuntime::CreationOptions options;
    IRuntimePtr runtime(IRuntime::Create(options));

    // Builds up the structure of the network.
    INetworkPtr net(INetwork::Create());

    IConnectableLayer* input = net->AddInputLayer(0);

    // This layer configuration isn't supported by CpuAcc but we allow fallback to CpuRef so it shoud pass.
    NormalizationDescriptor descriptor;
    IConnectableLayer* pooling = net->AddNormalizationLayer(descriptor);

    IConnectableLayer* output = net->AddOutputLayer(0);

    input->GetOutputSlot(0).Connect(pooling->GetInputSlot(0));
    pooling->GetOutputSlot(0).Connect(output->GetInputSlot(0));

    input->GetOutputSlot(0).SetTensorInfo(TensorInfo({ 1, 1, 4, 4 }, DataType::Float32));
    pooling->GetOutputSlot(0).SetTensorInfo(TensorInfo({ 1, 1, 4, 4 }, DataType::Float32));

    // optimize the network
    std::vector<BackendId> backends = {Compute::CpuAcc, Compute::CpuRef};
    IOptimizedNetworkPtr optNet = Optimize(*net, backends, runtime->GetDeviceSpec());

    // Load it into the runtime. It should pass.
    NetworkId netId;
    BOOST_TEST(runtime->LoadNetwork(netId, std::move(optNet)) == Status::Success);
}

#endif

BOOST_AUTO_TEST_CASE(NeonGreaterSimpleEndToEndTest)
{
    const std::vector<uint8_t> expectedOutput({ 0, 0, 0, 0,  1, 1, 1, 1,
                                                0, 0, 0, 0,  0, 0, 0, 0 });

    ComparisonSimpleEndToEnd<armnn::DataType::Float32>(defaultBackends,
                                                       ComparisonOperation::Greater,
                                                       expectedOutput);
}

BOOST_AUTO_TEST_CASE(NeonGreaterSimpleEndToEndUint8Test)
{
    const std::vector<uint8_t> expectedOutput({ 0, 0, 0, 0,  1, 1, 1, 1,
                                                0, 0, 0, 0,  0, 0, 0, 0 });

    ComparisonSimpleEndToEnd<armnn::DataType::QAsymmU8>(defaultBackends,
                                                               ComparisonOperation::Greater,
                                                               expectedOutput);
}

BOOST_AUTO_TEST_CASE(NeonGreaterBroadcastEndToEndTest)
{
    const std::vector<uint8_t> expectedOutput({ 0, 1, 0, 0, 0, 1,
                                                1, 1, 1, 1, 1, 1 });

    ComparisonBroadcastEndToEnd<armnn::DataType::Float32>(defaultBackends,
                                                          ComparisonOperation::Greater,
                                                          expectedOutput);
}

BOOST_AUTO_TEST_CASE(NeonGreaterBroadcastEndToEndUint8Test)
{
    const std::vector<uint8_t> expectedOutput({ 0, 1, 0, 0, 0, 1,
                                                1, 1, 1, 1, 1, 1 });

    ComparisonBroadcastEndToEnd<armnn::DataType::QAsymmU8>(defaultBackends,
                                                                  ComparisonOperation::Greater,
                                                                  expectedOutput);
}

BOOST_AUTO_TEST_CASE(NeonConcatEndToEndDim0Test)
{
    ConcatDim0EndToEnd<armnn::DataType::Float32>(defaultBackends);
}

BOOST_AUTO_TEST_CASE(NeonConcatEndToEndDim0Uint8Test)
{
    ConcatDim0EndToEnd<armnn::DataType::QAsymmU8>(defaultBackends);
}

BOOST_AUTO_TEST_CASE(NeonConcatEndToEndDim1Test)
{
    ConcatDim1EndToEnd<armnn::DataType::Float32>(defaultBackends);
}

BOOST_AUTO_TEST_CASE(NeonConcatEndToEndDim1Uint8Test)
{
    ConcatDim1EndToEnd<armnn::DataType::QAsymmU8>(defaultBackends);
}

BOOST_AUTO_TEST_CASE(NeonConcatEndToEndDim3Test)
{
    ConcatDim3EndToEnd<armnn::DataType::Float32>(defaultBackends);
}

BOOST_AUTO_TEST_CASE(NeonConcatEndToEndDim3Uint8Test)
{
    ConcatDim3EndToEnd<armnn::DataType::QAsymmU8>(defaultBackends);
}

// DepthToSpace
BOOST_AUTO_TEST_CASE(DephtToSpaceEndToEndNchwFloat32)
{
    DepthToSpaceEndToEnd<armnn::DataType::Float32>(defaultBackends, armnn::DataLayout::NCHW);
}

BOOST_AUTO_TEST_CASE(DephtToSpaceEndToEndNchwFloat16)
{
    DepthToSpaceEndToEnd<armnn::DataType::Float16>(defaultBackends, armnn::DataLayout::NCHW);
}

BOOST_AUTO_TEST_CASE(DephtToSpaceEndToEndNchwUint8)
{
    DepthToSpaceEndToEnd<armnn::DataType::QAsymmU8>(defaultBackends, armnn::DataLayout::NCHW);
}

BOOST_AUTO_TEST_CASE(DephtToSpaceEndToEndNchwInt16)
{
    DepthToSpaceEndToEnd<armnn::DataType::QSymmS16>(defaultBackends, armnn::DataLayout::NCHW);
}

BOOST_AUTO_TEST_CASE(DephtToSpaceEndToEndNhwcFloat32)
{
    DepthToSpaceEndToEnd<armnn::DataType::Float32>(defaultBackends, armnn::DataLayout::NHWC);
}

BOOST_AUTO_TEST_CASE(DephtToSpaceEndToEndNhwcFloat16)
{
    DepthToSpaceEndToEnd<armnn::DataType::Float16>(defaultBackends, armnn::DataLayout::NHWC);
}

BOOST_AUTO_TEST_CASE(DephtToSpaceEndToEndNhwcUint8)
{
    DepthToSpaceEndToEnd<armnn::DataType::QAsymmU8>(defaultBackends, armnn::DataLayout::NHWC);
}

BOOST_AUTO_TEST_CASE(DephtToSpaceEndToEndNhwcInt16)
{
    DepthToSpaceEndToEnd<armnn::DataType::QSymmS16>(defaultBackends, armnn::DataLayout::NHWC);
}

// Dequantize
BOOST_AUTO_TEST_CASE(DequantizeEndToEndSimpleTest)
{
    DequantizeEndToEndSimple<armnn::DataType::QAsymmU8>(defaultBackends);
}

BOOST_AUTO_TEST_CASE(DequantizeEndToEndOffsetTest)
{
    DequantizeEndToEndOffset<armnn::DataType::QAsymmU8>(defaultBackends);
}

BOOST_AUTO_TEST_CASE(NeonPreluEndToEndFloat32Test)
{
    PreluEndToEndNegativeTest<armnn::DataType::Float32>(defaultBackends);
}

BOOST_AUTO_TEST_CASE(NeonPreluEndToEndTestUint8Test)
{
    PreluEndToEndPositiveTest<armnn::DataType::QAsymmU8>(defaultBackends);
}

BOOST_AUTO_TEST_CASE(NeonSpaceToDepthNhwcEndToEndTest1)
{
    SpaceToDepthNhwcEndToEndTest1(defaultBackends);
}

BOOST_AUTO_TEST_CASE(NeonSpaceToDepthNchwEndToEndTest1)
{
    SpaceToDepthNchwEndToEndTest1(defaultBackends);
}

BOOST_AUTO_TEST_CASE(NeonSpaceToDepthNhwcEndToEndTest2)
{
    SpaceToDepthNhwcEndToEndTest2(defaultBackends);
}

BOOST_AUTO_TEST_CASE(NeonSpaceToDepthNchwEndToEndTest2)
{
    SpaceToDepthNchwEndToEndTest2(defaultBackends);
}

BOOST_AUTO_TEST_CASE(NeonSplitter1dEndToEndTest)
{
    Splitter1dEndToEnd<armnn::DataType::Float32>(defaultBackends);
}

BOOST_AUTO_TEST_CASE(NeonSplitter1dEndToEndUint8Test)
{
    Splitter1dEndToEnd<armnn::DataType::QAsymmU8>(defaultBackends);
}

BOOST_AUTO_TEST_CASE(NeonSplitter2dDim0EndToEndTest)
{
    Splitter2dDim0EndToEnd<armnn::DataType::Float32>(defaultBackends);
}

BOOST_AUTO_TEST_CASE(NeonSplitter2dDim1EndToEndTest)
{
    Splitter2dDim1EndToEnd<armnn::DataType::Float32>(defaultBackends);
}

BOOST_AUTO_TEST_CASE(NeonSplitter2dDim0EndToEndUint8Test)
{
    Splitter2dDim0EndToEnd<armnn::DataType::QAsymmU8>(defaultBackends);
}

BOOST_AUTO_TEST_CASE(NeonSplitter2dDim1EndToEndUint8Test)
{
    Splitter2dDim1EndToEnd<armnn::DataType::QAsymmU8>(defaultBackends);
}

BOOST_AUTO_TEST_CASE(NeonSplitter3dDim0EndToEndTest)
{
    Splitter3dDim0EndToEnd<armnn::DataType::Float32>(defaultBackends);
}

BOOST_AUTO_TEST_CASE(NeonSplitter3dDim1EndToEndTest)
{
    Splitter3dDim1EndToEnd<armnn::DataType::Float32>(defaultBackends);
}

BOOST_AUTO_TEST_CASE(NeonSplitter3dDim2EndToEndTest)
{
    Splitter3dDim2EndToEnd<armnn::DataType::Float32>(defaultBackends);
}

BOOST_AUTO_TEST_CASE(NeonSplitter3dDim0EndToEndUint8Test)
{
    Splitter3dDim0EndToEnd<armnn::DataType::QAsymmU8>(defaultBackends);
}

BOOST_AUTO_TEST_CASE(NeonSplitter3dDim1EndToEndUint8Test)
{
    Splitter3dDim1EndToEnd<armnn::DataType::QAsymmU8>(defaultBackends);
}

BOOST_AUTO_TEST_CASE(NeonSplitter3dDim2EndToEndUint8Test)
{
    Splitter3dDim2EndToEnd<armnn::DataType::QAsymmU8>(defaultBackends);
}

BOOST_AUTO_TEST_CASE(NeonSplitter4dDim0EndToEndTest)
{
    Splitter4dDim0EndToEnd<armnn::DataType::Float32>(defaultBackends);
}

BOOST_AUTO_TEST_CASE(NeonSplitter4dDim1EndToEndTest)
{
    Splitter4dDim1EndToEnd<armnn::DataType::Float32>(defaultBackends);
}

BOOST_AUTO_TEST_CASE(NeonSplitter4dDim2EndToEndTest)
{
    Splitter4dDim2EndToEnd<armnn::DataType::Float32>(defaultBackends);
}

BOOST_AUTO_TEST_CASE(NeonSplitter4dDim3EndToEndTest)
{
    Splitter4dDim3EndToEnd<armnn::DataType::Float32>(defaultBackends);
}

BOOST_AUTO_TEST_CASE(NeonSplitter4dDim0EndToEndUint8Test)
{
    Splitter4dDim0EndToEnd<armnn::DataType::QAsymmU8>(defaultBackends);
}

BOOST_AUTO_TEST_CASE(NeonSplitter4dDim1EndToEndUint8Test)
{
    Splitter4dDim1EndToEnd<armnn::DataType::QAsymmU8>(defaultBackends);
}

BOOST_AUTO_TEST_CASE(NeonSplitter4dDim2EndToEndUint8Test)
{
    Splitter4dDim2EndToEnd<armnn::DataType::QAsymmU8>(defaultBackends);
}

BOOST_AUTO_TEST_CASE(NeonSplitter4dDim3EndToEndUint8Test)
{
    Splitter4dDim3EndToEnd<armnn::DataType::QAsymmU8>(defaultBackends);
}

BOOST_AUTO_TEST_CASE(NeonQuantizedLstmEndToEndTest)
{
    QuantizedLstmEndToEnd(defaultBackends);
}

BOOST_AUTO_TEST_CASE(NeonTransposeConvolution2dEndToEndFloatNchwTest)
{
    TransposeConvolution2dEndToEnd<armnn::DataType::Float32, armnn::DataType::Float32>(
        defaultBackends, armnn::DataLayout::NCHW);
}

BOOST_AUTO_TEST_CASE(NeonTransposeConvolution2dEndToEndUint8NchwTest)
{
    TransposeConvolution2dEndToEnd<armnn::DataType::QAsymmU8, armnn::DataType::Signed32>(
        defaultBackends, armnn::DataLayout::NCHW);
}

BOOST_AUTO_TEST_CASE(NeonTransposeConvolution2dEndToEndFloatNhwcTest)
{
    TransposeConvolution2dEndToEnd<armnn::DataType::Float32, armnn::DataType::Float32>(
        defaultBackends, armnn::DataLayout::NHWC);
}

BOOST_AUTO_TEST_CASE(NeonTransposeConvolution2dEndToEndUint8NhwcTest)
{
    TransposeConvolution2dEndToEnd<armnn::DataType::QAsymmU8, armnn::DataType::Signed32>(
        defaultBackends, armnn::DataLayout::NHWC);
}

BOOST_AUTO_TEST_CASE(NeonImportNonAlignedInputPointerTest)
{
    ImportNonAlignedInputPointerTest(defaultBackends);
}

BOOST_AUTO_TEST_CASE(NeonExportNonAlignedOutputPointerTest)
{
    ExportNonAlignedOutputPointerTest(defaultBackends);
}

BOOST_AUTO_TEST_CASE(NeonImportAlignedPointerTest, * boost::unit_test::disabled())
{
    ImportAlignedPointerTest(defaultBackends);
}

BOOST_AUTO_TEST_CASE(NeonImportOnlyWorkload, * boost::unit_test::disabled())
{
    ImportOnlyWorkload(defaultBackends);
}

BOOST_AUTO_TEST_CASE(NeonExportOnlyWorkload, * boost::unit_test::disabled())
{
    ExportOnlyWorkload(defaultBackends);
}

BOOST_AUTO_TEST_CASE(NeonImportAndExportWorkload, * boost::unit_test::disabled())
{
    ImportAndExportWorkload(defaultBackends);
}

BOOST_AUTO_TEST_CASE(NeonExportOutputWithSeveralOutputSlotConnectionsTest, * boost::unit_test::disabled())
{
    ExportOutputWithSeveralOutputSlotConnectionsTest(defaultBackends);
}

// InstanceNormalization
BOOST_AUTO_TEST_CASE(NeonInstanceNormalizationNchwEndToEndTest1)
{
    InstanceNormalizationNchwEndToEndTest1(defaultBackends);
}

BOOST_AUTO_TEST_CASE(NeonInstanceNormalizationNchwEndToEndTest2)
{
    InstanceNormalizationNchwEndToEndTest2(defaultBackends);
}

// ArgMinMax
BOOST_AUTO_TEST_CASE(NeonArgMaxSimpleTest)
{
    ArgMaxEndToEndSimple<armnn::DataType::Float32>(defaultBackends);
}

BOOST_AUTO_TEST_CASE(NeonArgMinSimpleTest)
{
    ArgMinEndToEndSimple<armnn::DataType::Float32>(defaultBackends);
}

BOOST_AUTO_TEST_CASE(NeonArgMaxAxis0Test)
{
    ArgMaxAxis0EndToEnd<armnn::DataType::Float32>(defaultBackends);
}

BOOST_AUTO_TEST_CASE(NeonArgMinAxis0Test)
{
    ArgMinAxis0EndToEnd<armnn::DataType::Float32>(defaultBackends);
}

BOOST_AUTO_TEST_CASE(NeonArgMaxAxis1Test)
{
    ArgMaxAxis1EndToEnd<armnn::DataType::Float32>(defaultBackends);
}

BOOST_AUTO_TEST_CASE(NeonArgMinAxis1Test)
{
    ArgMinAxis1EndToEnd<armnn::DataType::Float32>(defaultBackends);
}

BOOST_AUTO_TEST_CASE(NeonArgMaxAxis2Test)
{
    ArgMaxAxis2EndToEnd<armnn::DataType::Float32>(defaultBackends);
}

BOOST_AUTO_TEST_CASE(NeonArgMinAxis2Test)
{
    ArgMinAxis2EndToEnd<armnn::DataType::Float32>(defaultBackends);
}

BOOST_AUTO_TEST_CASE(NeonArgMaxAxis3Test)
{
    ArgMaxAxis3EndToEnd<armnn::DataType::Float32>(defaultBackends);
}

BOOST_AUTO_TEST_CASE(NeonArgMinAxis3Test)
{
    ArgMinAxis3EndToEnd<armnn::DataType::Float32>(defaultBackends);
}

BOOST_AUTO_TEST_CASE(NeonArgMaxSimpleTestQuantisedAsymm8)
{
    ArgMaxEndToEndSimple<armnn::DataType::QAsymmU8>(defaultBackends);
}

BOOST_AUTO_TEST_CASE(NeonArgMinSimpleTestQuantisedAsymm8)
{
    ArgMinEndToEndSimple<armnn::DataType::QAsymmU8>(defaultBackends);
}

BOOST_AUTO_TEST_CASE(NeonArgMaxAxis0TestQuantisedAsymm8)
{
    ArgMaxAxis0EndToEnd<armnn::DataType::QAsymmU8>(defaultBackends);
}

BOOST_AUTO_TEST_CASE(NeonArgMinAxis0TestQuantisedAsymm8)
{
    ArgMinAxis0EndToEnd<armnn::DataType::QAsymmU8>(defaultBackends);
}

BOOST_AUTO_TEST_CASE(NeonArgMaxAxis1TestQuantisedAsymm8)
{
    ArgMaxAxis1EndToEnd<armnn::DataType::QAsymmU8>(defaultBackends);
}

BOOST_AUTO_TEST_CASE(NeonArgMinAxis1TestQuantisedAsymm8)
{
    ArgMinAxis1EndToEnd<armnn::DataType::QAsymmU8>(defaultBackends);
}

BOOST_AUTO_TEST_CASE(NeonArgMaxAxis2TestQuantisedAsymm8)
{
    ArgMaxAxis2EndToEnd<armnn::DataType::QAsymmU8>(defaultBackends);
}

BOOST_AUTO_TEST_CASE(NeonArgMinAxis2TestQuantisedAsymm8)
{
    ArgMinAxis2EndToEnd<armnn::DataType::QAsymmU8>(defaultBackends);
}

BOOST_AUTO_TEST_CASE(NeonArgMaxAxis3TestQuantisedAsymm8)
{
    ArgMaxAxis3EndToEnd<armnn::DataType::QAsymmU8>(defaultBackends);
}

BOOST_AUTO_TEST_CASE(NeonArgMinAxis3TestQuantisedAsymm8)
{
    ArgMinAxis3EndToEnd<armnn::DataType::QAsymmU8>(defaultBackends);
}

BOOST_AUTO_TEST_CASE(NeonDetectionPostProcessRegularNmsTest, * boost::unit_test::disabled())
{
    std::vector<float> boxEncodings({
                                        0.0f, 0.0f, 0.0f, 0.0f,
                                        0.0f, 1.0f, 0.0f, 0.0f,
                                        0.0f, -1.0f, 0.0f, 0.0f,
                                        0.0f, 0.0f, 0.0f, 0.0f,
                                        0.0f, 1.0f, 0.0f, 0.0f,
                                        0.0f, 0.0f, 0.0f, 0.0f
                                    });
    std::vector<float> scores({
                                  0.0f, 0.9f, 0.8f,
                                  0.0f, 0.75f, 0.72f,
                                  0.0f, 0.6f, 0.5f,
                                  0.0f, 0.93f, 0.95f,
                                  0.0f, 0.5f, 0.4f,
                                  0.0f, 0.3f, 0.2f
                              });
    std::vector<float> anchors({
                                   0.5f, 0.5f, 1.0f, 1.0f,
                                   0.5f, 0.5f, 1.0f, 1.0f,
                                   0.5f, 0.5f, 1.0f, 1.0f,
                                   0.5f, 10.5f, 1.0f, 1.0f,
                                   0.5f, 10.5f, 1.0f, 1.0f,
                                   0.5f, 100.5f, 1.0f, 1.0f
                               });
    DetectionPostProcessRegularNmsEndToEnd<armnn::DataType::Float32>(defaultBackends, boxEncodings, scores, anchors);
}

inline void QuantizeData(uint8_t* quant, const float* dequant, const TensorInfo& info)
{
    for (size_t i = 0; i < info.GetNumElements(); i++)
    {
        quant[i] = armnn::Quantize<uint8_t>(dequant[i], info.GetQuantizationScale(), info.GetQuantizationOffset());
    }
}

BOOST_AUTO_TEST_CASE(NeonDetectionPostProcessRegularNmsUint8Test, * boost::unit_test::disabled())
{
    armnn::TensorInfo boxEncodingsInfo({ 1, 6, 4 }, armnn::DataType::Float32);
    armnn::TensorInfo scoresInfo({ 1, 6, 3 }, armnn::DataType::Float32);
    armnn::TensorInfo anchorsInfo({ 6, 4 }, armnn::DataType::Float32);

    boxEncodingsInfo.SetQuantizationScale(1.0f);
    boxEncodingsInfo.SetQuantizationOffset(1);
    scoresInfo.SetQuantizationScale(0.01f);
    scoresInfo.SetQuantizationOffset(0);
    anchorsInfo.SetQuantizationScale(0.5f);
    anchorsInfo.SetQuantizationOffset(0);

    std::vector<float> boxEncodings({
                                        0.0f, 0.0f, 0.0f, 0.0f,
                                        0.0f, 1.0f, 0.0f, 0.0f,
                                        0.0f, -1.0f, 0.0f, 0.0f,
                                        0.0f, 0.0f, 0.0f, 0.0f,
                                        0.0f, 1.0f, 0.0f, 0.0f,
                                        0.0f, 0.0f, 0.0f, 0.0f
                                    });
    std::vector<float> scores({
                                  0.0f, 0.9f, 0.8f,
                                  0.0f, 0.75f, 0.72f,
                                  0.0f, 0.6f, 0.5f,
                                  0.0f, 0.93f, 0.95f,
                                  0.0f, 0.5f, 0.4f,
                                  0.0f, 0.3f, 0.2f
                              });
    std::vector<float> anchors({
                                   0.5f, 0.5f, 1.0f, 1.0f,
                                   0.5f, 0.5f, 1.0f, 1.0f,
                                   0.5f, 0.5f, 1.0f, 1.0f,
                                   0.5f, 10.5f, 1.0f, 1.0f,
                                   0.5f, 10.5f, 1.0f, 1.0f,
                                   0.5f, 100.5f, 1.0f, 1.0f
                               });

    std::vector<uint8_t> qBoxEncodings(boxEncodings.size(), 0);
    std::vector<uint8_t> qScores(scores.size(), 0);
    std::vector<uint8_t> qAnchors(anchors.size(), 0);
    QuantizeData(qBoxEncodings.data(), boxEncodings.data(), boxEncodingsInfo);
    QuantizeData(qScores.data(), scores.data(), scoresInfo);
    QuantizeData(qAnchors.data(), anchors.data(), anchorsInfo);
    DetectionPostProcessRegularNmsEndToEnd<armnn::DataType::QAsymmU8>(defaultBackends, qBoxEncodings,
                                                                             qScores, qAnchors,
                                                                             1.0f, 1, 0.01f, 0, 0.5f, 0);
}

BOOST_AUTO_TEST_CASE(NeonDetectionPostProcessFastNmsTest, * boost::unit_test::disabled())
{
    std::vector<float> boxEncodings({
                                        0.0f, 0.0f, 0.0f, 0.0f,
                                        0.0f, 1.0f, 0.0f, 0.0f,
                                        0.0f, -1.0f, 0.0f, 0.0f,
                                        0.0f, 0.0f, 0.0f, 0.0f,
                                        0.0f, 1.0f, 0.0f, 0.0f,
                                        0.0f, 0.0f, 0.0f, 0.0f
                                    });
    std::vector<float> scores({
                                  0.0f, 0.9f, 0.8f,
                                  0.0f, 0.75f, 0.72f,
                                  0.0f, 0.6f, 0.5f,
                                  0.0f, 0.93f, 0.95f,
                                  0.0f, 0.5f, 0.4f,
                                  0.0f, 0.3f, 0.2f
                              });
    std::vector<float> anchors({
                                   0.5f, 0.5f, 1.0f, 1.0f,
                                   0.5f, 0.5f, 1.0f, 1.0f,
                                   0.5f, 0.5f, 1.0f, 1.0f,
                                   0.5f, 10.5f, 1.0f, 1.0f,
                                   0.5f, 10.5f, 1.0f, 1.0f,
                                   0.5f, 100.5f, 1.0f, 1.0f
                               });
    DetectionPostProcessFastNmsEndToEnd<armnn::DataType::Float32>(defaultBackends, boxEncodings, scores, anchors);
}

BOOST_AUTO_TEST_CASE(NeonDetectionPostProcessFastNmsUint8Test, * boost::unit_test::disabled())
{
    armnn::TensorInfo boxEncodingsInfo({ 1, 6, 4 }, armnn::DataType::Float32);
    armnn::TensorInfo scoresInfo({ 1, 6, 3 }, armnn::DataType::Float32);
    armnn::TensorInfo anchorsInfo({ 6, 4 }, armnn::DataType::Float32);

    boxEncodingsInfo.SetQuantizationScale(1.0f);
    boxEncodingsInfo.SetQuantizationOffset(1);
    scoresInfo.SetQuantizationScale(0.01f);
    scoresInfo.SetQuantizationOffset(0);
    anchorsInfo.SetQuantizationScale(0.5f);
    anchorsInfo.SetQuantizationOffset(0);

    std::vector<float> boxEncodings({
                                        0.0f, 0.0f, 0.0f, 0.0f,
                                        0.0f, 1.0f, 0.0f, 0.0f,
                                        0.0f, -1.0f, 0.0f, 0.0f,
                                        0.0f, 0.0f, 0.0f, 0.0f,
                                        0.0f, 1.0f, 0.0f, 0.0f,
                                        0.0f, 0.0f, 0.0f, 0.0f
                                    });
    std::vector<float> scores({
                                  0.0f, 0.9f, 0.8f,
                                  0.0f, 0.75f, 0.72f,
                                  0.0f, 0.6f, 0.5f,
                                  0.0f, 0.93f, 0.95f,
                                  0.0f, 0.5f, 0.4f,
                                  0.0f, 0.3f, 0.2f
                              });
    std::vector<float> anchors({
                                   0.5f, 0.5f, 1.0f, 1.0f,
                                   0.5f, 0.5f, 1.0f, 1.0f,
                                   0.5f, 0.5f, 1.0f, 1.0f,
                                   0.5f, 10.5f, 1.0f, 1.0f,
                                   0.5f, 10.5f, 1.0f, 1.0f,
                                   0.5f, 100.5f, 1.0f, 1.0f
                               });

    std::vector<uint8_t> qBoxEncodings(boxEncodings.size(), 0);
    std::vector<uint8_t> qScores(scores.size(), 0);
    std::vector<uint8_t> qAnchors(anchors.size(), 0);
    QuantizeData(qBoxEncodings.data(), boxEncodings.data(), boxEncodingsInfo);
    QuantizeData(qScores.data(), scores.data(), scoresInfo);
    QuantizeData(qAnchors.data(), anchors.data(), anchorsInfo);
    DetectionPostProcessFastNmsEndToEnd<armnn::DataType::QAsymmU8>(defaultBackends, qBoxEncodings,
                                                                          qScores, qAnchors,
                                                                          1.0f, 1, 0.01f, 0, 0.5f, 0);
}

BOOST_AUTO_TEST_SUITE_END()
