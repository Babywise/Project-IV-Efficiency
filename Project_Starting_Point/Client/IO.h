#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <future>
#include "../Shared/configManager.h"

void GetSizePromise(std::promise<unsigned int> promise);