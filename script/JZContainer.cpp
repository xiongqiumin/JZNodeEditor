#include <functional>
#include <stdexcept>
#include "JZContainer.h"

void checkEmpty(int size)
{
    if (size == 0)
    {
        QString error = QString::asprintf("is empty");
        throw std::runtime_error(qPrintable(error));
    }
}

void checkSize(int index, int size)
{
    if (index < 0 || index >= size)
    {
        QString error = QString::asprintf("index %d out of range %d", index, size);
        throw std::runtime_error(qPrintable(error));
    }
}

void checkContains(bool flag)
{
    if (flag)
    {
        QString error = QString::asprintf("not contains");
        throw std::runtime_error(qPrintable(error));
    }
}