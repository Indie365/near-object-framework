
#ifndef UWB_APP_CONFIGURATION_HXX
#define UWB_APP_CONFIGURATION_HXX

#include <cstddef>
#include <cstdint>
#include <memory>
#include <tuple>

#include <UwbCxLrpDeviceGlue.h>

namespace windows::devices
{
/**
 * @brief Adaptor for using UWB_APP_CONFIG_PARAM UwbCx DDI structure in a nicer way. 
 * 
 * @tparam PropertyT The type of property the UWB_APP_CONFIG_PARAM structure
 * holds as its flexible array member.
 */
template <typename PropertyT>
class UwbAppConfigurationParameter
{
public:
    explicit UwbAppConfigurationParameter(const PropertyT& value, size_t parameterSize = sizeof(PropertyT)) :
        m_size(offsetof(UWB_APP_CONFIG_PARAM, paramValue[sizeof(PropertyT)])),
        m_buffer(std::make_unique<uint8_t[]>(m_size)),
        m_parameter(*reinterpret_cast<UWB_APP_CONFIG_PARAM*>(m_buffer.get())),
        m_value(reinterpret_cast<PropertyT&>(m_parameter.paramValue))
    {
        m_parameter.size = m_size;
        m_parameter.paramLength = parameterSize;
        m_value = value;
    }

    /**
     * @brief The total size of the UWB_APP_CONFIG_PARAM structure. 
     * 
     * @return std::size_t 
     */
    std::size_t
    Size() noexcept
    {
        return m_size;
    }

    /**
     * @brief The buffer containing the complete UWB_APP_CONFIG_PARAM structure,
     * along with the trailing parameter value.
     * 
     * @return uint8_t* 
     */
    uint8_t*
    Buffer() noexcept
    {
        return m_buffer;
    }

    /**
     * @brief Return a typed reference to the flexible array member value.
     * 
     * @return PropertyT& 
     */
    PropertyT&
    Value() noexcept
    {
        return m_value;
    }

    /**
     * @brief Reference to the UwbCx DDI structure. 
     * 
     * @return UWB_APP_CONFIG_PARAM& 
     */
    UWB_APP_CONFIG_PARAM&
    DdiParameter() noexcept
    {
        return m_parameter;
    }

    /**
     * @brief The buffer and associated size that is suitable for passing to the UwbCx DDI. 
     * 
     * @return std::tuple<uint8_t*, std::size_t> 
     */
    std::tuple<uint8_t*, std::size_t>
    DdiBuffer() noexcept
    {
        return { m_buffer, m_size };
    }

private:
    // order of members here is important to enforce proper initialization order
    std::size_t m_size;
    std::unique_ptr<uint8_t[]> m_buffer;
    UWB_APP_CONFIG_PARAM& m_parameter;
    PropertyT& m_value;
};
} // namespace windows::devices

#endif // UWB_APP_CONFIGURATION_HXX
