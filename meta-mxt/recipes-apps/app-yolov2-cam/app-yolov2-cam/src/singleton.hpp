#ifndef SINGLETON_HPP
#define SINGLETON_HPP
 
/* Template definition for Singleton */
template<typename C>
class Singleton{
public:
    virtual ~Singleton(){
        m_instance = nullptr;
    }
 
    static C* GetInstance(){
        if(!m_instance){
            m_instance = new C();
        }
        return m_instance;
    }
protected:
    static C* m_instance;
};
 
template<typename C>
C* Singleton<C>::m_instance = nullptr;

#endif // SINGLETON_HPP
