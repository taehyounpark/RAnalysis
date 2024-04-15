#pragma once

#include "AnaQuery/EventHelpers.h"

#include "AsgTools/AnaToolHandle.h"

// Trigger include(s).
#include "TrigConfxAOD/xAODConfigTool.h"
#include "TrigDecisionTool/TrigDecisionTool.h"

#include "PATInterfaces/SystematicRegistry.h"
#include "PATInterfaces/SystematicSet.h"
#include "PATInterfaces/SystematicsUtil.h"
#include "PATInterfaces/SystematicVariation.h"

#include "xAODCore/ShallowCopy.h"
#include "xAODEventInfo/EventInfo.h"

#include "queryosity.h"

namespace AnaQ {


class TriggerDecision : public EventDecision<bool> {

private:
class xAODConfigTool : public TrigConf::xAODConfigTool {

public:
   // Inherit all constructors from the base class
   using TrigConf::xAODConfigTool::xAODConfigTool;

   // Make the beginEvent() function publicly accessible.
   using TrigConf::xAODConfigTool::beginEvent;

}; // class xAODConfigTool

public:
  TriggerDecision(const std::string& triggerSelection);
  virtual ~TriggerDecision() = default;

  void initialize(unsigned int, unsigned long long ,unsigned long long) override;
  bool evaluate(qty::column::observable<xAOD::EventInfo> evntInfo) const override;
  void finalize(unsigned int) override;

protected:
  mutable std::unique_ptr<xAODConfigTool> m_configTool; //!
  mutable std::unique_ptr<Trig::TrigDecisionTool> m_decisionTool; //!

  std::string m_triggerSelection;

};

}