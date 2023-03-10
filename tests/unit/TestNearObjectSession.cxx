
#include <array>
#include <condition_variable>
#include <chrono>
#include <memory>
#include <mutex>

#include <catch2/catch.hpp>

#include <nearobject/NearObjectSession.hxx>
#include <nearobject/NearObjectSessionEventCallbacks.hxx>

namespace nearobject
{
namespace test
{
static constexpr NearObjectCapabilities AllCapabilitiesSupported = {
    true, // SupportsRanging
    true, // SupportsPositioning
    true, // SupportsSecureDevice
    true, // SupportsSecureChannels
};

static const std::vector<std::shared_ptr<NearObject>> NearObjectsContainerEmpty{};
static const std::vector<std::shared_ptr<NearObject>> NearObjectsContainerSingle{ std::make_shared<NearObject>() };
static const std::vector<std::shared_ptr<NearObject>> NearObjectsContainerMultiple{ 
    std::make_shared<NearObject>(), 
    std::make_shared<NearObject>(), 
    std::make_shared<NearObject>(),
    std::make_shared<NearObject>(),
    std::make_shared<NearObject>(),
};

struct NearObjectSessionEventCallbacksNoop :
    public NearObjectSessionEventCallbacks
{
    virtual void
    OnSessionEnded(NearObjectSession *) override
    {}

    virtual void
    OnRangingStarted(NearObjectSession *) override
    {}

    virtual void
    OnRangingStopped(NearObjectSession *) override
    {}

    virtual void
    OnNearObjectPropertiesChanged(NearObjectSession *, const std::vector<std::shared_ptr<NearObject>>) override
    {}

    virtual void
    OnSessionMembershipChanged(NearObjectSession *, const std::vector<std::shared_ptr<NearObject>>, const std::vector<std::shared_ptr<NearObject>>) override
    {}
};

struct NearObjectSessionTest final :
    public NearObjectSession
{
    // Forward base class constructor.
    using NearObjectSession::NearObjectSession;

    // Make some protected members public for testing purposes.
    using NearObjectSession::GetNearObjects;
    using NearObjectSession::AddNearObject;
    using NearObjectSession::AddNearObjects;
    using NearObjectSession::RemoveNearObject;
    using NearObjectSession::RemoveNearObjects;
    using NearObjectSession::NearObjectPropertiesChanged;
    using NearObjectSession::EndSession;
};

} // namespace test
} // namespace nearobject

TEST_CASE("near objects  can be added at session creation", "[basic]")
{
    using namespace nearobject;

    const auto callbacksNoop = std::make_shared<test::NearObjectSessionEventCallbacksNoop>();
    const auto ValidateInitialList = [&](const std::vector<std::shared_ptr<NearObject>>& nearObjectsInitial) {
        test::NearObjectSessionTest session(test::AllCapabilitiesSupported, nearObjectsInitial, callbacksNoop);
        REQUIRE(session.GetNearObjects() == nearObjectsInitial);
    };

    SECTION("creation with 0 near objects doesn't cause a crash")
    {
        REQUIRE_NOTHROW(std::make_unique<NearObjectSession>(test::AllCapabilitiesSupported, test::NearObjectsContainerEmpty, callbacksNoop));
    }

    SECTION("creation with 1 near object doesn't cause a crash")
    {
        REQUIRE_NOTHROW(std::make_unique<NearObjectSession>(test::AllCapabilitiesSupported, test::NearObjectsContainerSingle, callbacksNoop));
    }

    SECTION("creation with 1+ near objects doesn't cause a crash")
    {
        REQUIRE_NOTHROW(std::make_unique<NearObjectSession>(test::AllCapabilitiesSupported, test::NearObjectsContainerMultiple, callbacksNoop));
    }

    SECTION("membership reflects 0 near objects specified upon construction")
    {
        ValidateInitialList({});
    }

    SECTION("membership reflects 1 near object specified upon construction")
    {
        ValidateInitialList(test::NearObjectsContainerSingle);
    }

    SECTION("membership reflects 1+ near objects specified upon construction")
    {
        ValidateInitialList(test::NearObjectsContainerMultiple);
    }
}

