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
#ifndef __MAXCONNECTOR_H__
#define __MAXCONNECTOR_H__

#include "StochasticObject.h"
#include <memory>

class MaxConnector: public StochasticObject {
    private:
        std::shared_ptr<StochasticObject> pObject1, pObject2;
    public:
        MaxConnector(const std::shared_ptr<StochasticObject> &pObject1_, const std::shared_ptr<StochasticObject> &pObject2_);
        virtual ~MaxConnector(void) = default;
        virtual double distributionFunction(double) const;
        virtual double getMinimum(void) const;

};
#endif
