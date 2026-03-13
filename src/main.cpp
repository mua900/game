#include "application.h"

int main()
{
    Application application;

    application.initialize();

    while (!application.quit)
    {
        application.handle_events();
        application.update();
        application.draw();
    }

    application.cleanup();

    return 0;
}