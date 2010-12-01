
#if !defined(LAUNCHER_ARGUMENTBUILDER_H)
#define LAUNCHER_ARGUMENTBUILDER_H

/**
Builds an argument list for a call to a Mono delegate.
*/
template<int MaxArgs>
class QtToMonoArgumentConverter
{
public:


private:
    QVarLengthArray<qint64, MaxArgs> mValueArguments;
    void* mStringArguments[MaxArgs];
};

#endif
