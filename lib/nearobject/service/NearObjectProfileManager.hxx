
#ifndef __NEAR_OBJECT_PROFILE_MANAGER_HXX__
#define __NEAR_OBJECT_PROFILE_MANAGER_HXX__

#include <shared_mutex>
#include <vector>

#include <nearobject/NearObjectProfile.hxx>

namespace nearobject
{
namespace service
{
class NearObjectProfileManager
{
public:
    /**
     * @brief Describes the lifetime of the profile.
     */
    enum ProfileLifetime {
        Persistent,
        Ephemeral
    };

    /**
     * @brief Adds a new profile to support.
     *
     * @param profile The profile to add.
     * @param lifetime The lifetime of the profile.
     */
    void
    AddProfile(const NearObjectProfile& profile, ProfileLifetime lifetime = ProfileLifetime::Persistent);

    /**
     * @brief Find all profiles which match the provided profile.
     *
     * TODO: Matching by another profile might not make sense. Consider what
     * input argument(s) are more sensible, and update this prototype with them.
     * This will likely come from an out-of-band connection, and may identify
     * this device as a peer in some way.
     *
     * @param profile
     * @return std::vector<NearObjectProfile>
     */
    std::vector<NearObjectProfile>
    FindMatchingProfiles(const NearObjectProfile& profile) const;

    /**
     * @brief Return all known profiles.
     *
     * @return std::vector<NearObjectProfile>
     */
    std::vector<NearObjectProfile>
    GetAllProfiles() const;

protected:
    /**
     * @brief Persist the profile.
     *
     * @param profile The profile to persist.
     */
    void
    PersistProfile(const NearObjectProfile& profile);

    /**
     * @brief Obtain all persisted profiles.
     *
     * @return std::vector<NearObjectProfile>
     */
    std::vector<NearObjectProfile>
    ReadPersistedProfiles() const;

private:
    mutable std::shared_mutex m_profilesGate{};
    std::vector<NearObjectProfile> m_profiles{};
};

} // namespace service
} // namespace nearobject

#endif // __NEAR_OBJECT_PROFILE_MANAGER_HXX__