#pragma once

#include <memory>

struct IObjModel
{
  virtual ~IObjModel(){}
  virtual void draw() = 0;
};

std::shared_ptr<IObjModel> create_model(char const * fileaname);