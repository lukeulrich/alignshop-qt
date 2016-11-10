#ifndef SINGLETONPTR_H
#define SINGLETONPTR_H

/**
  * SingletonPtr is a templated class that encapsulates access to a singleton class.
  *
  * T must be a Singleton class with a static method called instance which returns a pointer to T.
  *
  * Reference: http://www.flipcode.com/archives/Smart_Pointers.shtml (26 May 2010)
  */
template <class T>
class SingletonPtr
{
public:
    operator bool() const;
    bool operator!() const;
    operator T*() const;
    operator const T*() const;
    T& operator*() const;
    T* operator->(void) const;
};

/**
  * @returns bool
  */
template <class T>
inline
SingletonPtr::operator bool() const
{
    return T::instance() != 0;
}

/**
  * @returns bool
  */
template <class T>
inline
bool SingletonPtr::operator!() const
{
    return T::instance() == 0;
}

/**
  * returns T *
  */
template <class T>
inline
SingletonPtr::operator T*() const
{
    return T::instance();
}

/**
  * returns const T *
  */
template <class T>
inline
SingletonPtr::operator const T*() const
{
    return T::instance();
}

/**
  * @returns T &
  */
template <class T>
inline
T& SingletonPtr::operator*() const
{
    return *T::instance();
}

/**
  * @returns T *
  */
template <class T>
inline
T* SingletonPtr::operator->(void) const
{
    return T::instance();
}

#endif // SINGLETONPTR_H
