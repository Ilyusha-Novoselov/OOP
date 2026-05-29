#include <LabTester.hxx>
#include <iostream>

int main()
{
    try {
        LabTester aTester;
        aTester.RunAllTests();
    } catch (const std::exception& anException) {
        std::cerr << "CRITICAL ERROR during testing: " << anException.what() << "\n";
        return 1;
    } catch (...) {
        std::cerr << "Unknown error occurred.\n";
        return 1;
    }

    return 0;
}