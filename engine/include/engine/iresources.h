
#if !defined(ENGINE_IRESOURCES_H)
#define ENGINE_IRESOURCES_H

namespace EvilTemple {

/**
    This interface should be used to access resources.
  */
class IResources {
public:
    
    /**
        Reads an entire resource into memory, given the resource's path.
     */
    virtual QByteArray getResource(const QByteArray &path) = 0;

    /**
        Lists all resources within a given directory.
     */
    virtual QList<QByteArray> listResources(const QByteArray &path, bool recursive = false) = 0;

    /**
        Searches for resources, given a pattern that will be matched against the path of the resources.
     */
    virtual QList<QByteArray> findResources(const QByteArray &path,
                                            const QByteArray &pattern, 
                                            bool recursive = false) = 0;
    
protected:
    /*
        This is a pure interface and should not be used to destruct the actual instance.
     */
    ~IResources() {}

};

}

#endif // ENGINE_IRESOURCES_H
