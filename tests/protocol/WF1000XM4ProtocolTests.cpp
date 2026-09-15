#include <catch2/catch_test_macros.hpp>
#include "sony/protocol/ProtocolV2.h"
#include "sony/protocol/FrameCodec.h"
#include "sony/transport/FakeTransport.h"
#include "ReplyingFakeTransport.h"

using namespace sony;
using namespace sony::protocol;
using namespace sony::transport;
using sony::test::ReplyingFakeTransport;

// WF-1000XM4 uses the older NCASM 0x15 layout documented by the
// model-specific sony-headphones-ctl implementation, including the wind byte.
TEST_CASE("WF-1000XM4: noise control uses 0x6815 layout", "[protocol][v2][wf1000xm4]")
{
    ReplyingFakeTransport fake;
    SonyProtocolSession session(&fake);
    session.connect("11:22:33:44:55:66");

    ProtocolV2 v2(session, SonyModel::WF1000XM4);

    SECTION("ambient level is placed after the wind and voice fields")
    {
        fake.queueReply({ SonyFrame{ .type = DataType::Ack, .sequence = 0 } });

        NoiseControlState state{
            .mode = NoiseControlMode::Ambient,
            .ambientLevel = 10,
            .focusOnVoice = false
        };
        v2.setNoiseControl(state);

        REQUIRE(fake.sentCount() == 1);
        auto sent = FrameCodec::decode(fake.lastSentFrame());
        REQUIRE(sent.payload == std::vector<uint8_t>{
            0x68, 0x15, 0x01, 0x01, 0x01, 0x02, 0x00, 0x0a
        });
    }

    SECTION("ANC on uses noise-cancelling mode and no-wind field")
    {
        fake.queueReply({ SonyFrame{ .type = DataType::Ack, .sequence = 0 } });

        NoiseControlState state{
            .mode = NoiseControlMode::NoiseCancelling,
            .ambientLevel = 0,
            .focusOnVoice = false
        };
        v2.setNoiseControl(state);

        REQUIRE(fake.sentCount() == 1);
        auto sent = FrameCodec::decode(fake.lastSentFrame());
        REQUIRE(sent.payload == std::vector<uint8_t>{
            0x68, 0x15, 0x01, 0x01, 0x00, 0x02, 0x00, 0x01
        });
    }
}
