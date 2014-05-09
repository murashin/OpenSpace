/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2014                                                               *
 *                                                                                       *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this  *
 * software and associated documentation files (the "Software"), to deal in the Software *
 * without restriction, including without limitation the rights to use, copy, modify,    *
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to    *
 * permit persons to whom the Software is furnished to do so, subject to the following   *
 * conditions:                                                                           *
 *                                                                                       *
 * The above copyright notice and this permission notice shall be included in all copies *
 * or substantial portions of the Software.                                              *
 *                                                                                       *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,   *
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A         *
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT    *
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF  *
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE  *
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                                         *
 ****************************************************************************************/

#include "gtest/gtest.h"

#include <openspace/util/psc.h>
#include <openspace/util/pss.h>

class PowerscaleCoordinatesTest : public testing::Test {
protected:
	PowerscaleCoordinatesTest() {
    }

	~PowerscaleCoordinatesTest() {
    }

    void reset() {
    }

    openspace::SceneGraph* scenegraph;
};


TEST_F(PowerscaleCoordinatesTest, psc) {

    openspace::psc reference(2.0, 1.0, 1.1, 1.0);
    
    openspace::psc first(1.0,0.0,1.0,0.0);
    openspace::psc second(1.9,1.0,1.0,1.0);
    
    EXPECT_EQ(reference, first + second);
    EXPECT_TRUE(reference == (first + second));
    
    openspace::psc third = first;
    first[0] = 0.0;
    
    EXPECT_TRUE(third != first);
    
    
}

TEST_F(PowerscaleCoordinatesTest, pss) {
    
    openspace::pss first(1.0,1.0);
    openspace::pss second(1.0,-1.0);
    EXPECT_EQ(openspace::pss(1.01,1.0), first + second);
    EXPECT_EQ(openspace::pss(1.01,1.0), second + first);
    /*
    EXPECT_TRUE(first < (first + second));
    bool retu =(second < (first + second));
    
    std::cout << "retu: " << retu << std::endl;
    EXPECT_TRUE(retu);
    
    EXPECT_FALSE(first > (first + second));
    EXPECT_FALSE(second > (first + second));
    
    */
}

