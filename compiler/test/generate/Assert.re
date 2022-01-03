open Kore;

include TestLibrary.Assert;

module Formatter = Generate.JavaScript_Formatter;

module Compare = {
  open Alcotest;

  include Compare;

  let expression = (module_type: Target.module_t) =>
    testable(Formatter.fmt_expression(module_type), (==));

  let statement = (module_type: Target.module_t) =>
    testable(Formatter.fmt_statement(module_type), (==));
};