TEST_CASE("near object session capabilities are accurate", "[basic]")
{
    using namespace nearobject;

    const auto &capabilities = test::AllCapabilitiesSupported;

    SECTION("capabilities match post-creation")
    {
        const auto callbacksNoop = std::make_shared<test::NearObjectSessionEventCallbacksNoop>();
        NearObjectSession session(capabilities, {}, callbacksNoop);
        REQUIRE(session.GetCapabilities() == capabilities);
    }

    SECTION("capabilities don't change during a session")
    {
        const auto callbacksNoop = std::make_shared<test::NearObjectSessionEventCallbacksNoop>();
        NearObjectSession session(capabilities, {}, callbacksNoop);
        session.StartRanging();
        REQUIRE(session.GetCapabilities() == capabilities);
        session.StopRanging();
        REQUIRE(session.GetCapabilities() == capabilities);
    }
}

TEST_CASE("near object session ranging state is correctly reflected", "[basic]")
{
    using namespace nearobject;

    const auto &capabilities = test::AllCapabilitiesSupported;

    SECTION("session indicates ranging is inactive upon construction")
    {
        const auto callbacksNoop = std::make_shared<test::NearObjectSessionEventCallbacksNoop>();
        NearObjectSession session(capabilities, {}, callbacksNoop);
        REQUIRE(!session.IsRangingActive());
    }

    SECTION("session indicates ranging is active after ranging is started")
    {
        const auto callbacksNoop = std::make_shared<test::NearObjectSessionEventCallbacksNoop>();
        NearObjectSession session(capabilities, {}, callbacksNoop);
        session.StartRanging();
        REQUIRE(session.IsRangingActive());
    }

    SECTION("session indicates ranging is active after ranging is started repeatedly")
    {
        const auto callbacksNoop = std::make_shared<test::NearObjectSessionEventCallbacksNoop>();
        NearObjectSession session(capabilities, {}, callbacksNoop);
        session.StartRanging();
        REQUIRE(session.IsRangingActive());
        session.StartRanging();
        REQUIRE(session.IsRangingActive());
    }

    SECTION("session indicates ranging is inactive after ranging is stopped")
    {
        const auto callbacksNoop = std::make_shared<test::NearObjectSessionEventCallbacksNoop>();
        NearObjectSession session(capabilities, {}, callbacksNoop);
        session.StartRanging();
        session.StopRanging();
        REQUIRE(!session.IsRangingActive());
    }

    SECTION("session indicates ranging is active after ranging is stopped repeatedly")
    {
        const auto callbacksNoop = std::make_shared<test::NearObjectSessionEventCallbacksNoop>();
        NearObjectSession session(capabilities, {}, callbacksNoop);
        session.StartRanging();
        session.StopRanging();
        REQUIRE(!session.IsRangingActive());
        session.StopRanging();
        REQUIRE(!session.IsRangingActive());
    }

    SECTION("session indicates ranging is active after starting subsequent ranging is started")
    {
        const auto callbacksNoop = std::make_shared<test::NearObjectSessionEventCallbacksNoop>();
        NearObjectSession session(capabilities, {}, callbacksNoop);
        session.StartRanging();
        session.StopRanging();
        session.StartRanging();
        REQUIRE(session.IsRangingActive());
    }

    SECTION("session indicates consistent ranging state after ranging is started")
    {
        const auto callbacksNoop = std::make_shared<test::NearObjectSessionEventCallbacksNoop>();
        NearObjectSession session(capabilities, {}, callbacksNoop);
        session.StartRanging();
        REQUIRE(session.IsRangingActive());
        REQUIRE(session.IsRangingActive());
    }

    SECTION("session indicates consistent ranging state after ranging is stoppted")
    {
        const auto callbacksNoop = std::make_shared<test::NearObjectSessionEventCallbacksNoop>();
        NearObjectSession session(capabilities, {}, callbacksNoop);
        session.StartRanging();
        session.StopRanging();
        REQUIRE(!session.IsRangingActive());
        REQUIRE(!session.IsRangingActive());
    }
}

