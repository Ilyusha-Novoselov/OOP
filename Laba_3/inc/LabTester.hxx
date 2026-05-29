#ifndef _Laba3_LabTester_Header
#define _Laba3_LabTester_Header

class LabTester
{
public:
    void RunAllTests();

private:
    void TestSingletonAndFactory();
    void TestVirtualConstructors();
    void TestMathAndDelegation();
    void TestPolymorphicPersistence();
    void TestMatryoshkaEffect();
};

#endif
