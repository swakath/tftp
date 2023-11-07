/**
 * @file singleton.cpp
 * @brief Singleton Signature.
 *
 * @date November 07, 2023
 * @author S U Swakath
 * Contact suswakath@gmail.com
 * 
 * MIT License
*/ 
#ifndef SINGLETON_H
#define SINGLETON_H
    template <class T>
    class Singleton{
        protected:
            Singleton(){}
            virtual ~Singleton(){}
        private:
            Singleton(const Singleton&) = delete;
            Singleton& operator=(const Singleton&) = delete;
        public:
            static T& getInstance(){
                static T instance;
                return instance;
            }
    };
#endif