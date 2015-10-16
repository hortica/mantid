#include "MantidGeometry/MDGeometry/GeneralFrame.h"

namespace Mantid {
namespace Geometry {

const std::string GeneralFrame::GeneralFrameDistance = "Distance";
const std::string GeneralFrame::GeneralFrameTOF = "Time of Flight";
const std::string GeneralFrame::GeneralFrameName = "General Frame";

GeneralFrame::GeneralFrame(const std::string &frameName,
                           std::unique_ptr<Kernel::MDUnit> unit)
    : m_unit(unit.release()), m_frameName(frameName) {}

GeneralFrame::GeneralFrame(const std::string &frameName,
                           const Kernel::UnitLabel &unit)
    : m_unit(new Mantid::Kernel::LabelUnit(unit)), m_frameName(frameName) {}

//----------------------------------------------------------------------------------------------
/** Destructor
 */
GeneralFrame::~GeneralFrame() {}

Kernel::UnitLabel GeneralFrame::getUnitLabel() const {
  return m_unit->getUnitLabel();
}

const Kernel::MDUnit &GeneralFrame::getMDUnit() const { return *m_unit; }

bool GeneralFrame::canConvertTo(const Kernel::MDUnit &otherUnit) const {
  return *this->m_unit == otherUnit;
}

std::string GeneralFrame::name() const { return m_frameName; }

GeneralFrame *GeneralFrame::clone() const {
  return new GeneralFrame(m_frameName,
                          std::unique_ptr<Kernel::MDUnit>(m_unit->clone()));
}

Mantid::Kernel::SpecialCoordinateSystem
GeneralFrame::equivalientSpecialCoordinateSystem() const {
  return Mantid::Kernel::SpecialCoordinateSystem::None;
}

bool GeneralFrame::isQ() const { return false; }

bool GeneralFrame::isSameType(const MDFrame &frame) const {
  auto isSameType = true;
  try {
    dynamic_cast<const GeneralFrame &>(frame);
  } catch (std::bad_cast &) {
    isSameType = false;
  }
  return isSameType;
}

} // namespace Geometry
} // namespace Mantid
