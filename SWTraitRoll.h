/*
Copyright 2021 Wilhelm Neubert
This file is part of SW Roll Calculator.

SW Roll Calculator is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

SW Roll Calculator is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SW Roll Calculator.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef __SWTRAITROLL_H__
#define __SWTRAITROLL_H__
#include "StochasticObject.h"

class SWTraitRoll: public StochasticObject {
    private:
        unsigned int nTraitDieSides;
        unsigned int nWildDieSides;
        unsigned int nTraitDiceCount;
        unsigned int nRerolls;
        int nMod;
    public:
        SWTraitRoll(unsigned int nTraitDieSides, unsigned int nWildDieSides = 6, int nMod = 0, int nTraitDiceCount_ = 1, int nRerolls_ = 0);
        virtual ~SWTraitRoll(void) = default;

        virtual double distributionFunction(double x) const;
        virtual double getMinimum(void) const {return -1.;};

        int getMod(void) const {return nMod;};
        void setMod(int nMod_) {nMod=nMod_;};

        unsigned int getRerolls(void) const {return nRerolls;};
        void setRerolls(unsigned int nRerolls_) {nRerolls=nRerolls_;};
};
#endif
