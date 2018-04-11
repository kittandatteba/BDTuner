#pragma once
#include <memory>
#include"BaseCommand.h"

class SvfParser {

public:

  static std::unique_ptr<BaseCommand> parse(tstring &line);

private:
  SvfParser() = delete;
  ~SvfParser() = delete;

  SvfParser(const SvfParser&) = delete;
  SvfParser(SvfParser&&) = delete;
  SvfParser& operator =(const SvfParser&) = delete;
  SvfParser& operator =(SvfParser&&) = delete;
};
