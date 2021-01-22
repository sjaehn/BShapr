/* B.Shapr
 * Beat / envelope shaper LV2 plugin
 *
 * Copyright (C) 2019 by Sven Jähnichen
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef RINGBUFFER_HPP_
#define RINGBUFFER_HPP_

#include <cstddef>
#include <cstring>
#include <exception>

#endif /* RINGBUFFER_HPP_ */

template <class T>
class RingBuffer
{
public:
        RingBuffer ();
        RingBuffer (const size_t size);
        ~RingBuffer ();

        RingBuffer& operator= (const RingBuffer& that);
        T& operator[] (const long n);
        const T& operator[] (const long n) const;
        size_t size () const;
        void resize (const size_t size);
        void fill (const T& value);
        void push_back (const T& value);
        void push_back (const T* values, const size_t n);
        void pop_back ();
        void pop_back (const size_t n);
        void move (const long n);

protected:
        T* data_;
        size_t size_;
        size_t position_;
};

template <class T> RingBuffer<T>::RingBuffer () : RingBuffer (0) {}

template <class T> RingBuffer<T>::RingBuffer  (const size_t size) : data_ (nullptr), size_ (0), position_ (0)
{
	if (size !=0)
	{
		try {resize (size);}
		catch (std::bad_alloc& ba) {throw ba;}
	}
}

template <class T> RingBuffer<T>::~RingBuffer ()
{
	if (data_) free (data_);
}

template <class T> T& RingBuffer<T>::operator[] (const long n)
{
        if (!size_) return T();
        return data_[(position_ + size_ + (n % size_)) % size_];
}

template <class T> RingBuffer<T>& RingBuffer<T>::operator= (const RingBuffer& that)
{
        resize (that.size());
        for (int i = 0; i < size_; ++i) data_[i] = that[i];
        return *this;
}

template <class T> const T& RingBuffer<T>::operator[] (const long n) const
{
        if (!size_) return T();
        return data_[(position_ + size_ + (n % size_)) % size_];
}

template <class T> size_t RingBuffer<T>::size () const {return size_;}

template <class T> void RingBuffer<T>::resize (const size_t size)
{
	if (data_) free (data_);
        position_ = 0;
        if (size)
        {
                calloc (data_, size * sizeof (T));
                if (!data_)
                {
                        size_ = 0;
                        throw std::bad_alloc();
                }
        }
        size_ = size;
}

template <class T> void RingBuffer<T>::fill (const T& value)
{
        for (size_t i = 0; i < size_; ++i) data_[i] = value;
        position_ = 0;
}

template <class T> void RingBuffer<T>::push_back (const T& value)
{
        if (size_)
        {
                data_[position_] = value;
                position_ = (position_ + 1) % size_;
        }
}

template <class T> void RingBuffer<T>::push_back (const T* values, const size_t n)
{
        if (size_)
        {
                size_t nx = (n > size_ ? size_ : n);
                size_t n0 = n - nx;
                size_t i1 = (position_ + nx > size_ ? size_ - position_ : nx);
                for (size_t i = 0; i < i1; ++i) data_[position_ + i] = values[i + n0];
                for (size_t i = i1; i < nx; ++i) data_[i - i1] = values[i + n0];
                position_ = (position_ + n) % size_;
        }
}

template <class T>void RingBuffer<T>::pop_back ()
{
        if (size_) position_ = (position_ + size_ - 1) % size_;
}

template <class T>void RingBuffer<T>::pop_back (const size_t n)
{
        if (size_) position_ = (position_ + size_ - (n % size_)) % size_;
}

template <class T>void RingBuffer<T>::move (const long n)
{
        if (size_) position_ = (position_ + size_ + (n % size_)) % size_;
}
