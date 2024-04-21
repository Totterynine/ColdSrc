#include "appframework.h"

int main(int argc, char *argv[])
{
    AppFramework framework = AppFramework();

    Array<String> args;
    for (int i = 0; i < argc; ++i)
        args.push_back( String(argv[i]));

    framework.Execute(Span(args.begin(), args.end()));

    return 0;
}