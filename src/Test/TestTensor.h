/*
* Tests for Simd Library (http://ermig1979.github.io/Simd).
*
* Copyright (c) 2011-2018 Yermalayeu Ihar.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/
#ifndef __TestTensor_h__
#define __TestTensor_h__

#include "Test/TestLog.h"

namespace Test
{
    typedef std::vector<size_t> Shape;
    typedef std::vector<size_t> Index;

    template<class T> class Tensor
    {
    public:
        typedef T Type;

        SIMD_INLINE Tensor()
            : _size(0)
        {
        }

        SIMD_INLINE Tensor(const Test::Shape & shape, const Type & value = Type())
            : _shape(shape)
        {
            Resize(value);
        }

        SIMD_INLINE Tensor(std::initializer_list<size_t> shape, const Type & value = Type())
            : _shape(shape.begin(), shape.end())
        {
            Resize(value);
        }

        SIMD_INLINE ~Tensor()
        {
        }

        SIMD_INLINE void Reshape(const Test::Shape & shape, const Type & value = Type())
        {
            _shape = shape;
            Resize(value);
        }

        SIMD_INLINE void Reshape(std::initializer_list<size_t> shape, const Type & value = Type())
        {
            _shape.assign(shape.begin(), shape.end());
            Resize(value);
        }

        SIMD_INLINE void Extend(const Test::Shape & shape)
        {
            _shape = shape;
            Extend();
        }

        SIMD_INLINE void Extend(std::initializer_list<size_t> shape)
        {
            _shape.assign(shape.begin(), shape.end());
            Extend();
        }

        SIMD_INLINE const Test::Shape & Shape() const
        {
            return _shape;
        }

        SIMD_INLINE size_t Count() const
        {
            return _shape.size();
        }

        SIMD_INLINE size_t Index(ptrdiff_t axis) const
        {
            if (axis < 0)
                axis += _shape.size();
            return axis;
        }

        SIMD_INLINE size_t Axis(ptrdiff_t axis) const
        {
            return _shape[Index(axis)];
        }

        SIMD_INLINE size_t Size(ptrdiff_t startAxis, ptrdiff_t endAxis) const
        {
            startAxis = Index(startAxis);
            endAxis = Index(endAxis);
            assert(startAxis <= endAxis && (size_t)endAxis <= _shape.size());
            size_t size = 1;
            for (ptrdiff_t axis = startAxis; axis < endAxis; ++axis)
                size *= _shape[axis];
            return size;
        }

        SIMD_INLINE size_t Size(ptrdiff_t startAxis) const
        {
            return Size(startAxis, _shape.size());
        }

        SIMD_INLINE size_t Size() const
        {
            return _size;
        }

        SIMD_INLINE size_t Offset(const Test::Index & index) const
        {
            assert(_shape.size() == index.size());

            size_t offset = 0;
            for (size_t axis = 0; axis < _shape.size(); ++axis)
            {
                assert(_shape[axis] > 0);
                assert(index[axis] < _shape[axis]);

                offset *= _shape[axis];
                offset += index[axis];
            }
            return offset;
        }

        SIMD_INLINE size_t Offset(std::initializer_list<size_t> index) const
        {
            assert(_shape.size() == index.size());

            size_t offset = 0;
            for (const size_t * s = _shape.data(), *i = index.begin(); i < index.end(); ++s, ++i)
            {
                assert(*s > 0);
                assert(*i < *s);

                offset *= *s;
                offset += *i;
            }
            return offset;
        }

        SIMD_INLINE Type * Data()
        {
            return _data.data();
        }

        SIMD_INLINE const Type * Data() const
        {
            return _data.data();
        }

        SIMD_INLINE Type * Data(const Test::Index & index)
        {
            return Data() + Offset(index);
        }

        SIMD_INLINE const Type * Data(const Test::Index & index) const
        {
            return Data() + Offset(index);
        }

        SIMD_INLINE Type * Data(std::initializer_list<size_t> index)
        {
            return Data() + Offset(index);
        }

        SIMD_INLINE const Type * Data(std::initializer_list<size_t> index) const
        {
            return Data() + Offset(index);
        }

        void DebugPrint(std::ostream & os, const String & name, size_t first = 5, size_t last = 2) const
        {
            os << name << " { ";
            for (size_t i = 0; i < _shape.size(); ++i)
                os << _shape[i] << " ";
            os << "} " << std::endl;

            if (_size == 0)
                return;

            size_t n = _shape.size();
            Test::Shape firsts(n), lasts(n), index(n, 0);
            Strings separators(n);
            for (ptrdiff_t i = n - 1; i >= 0; --i)
            {
                if (i == n - 1)
                {
                    firsts[i] = first;
                    lasts[i] = last;
                    separators[i] = "\t";
                }
                else
                {
                    firsts[i] = std::max<size_t>(firsts[i + 1] - 1, 1);
                    lasts[i] = std::max<size_t>(lasts[i + 1] - 1, 1);
                    separators[i] = separators[i + 1] + "\n";
                }
            }
            DebugPrint(os, firsts, lasts, separators, index, 0);
            if (n == 1 || n == 0)
                os << "\n";
        }

    private:

        void DebugPrint(std::ostream & os, const Test::Shape & firsts, const Test::Shape & lasts, const Strings & separators, Test::Shape index, size_t order) const
        {
            if (order == _shape.size())
            {
                std::cout << std::fixed << std::setprecision(4);
                os << *Data(index);
                return;
            }
            if (firsts[order] + lasts[order] < _shape[order])
            {
                size_t lo = firsts[order], hi = _shape[order] - lasts[order];
                for (index[order] = 0; index[order] < lo; ++index[order])
                {
                    DebugPrint(os, firsts, lasts, separators, index, order + 1);
                    os << separators[order];
                }
                os << "..." << separators[order];
                for (index[order] = hi; index[order] < _shape[order]; ++index[order])
                {
                    DebugPrint(os, firsts, lasts, separators, index, order + 1);
                    os << separators[order];
                }
            }
            else
            {
                for (index[order] = 0; index[order] < _shape[order]; ++index[order])
                {
                    DebugPrint(os, firsts, lasts, separators, index, order + 1);
                    os << separators[order];
                }
            }
        }

        SIMD_INLINE void Resize(const Type & value)
        {
            _size = Size(0, _shape.size());
            _data.resize(_size, value);
            SetDebugPtr();
        }

        SIMD_INLINE void Extend()
        {
            _size = Size(0, _shape.size());
            if (_size > _data.size())
                _data.resize(_size);
            SetDebugPtr();
        }

#if defined(_DEBUG) && defined(_MSC_VER)
        const Type * _ptr;

        SIMD_INLINE void SetDebugPtr()
        {
            _ptr = _data.data();
        }
#else
        SIMD_INLINE void SetDebugPtr()
        {
        }
#endif

        typedef std::vector<Type, Simd::Allocator<Type>> Vector;

        Test::Shape _shape;
        size_t _size;
        Vector _data;
    };

    typedef Tensor<float> Tensor32f;

    //-------------------------------------------------------------------------

    inline void Compare(const Tensor32f & a, const Tensor32f & b, float differenceMax, bool printError, int errorCountMax, DifferenceType differenceType, const String & description,
        Shape index, size_t order, int & errorCount, std::stringstream & message)
    {
        if (order == a.Count())
        {
            float _a = *a.Data(index);
            float _b = *b.Data(index);
            float absolute = ::fabs(_a - _b);
            float relative = ::fabs(_a - _b) / Simd::Max(::fabs(_a), ::fabs(_b));
            bool error = false;
            switch (differenceType)
            {
            case DifferenceAbsolute: error = absolute > differenceMax; break;
            case DifferenceRelative: error = relative > differenceMax; break;
            case DifferenceBoth: error = absolute > differenceMax && relative > differenceMax; break;
            case DifferenceAny: error = absolute > differenceMax || relative > differenceMax; break;
            }
            if (error)
            {
                errorCount++;
                if (printError)
                {
                    if (errorCount == 1)
                        message << std::endl << "Fail comparison: " << description << std::endl;
                    message << "Error at [";
                    for(size_t i = 0; i < index.size() - 1; ++i)
                        message << index[i] << ", ";
                    message << index[index.size() - 1] << "] : " << _a << " != " << _b << ";"
                        << " (absolute = " << absolute << ", relative = " << relative << ")!" << std::endl;
                }
                if (errorCount > errorCountMax)
                {
                    if (printError)
                        message << "Stop comparison." << std::endl;
                }
            }
        }
        else
        {
            for (index[order] = 0; index[order] < a.Axis(order) && errorCount < errorCountMax; ++index[order])
                Compare(a, b, differenceMax, printError, errorCountMax, differenceType, description, index, order + 1, errorCount, message);
        }
    }

    inline bool Compare(const Tensor32f & a, const Tensor32f & b, float differenceMax, bool printError, int errorCountMax, DifferenceType differenceType, const String & description = "")
    {
        std::stringstream message;
        message << std::fixed << std::setprecision(6);
        int errorCount = 0;
        Index index(a.Count(), 0);
        Compare(a, b, differenceMax, printError, errorCountMax, differenceType, description, index, 0, errorCount, message);
        if (printError && errorCount > 0)
            TEST_LOG_SS(Error, message.str());
        return errorCount == 0;
    }
}

#endif// __TestTensor_h__