TEST_CASE("near object event handlers can be registered", "[basic]")
{
    using namespace nearobject;

    SECTION("registering a handler doesn't cause a crash")
    {
        const auto callbacksNoop = std::make_shared<test::NearObjectSessionEventCallbacksNoop>();
        REQUIRE_NOTHROW(std::make_unique<NearObjectSession>(test::AllCapabilitiesSupported, std::vector<std::shared_ptr<NearObject>>{}, callbacksNoop));
    }

    SECTION("starting and ending a ranging session doesn't cause a crash")
    {
        const auto callbacksNoop = std::make_shared<test::NearObjectSessionEventCallbacksNoop>();
        NearObjectSession session(test::AllCapabilitiesSupported, std::vector<std::shared_ptr<NearObject>>{}, callbacksNoop);
        REQUIRE_NOTHROW([&]() {
            session.StartRanging();
            session.StopRanging();
        }());
    }

    SECTION("handler being deleted doesn't cause a crash")
    {
        auto callbacksNoop = std::make_shared<test::NearObjectSessionEventCallbacksNoop>();
        NearObjectSession session(test::AllCapabilitiesSupported, std::vector<std::shared_ptr<NearObject>>{}, callbacksNoop);
        session.StartRanging();
        callbacksNoop.reset();
        REQUIRE_NOTHROW(session.StopRanging());
    }

    SECTION("callbacks provide the session pointer for which it was registered")
    {
        struct NearObjectSessionEventCallbacksCheckSessionPointer final :
            public NearObjectSessionEventCallbacks
        {
            void
            OnSessionEnded(NearObjectSession *session) override
            {
                REQUIRE(Session == session);
            }

            void
            OnRangingStarted(NearObjectSession *session) override
            {
                REQUIRE(Session == session);
            }

            void
            OnRangingStopped(NearObjectSession *session) override
            {
                REQUIRE(Session == session);
            }

            void
            OnNearObjectPropertiesChanged(NearObjectSession *session, const std::vector<std::shared_ptr<NearObject>> /* nearObjectsChanged */) override
            {
                REQUIRE(Session == session);
            }

            void
            OnSessionMembershipChanged(NearObjectSession *session, const std::vector<std::shared_ptr<NearObject>> nearObjectsAdded, const std::vector<std::shared_ptr<NearObject>> nearObjectsRemoved) override
            {
                REQUIRE(Session == session);
            }

            NearObjectSession *Session{ nullptr };
        };

        const auto callbacks = std::make_shared<NearObjectSessionEventCallbacksCheckSessionPointer>();
        NearObjectSession session(test::AllCapabilitiesSupported, std::vector<std::shared_ptr<NearObject>>{}, callbacks);
        callbacks->Session = &session;
        session.StartRanging();
        session.StopRanging();
        // TODO: trigger NO property changes
        // TODO: trigger NO membership changes
    }
}

