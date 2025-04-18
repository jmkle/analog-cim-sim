/******************************************************************************
 * Copyright (C) 2025 Rebecca Pelke                                           *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 ******************************************************************************/
#ifndef SYMADC_H
#define SYMADC_H

#include "adc/adc.h"

namespace nq {

class SymADC : public ADC {
  public:
    explicit SymADC();
    SymADC(const SymADC &) = delete;
    virtual ~SymADC() = default;

    float analog_digital_conversion(const float current) const override;

  private:
    float get_max_curr() const;
    float get_min_curr() const;
    const float step_size_; // ADC step size (delta)
};

} // namespace nq

#endif