TEST_CASE("near object event handlers reflect near object changes", "[basic]")
{
    using namespace nearobject;
    using namespace std::chrono_literals;

    struct NearObjectSessionEventCallbacksTestMembershipChanged final :
        public test::NearObjectSessionEventCallbacksNoop
    {
        void
        OnSessionMembershipChanged(NearObjectSession * /* session */, const std::vector<std::shared_ptr<NearObject>> nearObjectsAdded, const std::vector<std::shared_ptr<NearObject>> nearObjectsRemoved) override
        {
            bool validationDone = false;
            if (ValidateAdded) {
                REQUIRE(nearObjectsAdded == NearObjectsAdded);
                validationDone = true;
            }
            if (ValidateRemoved) {
                REQUIRE(nearObjectsRemoved == NearObjectsRemoved);
                validationDone = true;
            }

            if (validationDone) {
                ValidationDone = true;
                m_validationDone.notify_all();
            }
        }

        std::vector<std::shared_ptr<NearObject>> NearObjectsAdded{};
        std::vector<std::shared_ptr<NearObject>> NearObjectsRemoved{};

        void
        WaitForValidationComplete()
        {
            auto validationIsDone = [&](){ return ValidationDone; };
            auto validationDoneLock = std::unique_lock{ m_validationDoneGate };
            REQUIRE(m_validationDone.wait_for(validationDoneLock, 100ms, validationIsDone));
        }

        bool ValidateAdded{ false };
        bool ValidateRemoved{ false };
        bool ValidationDone{ false };

    private:
        std::mutex m_validationDoneGate;
        std::condition_variable m_validationDone;
    };

    SECTION("membership changed event callback invoked with single near object added")
    {
        auto callbacks = std::make_shared<NearObjectSessionEventCallbacksTestMembershipChanged>();
        test::NearObjectSessionTest session(test::AllCapabilitiesSupported, std::vector<std::shared_ptr<NearObject>>{}, callbacks);

        callbacks->ValidateAdded = true;
        callbacks->NearObjectsAdded = test::NearObjectsContainerSingle;
        session.AddNearObject(test::NearObjectsContainerSingle[0]);
        REQUIRE(session.GetNearObjects().size() == callbacks->NearObjectsAdded.size());
        callbacks->WaitForValidationComplete();
    }

    SECTION("membership changed event callback invoked with multiple near object added")
    {
        auto callbacks = std::make_shared<NearObjectSessionEventCallbacksTestMembershipChanged>();
        test::NearObjectSessionTest session(test::AllCapabilitiesSupported, std::vector<std::shared_ptr<NearObject>>{}, callbacks);

        callbacks->ValidateAdded = true;
        callbacks->NearObjectsAdded = test::NearObjectsContainerMultiple;
        session.AddNearObjects(test::NearObjectsContainerMultiple);
        REQUIRE(session.GetNearObjects().size() == callbacks->NearObjectsAdded.size());
        callbacks->WaitForValidationComplete();
    }

    SECTION("membership changed event callback invoked with single near object removed")
    {
        auto callbacks = std::make_shared<NearObjectSessionEventCallbacksTestMembershipChanged>();
        test::NearObjectSessionTest session(test::AllCapabilitiesSupported, test::NearObjectsContainerSingle, callbacks);

        callbacks->NearObjectsRemoved = test::NearObjectsContainerSingle;
        callbacks->ValidateRemoved = true;
        session.RemoveNearObject(test::NearObjectsContainerSingle[0]);
        REQUIRE(session.GetNearObjects().size() == test::NearObjectsContainerSingle.size() - callbacks->NearObjectsRemoved.size());
        callbacks->WaitForValidationComplete();
    }

    SECTION("membership changed event callback invoked with multiple near objects removed")
    {
        auto callbacks = std::make_shared<NearObjectSessionEventCallbacksTestMembershipChanged>();
        test::NearObjectSessionTest session(test::AllCapabilitiesSupported, test::NearObjectsContainerMultiple, callbacks);

        callbacks->ValidateRemoved = true;
        callbacks->NearObjectsRemoved = test::NearObjectsContainerMultiple;
        session.RemoveNearObjects(test::NearObjectsContainerMultiple);
        REQUIRE(session.GetNearObjects().size() == test::NearObjectsContainerMultiple.size() - callbacks->NearObjectsRemoved.size());
        callbacks->WaitForValidationComplete();
    }

    SECTION("membership changed event callback invoked with subset of near objects removed")
    {
        const std::array<std::vector<std::shared_ptr<NearObject>>, 3> NearObjectsToRemove {
            // Subset overlaps start of near object membership.
            std::vector<std::shared_ptr<NearObject>>{ 
                std::cbegin(test::NearObjectsContainerMultiple), 
                std::next(std::cbegin(test::NearObjectsContainerMultiple), 2) 
            },
            // Subset is in middle of near object membership.
            std::vector<std::shared_ptr<NearObject>>{ 
                std::next(std::cbegin(test::NearObjectsContainerMultiple)), 
                std::prev(std::end(test::NearObjectsContainerMultiple), 2)
            },
            // Subset overlaps end of near object membership.
            std::vector<std::shared_ptr<NearObject>>{ 
                std::prev(std::cend(test::NearObjectsContainerMultiple), 3), 
                std::cend(test::NearObjectsContainerMultiple)
            }
        };

        auto callbacks = std::make_shared<NearObjectSessionEventCallbacksTestMembershipChanged>();
        callbacks->ValidateRemoved = true;

        for (const auto& nearObjectsToRemove : NearObjectsToRemove) {
            test::NearObjectSessionTest session(test::AllCapabilitiesSupported, test::NearObjectsContainerMultiple, callbacks);
            callbacks->NearObjectsRemoved = nearObjectsToRemove;
            session.RemoveNearObjects(nearObjectsToRemove);
            REQUIRE(session.GetNearObjects().size() == test::NearObjectsContainerMultiple.size() - callbacks->NearObjectsRemoved.size());
            callbacks->WaitForValidationComplete();
            callbacks->ValidationDone = false;
        }
    }

    SECTION("removing non-existent near objects doesn't change membership")
    {
        auto callbacks = std::make_shared<test::NearObjectSessionEventCallbacksNoop>();

        std::array<test::NearObjectSessionTest, 3> Sessions {
            test::NearObjectSessionTest(test::AllCapabilitiesSupported, {}, callbacks),
            test::NearObjectSessionTest(test::AllCapabilitiesSupported, test::NearObjectsContainerSingle, callbacks),
            test::NearObjectSessionTest(test::AllCapabilitiesSupported, test::NearObjectsContainerMultiple, callbacks),
        };

        for (auto& session : Sessions) {
            const auto currentNearObjects = session.GetNearObjects();
            session.RemoveNearObjects({ std::make_shared<NearObject>() });
            REQUIRE(currentNearObjects == session.GetNearObjects());
            session.RemoveNearObjects({ std::make_shared<NearObject>(), std::make_shared<NearObject>(), std::make_shared<NearObject>() });
            REQUIRE(currentNearObjects == session.GetNearObjects());
        }
    }
}
